#ifndef CLIENTAPP_H
#define CLIENTAPP_H

#include <QObject>
#include <QApplication>
#include <QFile>
#include <QAbstractSocket> // For error codes
#include <QUuid>
#include <QTranslator>
#include <QStandardPaths>
#include <QLibraryInfo>

#include "main/MainWindow.h"
#ifndef OPENTERA_WEBASSEMBLY
#include "main/MainKitWindow.h"
#endif

#ifndef OPENTERA_WEBASSEMBLY
#include "dialogs/WebLoginDialog.h"
#endif

#include "dialogs/LoginDialog.h"
#include "GlobalMessageBox.h"

#include "managers/ComManager.h"
#include "managers/ConfigManagerClient.h"

class ClientApp : public QApplication
{
    Q_OBJECT
public:
    ClientApp(int &argc, char** argv);
    virtual ~ClientApp() override;
    ComManager *getComManager();

protected:
    void loadConfig();
    void connectSignals();
    void showLogin();
    void showMainWindow();
#ifndef OPENTERA_WEBASSEMBLY
    void showMainKitWindow();
#endif
    void setupLogger();

    void processQueuedEvents();

    void setTranslation(QString language = "");

    ConfigManagerClient m_config;

#ifndef OPENTERA_WEBASSEMBLY
    WebLoginDialog *m_loginDiag;
#else
    LoginDialog*        m_loginDiag;
#endif

    MainWindow*         m_mainWindow;
#ifndef OPENTERA_WEBASSEMBLY
    MainKitWindow*      m_mainKitWindow;
#endif

    QList<TeraEvent>  m_eventQueue; // Queue to stack missed events when just connected, but no MainWindow yet.

    ComManager*         m_comMan;
    QTranslator*        m_translator;
    QTranslator*        m_qt_translator;
    QLocale             m_currentLocale;


private slots:
    void loginRequested(QString username, QString password, QString server_name);
    void logoutRequested();
    void on_loginResult(bool logged, QString log_msg);
    void loginQuitRequested();

    void on_serverDisconnected();
    void on_serverError(QAbstractSocket::SocketError error, QString error_str);
    void on_networkError(QNetworkReply::NetworkError error, QString error_str, QNetworkAccessManager::Operation, int status_code);

    void on_newVersionAvailable(QString version, QString download_url);

    void on_currentUserUpdated();

    void ws_genericEventReceived(opentera::protobuf::TeraEvent event);


    void preferencesUpdated();
};

#endif // CLIENTAPP_H
