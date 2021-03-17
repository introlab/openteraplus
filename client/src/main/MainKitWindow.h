#ifndef MAINKITWINDOW_H
#define MAINKITWINDOW_H

#include <QMainWindow>
#include <QStandardPaths>

#include "managers/ComManager.h"
#include "managers/ConfigManagerClient.h"

#include "dialogs/LoginDialog.h"

#include "kit/KitConfigDialog.h"
#include "kit/KitConfigManager.h"

#include "Utils.h"

#include "GlobalMessageBox.h"

#ifdef Q_OS_WIN
    #include "qt_windows.h"
#endif

namespace Ui {
class MainKitWindow;
}

class MainKitWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainKitWindow(ConfigManagerClient* config, QWidget *parent = nullptr);
    ~MainKitWindow();

private slots:
    void userLoginRequested(QString username, QString password, QString server_name);
    void userLoginCancelled();
    void on_loginResult(bool logged);

    void on_serverDisconnected();
    void on_serverError(QAbstractSocket::SocketError error, QString error_str);
    void on_networkError(QNetworkReply::NetworkError error, QString error_str, QNetworkAccessManager::Operation, int status_code);
    void on_newVersionAvailable(QString version, QString download_url);

    void on_btnExit_clicked();
    void on_btnReboot_clicked();
    void on_btnConfig_clicked();

    void closeConfigDialog();

private:
    void setWindowOnTop(bool top);
    void initUi();
    void loadConfig();

    void showLogin();
    void showConfigDialog();


    Ui::MainKitWindow*      ui;
    ComManager*             m_comManager;
    LoginDialog*            m_loginDiag;
    ConfigManagerClient*    m_config;
    KitConfigDialog*        m_configDiag;
    KitConfigManager        m_kitConfig;
};

#endif // MAINKITWINDOW_H
