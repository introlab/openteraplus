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

    //Translations...
    QLocale english = QLocale(QLocale::English);
    QLocale french = QLocale(QLocale::French);
    QLocale::setDefault(english);
    qDebug() << QLocale();
    if (m_translator->load(english, QLatin1String("openteraplus"), QLatin1String("_"), QLatin1String(":/translations"))) {
        this->installTranslator(m_translator);
        qDebug() << "Installed translator";
    }

    QFile file(":/stylesheet.qss");
    file.open(QFile::ReadOnly);
    QString stylesheet = QLatin1String(file.readAll());
    setStyleSheet(stylesheet);

    setApplicationName(QString("TeraClient v") + QString(TERAPLUS_VERSION));
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
    QString configFile = applicationDirPath() + "/config/client/TeraClientConfig.txt";
    qDebug() << "Loading config file: " << configFile;

    // Check if config file exists and, if not, copy from QRC
    if (!QFile::exists(configFile)){
        qDebug() << "ClientApp : No Config File - Creating new one.";
        // Create folder if not exists
        QFileInfo config_file_info(configFile);
        QDir config_folder;
        config_folder.mkpath(config_file_info.path());
        QFile::copy("://defaults/TeraClientConfig.txt", configFile);
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
}

void ClientApp::setupLogger()
{
    // TODO: Disable logging to file for security reasons in most case!
    if (m_config.getLogToFile()){
        GlobalEventLogger::instance()->startLogging(m_config.getLogPath());
    }else{
        GlobalEventLogger::instance()->startLogging();
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

    // Connect to server
    m_comMan->connectToServer(username, password);

}

void ClientApp::logoutRequested()
{
    m_comMan->disconnectFromServer();
}

void ClientApp::on_loginResult(bool logged)
{
    if (!logged){
        m_loginDiag->setStatusMessage(tr("Utilisateur ou mot de passe invalide."),true);
    }else{
        m_loginDiag->setStatusMessage(tr("Bienvenue!"));
        showMainWindow();
    }
}

void ClientApp::on_serverDisconnected()
{
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

void ClientApp::on_networkError(QNetworkReply::NetworkError error, QString error_str)
{
    if (m_loginDiag){
        switch(error){
            case QNetworkReply::ConnectionRefusedError:
                error_str = tr("La connexion a été refusée par le serveur.");
            break;
            case QNetworkReply::AuthenticationRequiredError:
                on_loginResult(false);
                return;
            break;
            case QNetworkReply::TimeoutError:
                error_str = tr("Impossible de rejoindre le serveur.");
            break;
            case QNetworkReply::HostNotFoundError:
                error_str = tr("Le serveur est introuvable.");
            break;
            default:
                error_str = tr("Impossible de se connecter (Code erreur: ") + QString::number(error) + ")";
        }

        m_loginDiag->setStatusMessage(error_str, true);
    }
    // TODO: Manage error in main UI
}
