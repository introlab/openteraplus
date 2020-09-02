#ifndef CLIENTAPP_H
#define CLIENTAPP_H

#include <QObject>
#include <QApplication>
#include <QFile>
#include <QAbstractSocket> // For error codes
#include <QUuid>
#include <QTranslator>

#include "MainWindow.h"
#include "LoginDialog.h"

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

    void setTranslation(QString language = "");

    ConfigManagerClient m_config;
    LoginDialog*        m_loginDiag;
    MainWindow*         m_mainWindow;

    ComManager*         m_comMan;
    QTranslator*        m_translator;
    QLocale             m_currentLocale;


private slots:
    void loginRequested(QString username, QString password, QString server_name);
    void logoutRequested();
    void on_loginResult(bool logged);

    void on_serverDisconnected();
    void on_serverError(QAbstractSocket::SocketError error, QString error_str);
    void on_networkError(QNetworkReply::NetworkError error, QString error_str);

    void preferencesUpdated();
};

#endif // CLIENTAPP_H
