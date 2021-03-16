#include "MainKitWindow.h"
#include "ui_MainKitWindow.h"

MainKitWindow::MainKitWindow(ConfigManagerClient *config, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainKitWindow)
{
    ui->setupUi(this);

    m_config = config;
    m_loginDiag = nullptr;
    m_comManager = nullptr;
    m_configDiag = nullptr;

    // Load config file

    // Initialize main UI
    initUi();

}

MainKitWindow::~MainKitWindow()
{
    delete ui;

    if (m_comManager){
        m_comManager->deleteLater();
    }

    if (m_loginDiag){
        m_loginDiag->deleteLater();
    }

    if (m_configDiag){
        m_configDiag->deleteLater();
    }
}

void MainKitWindow::userLoginRequested(QString username, QString password, QString server_name)
{
    // Find server url for that server
    QUrl server = m_config->getServerUrl(server_name);

    // Create ComManager for that server
    if (m_comManager){
        m_comManager->deleteLater();
    }
    m_comManager = new ComManager(server);

    // Connect signals
    connect(m_comManager, &ComManager::socketError,         this, &MainKitWindow::on_serverError);
    connect(m_comManager, &ComManager::serverDisconnected,  this, &MainKitWindow::on_serverDisconnected);
    connect(m_comManager, &ComManager::loginResult,         this, &MainKitWindow::on_loginResult);
    connect(m_comManager, &ComManager::networkError,        this, &MainKitWindow::on_networkError);
    connect(m_comManager, &ComManager::newVersionAvailable, this, &MainKitWindow::on_newVersionAvailable);

    // Connect to server
    m_comManager->connectToServer(username, password);
}

void MainKitWindow::on_loginResult(bool logged)
{
    if (m_loginDiag){
        if (!logged){
            m_loginDiag->setStatusMessage(tr("Utilisateur ou mot de passe invalide."),true);
        }else{
            m_loginDiag->setStatusMessage(tr("Bienvenue!"));
            showConfigDialog();
        }
    }
}

void MainKitWindow::userLoginCancelled()
{
    if (m_loginDiag){
        m_loginDiag->deleteLater();
        m_loginDiag = nullptr;
    }

    if (m_comManager){
        m_comManager->disconnectFromServer();
        m_comManager->deleteLater();
        m_comManager = nullptr;
    }

    initUi();
}

void MainKitWindow::on_serverDisconnected()
{
    closeConfigDialog();
}

void MainKitWindow::on_serverError(QAbstractSocket::SocketError error, QString error_str)
{
    if (m_loginDiag){
        if (error == QAbstractSocket::ConnectionRefusedError)
            error_str = tr("La connexion a été refusée par le serveur.");
        m_loginDiag->setStatusMessage(error_str, true);
    }
}

void MainKitWindow::on_networkError(QNetworkReply::NetworkError error, QString error_str, QNetworkAccessManager::Operation op, int status_code)
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
}

void MainKitWindow::on_newVersionAvailable(QString version, QString download_url)
{
    if (Utils::isNewerVersion(version))
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

void MainKitWindow::setWindowOnTop(bool top)
{
    if (top)
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    else
        setWindowFlags(Qt::FramelessWindowHint);
}

void MainKitWindow::initUi()
{
    // Version label
    ui->lblVersion->setText(OPENTERAPLUS_VERSION);

    // Reboot button
#ifndef Q_OS_WIN
    ui->btnReboot->hide(); // Reboot is not yet implemented on OS others than Windows.
#endif

    // Always on top
    setWindowOnTop(true);

    // Full screen mode
    showFullScreen();
}

void MainKitWindow::showLogin()
{
    setWindowOnTop(false);
    showMaximized();
    if (m_loginDiag == nullptr){
        m_loginDiag = new LoginDialog();
        connect(m_loginDiag, &LoginDialog::loginRequest, this, &MainKitWindow::userLoginRequested);
        connect(m_loginDiag, &LoginDialog::quitRequest, this, &MainKitWindow::userLoginCancelled);

        // Set server names
        m_loginDiag->setServerNames(m_config->getServerNames());

        // Show servers list... or not!
        m_loginDiag->showServers(m_config->showServers());
    }

    m_loginDiag->show();
}

void MainKitWindow::showConfigDialog()
{
    if (m_configDiag){
        m_configDiag->deleteLater();
    }

    if (m_loginDiag){
        m_loginDiag->deleteLater();
        m_loginDiag = nullptr;
    }

    m_configDiag = new KitConfigDialog(m_comManager, this);
    connect(m_configDiag, &KitConfigDialog::finished, this, &MainKitWindow::closeConfigDialog);

    m_configDiag->showMaximized();
}

void MainKitWindow::closeConfigDialog()
{
    if (m_configDiag){
        m_configDiag->hide();
        m_configDiag->deleteLater();
        m_configDiag = nullptr;
    }

    if (m_comManager){
        //m_comManager->disconnectFromServer();
        m_comManager->deleteLater();
        m_comManager = nullptr;
    }

    // Refresh UI
    initUi();
}

void MainKitWindow::on_btnExit_clicked()
{
    QApplication::quit();
}

void MainKitWindow::on_btnReboot_clicked()
{
#ifdef Q_OS_WIN

    // Setting reboot privileges for this process
    HANDLE           hToken;
    TOKEN_PRIVILEGES tkp   ;

    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken);
    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;                                 // Set 1 privilege
    tkp.Privileges[0].Attributes= SE_PRIVILEGE_ENABLED;

    // Get the shutdown privilege for this process
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

    if ( !ExitWindowsEx(EWX_REBOOT|EWX_FORCE,SHTDN_REASON_MAJOR_SOFTWARE))
    {
      // Failed, call GetLastError() to know why
    }
#else
    qDebug() << "SYSTEM REBOOT UNSUPPORTED ON THIS OS.";
#endif

    // TODO: support other OS
}

void MainKitWindow::on_btnConfig_clicked()
{
    if (!m_configDiag)
        showLogin();
}
