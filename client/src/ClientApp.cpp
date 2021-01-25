#include "ClientApp.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>


#include "GlobalEventLogger.h"
#include "GlobalEvent.h"

ClientApp::ClientApp(int &argc, char **argv)
 :   QApplication(argc, argv)
{
    m_comMan = nullptr;
    m_loginDiag = nullptr;
    m_mainWindow = nullptr;

    m_translator = new QTranslator();

    //Translations
    setTranslation();


    QFile file(":/stylesheet.qss");
    file.open(QFile::ReadOnly);
    QString stylesheet = QLatin1String(file.readAll());
    setStyleSheet(stylesheet);

    setApplicationName(QString("OpenTeraPlus v") + QString(OPENTERAPLUS_VERSION));
    qDebug() << "Starting App " << applicationName();

    // Load config
    loadConfig();

    // Setup global event logger
    setupLogger();

    // Connect signals
    connectSignals();

    // Show login dialog
    showLogin();
}

ClientApp::~ClientApp()
{
    if (m_loginDiag)
        delete m_loginDiag;

    if (m_comMan){
       m_comMan->disconnectFromServer();
       m_comMan->deleteLater();
    }

}

ComManager *ClientApp::getComManager()
{
    return m_comMan;
}

void ClientApp::loadConfig()
{
    //QString configFile = applicationDirPath() + "/config/client/TeraClientConfig.txt";
    QStringList documents_paths = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    QString configFile = documents_paths.first() + "/OpenTeraPlus/config/OpenTeraPlus.json";
    qDebug() << "Loading config file: " << configFile;

    // Check if config file exists and, if not, copy from QRC
    if (!QFile::exists(configFile)){
        qDebug() << "ClientApp : No Config File - Creating new one.";
        // Create folder if not exists
        QFileInfo config_file_info(configFile);
        QDir config_folder;
        config_folder.mkpath(config_file_info.path());

        QString localConfigFile = applicationDirPath() + "/config/OpenTeraPlus.json";
        if (QFile::exists(localConfigFile)){
            // Copy from installation directory
            QFile::copy(localConfigFile, configFile);
        }else{
            // Copy from QRC
            QFile::copy("://defaults/OpenTeraPlus.json", configFile);
        }
        QFile::setPermissions(configFile, QFile::WriteUser);
    }

    m_config.setFilename(configFile);

    if (!m_config.loadConfig()){
        if (!m_config.hasParseError()){ // Missing file
            qDebug() << "Can't load file: " << configFile;
        }else{
            qDebug() << "Parse error: " << m_config.getLastError().errorString() << " at character " << m_config.getLastError().offset;
        }
    }
}

void ClientApp::connectSignals()
{

}

void ClientApp::showLogin()
{
    if (m_loginDiag == nullptr){
        m_loginDiag = new LoginDialog();
        connect(m_loginDiag, &LoginDialog::loginRequest, this, &ClientApp::loginRequested);

        // Set server names
        m_loginDiag->setServerNames(m_config.getServerNames());

        // Show servers list... or not!
        m_loginDiag->showServers(m_config.showServers());
    }

    // Delete main window, if present
    if (m_mainWindow){
        m_mainWindow->deleteLater();
        m_mainWindow = nullptr;
    }
    m_loginDiag->show();
}

void ClientApp::showMainWindow()
{
    if (m_mainWindow != nullptr){
        m_mainWindow->deleteLater();
    }
    m_mainWindow = new MainWindow(m_comMan);

    // Delete login window, if present
    if (m_loginDiag){
        m_loginDiag->deleteLater();
        m_loginDiag = nullptr;
    }

    // Connect signals
    connect(m_mainWindow, &MainWindow::logout_request, this, &ClientApp::logoutRequested);

    // Add logged action
    GlobalEvent login_event(EVENT_LOGIN, tr("Connexion"));
    GlobalEventLogger::instance()->logEvent(login_event);

    m_mainWindow->showMaximized();

    if (m_comMan->getCurrentUser().hasNameField())
        processQueuedEvents();
}

void ClientApp::setupLogger()
{

    if (m_config.getLogToFile()){
        // Log to file
        QStringList documents_paths = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        QString logFile = documents_paths.first() + "/OpenTeraPlus/logs";
        Logger::instance()->setFileName(logFile + "/logs_" + QDateTime::currentDateTime().toString("yyyy_MM_dd") + ".txt");
        GlobalEventLogger::instance()->startLogging(logFile);
    }else{
        GlobalEventLogger::instance()->startLogging();
    }
}

void ClientApp::processQueuedEvents()
{
    if (!m_mainWindow)
        return;

    for (int i=0; i<m_eventQueue.count(); i++){
        for(int j=0; j< m_eventQueue.at(i).events_size(); j++){
            if (m_eventQueue.at(i).events(j).Is<JoinSessionEvent>()){
                JoinSessionEvent event;
                if (m_eventQueue.at(i).events(j).UnpackTo(&event))
                    m_mainWindow->ws_joinSessionEvent(event);
            }
        }
    }

    m_eventQueue.clear();
}

void ClientApp::setTranslation(QString language)
{

   bool lang_changed = false;

   if (language.isEmpty()){
       //Set French as default
       m_currentLocale = QLocale(QLocale::French);
       //m_currentLocale = QLocale(); // Use system locale by default
       lang_changed = true;
   }
   if (language.toLower() == "en" && m_currentLocale != QLocale::English){
       m_currentLocale = QLocale(QLocale::English);
       lang_changed = true;
   }

   if (language.toLower() == "fr" && m_currentLocale != QLocale::French){
       m_currentLocale = QLocale(QLocale::French);
       lang_changed = true;
   }

   //QLocale english = QLocale(QLocale::English);
   //QLocale french = QLocale(QLocale::French);
   //QLocale::setDefault(english);
   //qDebug() << QLocale();

   if (lang_changed){
       QLocale::setDefault(m_currentLocale);
       if (m_translator->load(m_currentLocale, QLatin1String("openteraplus"), QLatin1String("_"), QLatin1String(":/translations"))) {
           this->installTranslator(m_translator);
           //qDebug() << "Installed translator";
       }
   }

}

