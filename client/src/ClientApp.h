#ifndef CLIENTAPP_H
#define CLIENTAPP_H

#include <QObject>
#include <QApplication>
#include <QFile>
#include <QAbstractSocket> // For error codes
#include <QUuid>
#include <QTranslator>
#include <QStandardPaths>

#include "MainWindow.h"
#include "dialogs/LoginDialog.h"
#include "GlobalMessageBox.h"

#include "ComManager.h"
#include "ConfigManagerClient.h"

class ClientApp : public QApplication
{
    Q_OBJECT
public:
    ClientApp(int &argc, char** argv);
    ~ClientApp();

    ComManager *getComManager();

protected:
    void loadConfig();
    void connectSignals();
    void showLogin();
    void showMainWindow();
    void setupLogger();

    void processQueuedEvents();

    void setTranslation(QString language = "");

    ConfigManagerClient m_config;
    LoginDialog*        m_loginDiag;
    MainWindow*         m_mainWindow;

    QList<TeraEvent>  m_eventQueue; // Queue to stack missed events when just connected, but no MainWindow yet.

    ComManager*         m_comMan;
    QTranslator*        m_translator;
    QLocale             m_currentLocale;


private slots:
    void loginRequested(QString username, QString password, QString server_name);
    void logoutRequested();
    void on_loginResult(bool logged);

    void on_serverDisconnected();
    void on_serverError(QAbstractSocket::SocketError error, QString error_str);
    void on_networkError(QNetworkReply::NetworkError error, QString error_str, QNetworkAccessManager::Operation, int status_code);

    void on_newVersionAvailable(QString version, QString download_url);

    void on_currentUserUpdated();

    void ws_genericEventReceived(TeraEvent event);


    void preferencesUpdated();
};

#endif // CLIENTAPP_H