void ClientApp::loginRequested(QString username, QString password, QString server_name)
{
    // Find server url for that server
    QUrl server = m_config.getServerUrl(server_name);

    // Create ComManager for that server
    if (m_comMan){
        m_comMan->deleteLater();
    }
    m_comMan = new ComManager(server);

    // Connect signals
    connect(m_comMan, &ComManager::socketError, this, &ClientApp::on_serverError);
    connect(m_comMan, &ComManager::serverDisconnected, this, &ClientApp::on_serverDisconnected);
    connect(m_comMan, &ComManager::loginResult, this, &ClientApp::on_loginResult);
    connect(m_comMan, &ComManager::networkError, this, &ClientApp::on_networkError);
    connect(m_comMan, &ComManager::preferencesUpdated, this, &ClientApp::preferencesUpdated);
    connect(m_comMan, &ComManager::newVersionAvailable, this, &ClientApp::on_newVersionAvailable);
    connect(m_comMan, &ComManager::currentUserUpdated, this, &ClientApp::on_currentUserUpdated);

    connect(m_comMan->getWebSocketManager(), &WebSocketManager::genericEventReceived, this, &ClientApp::ws_genericEventReceived);

    // Connect to server
    m_comMan->connectToServer(username, password);

}

void ClientApp::logoutRequested()
{
    m_comMan->disconnectFromServer();
    showLogin();
}

void ClientApp::on_loginResult(bool logged)
{
    if (m_loginDiag){
        if (!logged){
            m_loginDiag->setStatusMessage(tr("Utilisateur ou mot de passe invalide."),true);
        }else{
            m_loginDiag->setStatusMessage(tr("Bienvenue!"));
            showMainWindow();
        }
    }else{
        // Looks like we are not able to authentificate to the server anymore... Show error and login dialog again
        GlobalMessageBox msg;
        msg.showError(tr("Déconnexion"), tr("Vous avez été déconnecté du serveur. Veuillez vous connecter à nouveau."));
        m_comMan->disconnectFromServer();
    }
}

void ClientApp::on_serverDisconnected()
{
    LOG_DEBUG("Disconnected from server.", "ClientApp::on_serverDisconnected");
    showLogin();
}

void ClientApp::on_serverError(QAbstractSocket::SocketError error, QString error_str)
{
    if (m_loginDiag){
        if (error == QAbstractSocket::ConnectionRefusedError)
            error_str = tr("La connexion a été refusée par le serveur.");
        m_loginDiag->setStatusMessage(error_str, true);
    }

    // TODO: Manage error in main UI
}

void ClientApp::on_networkError(QNetworkReply::NetworkError error, QString error_str, QNetworkAccessManager::Operation op, int status_code)
{
    Q_UNUSED(op);

    if (m_loginDiag){
        switch(error){
            case QNetworkReply::ConnectionRefusedError:
                error_str = tr("La connexion a été refusée par le serveur.");
            break;
            case QNetworkReply::AuthenticationRequiredError:
                //on_loginResult(false);
                return;
            break;
            case QNetworkReply::TimeoutError:
                error_str = tr("Impossible de rejoindre le serveur.");
            break;
            case QNetworkReply::HostNotFoundError:
                error_str = tr("Le serveur est introuvable.");
            break;
            default:
                error_str = tr("Impossible de se connecter (Code erreur: ") + QString::number(status_code) + " " + error_str + ")";
        }

        //Remove \n from error_str
        m_loginDiag->setStatusMessage(error_str.replace("\n", ""), true);
    }
    // TODO: Manage error in main UI
}

void ClientApp::on_newVersionAvailable(QString version, QString download_url)
{
    // Check to be sure that the new version is an updated version and not a previous one...
    QStringList versions = version.split(".");
    if (versions.count() < 3){
        LOG_WARNING(tr("Le format de la version est inconnu: ") + version, "ClientApp::on_newVersionAvailable");
    }

    if (versions.at(0).toInt() > QString(OPENTERAPLUS_VERSION_MAJOR).toInt() ||
            versions.at(1).toInt() > QString(OPENTERAPLUS_VERSION_MINOR).toInt() ||
            versions.at(2).toInt() > QString(OPENTERAPLUS_VERSION_PATCH).toInt())
    {

        GlobalMessageBox msg;
        QString version_info = tr("Une nouvelle version (") + version + tr(") du logiciel est disponible.") + "\n\n";
        if (download_url.isEmpty()){
            version_info += tr("Veuillez contacter votre fournisseur pour l'obtenir.");
        }else{
            version_info += tr("Cliquez ") + "<a href=" + download_url + ">" + tr("ICI") + "</a>" + tr(" pour la télécharger.");
        }
        msg.showInfo(tr("Nouvelle version disponible!"), version_info);
    }
}

void ClientApp::on_currentUserUpdated()
{
    if (!m_mainWindow)
        return;

    processQueuedEvents();
}

void ClientApp::ws_genericEventReceived(TeraEvent event)
{
    if (m_mainWindow)
        return; // Don't stack for nothing

    m_eventQueue.append(event);
}


void ClientApp::preferencesUpdated()
{
    // Update translation
    setTranslation(m_comMan->getCurrentPreferences().getLanguage());
}
