#ifndef WEBLOGINDIALOG_H
#define WEBLOGINDIALOG_H

#include <QDialog>
#include "TeraSettings.h"
#include "managers/ConfigManagerClient.h"
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebEngineCertificateError>

#include <QWebChannel>

class WebLoginSharedObject : public QObject {
    Q_OBJECT
public:

    Q_INVOKABLE void debugPrint(const QString &message) {
        qDebug() << "[WebLoginSharedObject::debugPrint] " << message;
    }

    Q_INVOKABLE void sendLoginSuccess(const QString &token, const QString &websocket_url, const QString &user_uuid) {
        qDebug() << "[WebLoginSharedObject::sendLoginSuccess] " << token << websocket_url;
        emit loginSuccess(token, websocket_url, user_uuid);
    }

    Q_INVOKABLE void sendLoginFailure(const QString &message) {
        emit loginFailure(message);
    }

signals:

    void loginSuccess(const QString &token, const QString &websocket_url, const QString& user_uuid);
    void loginFailure(const QString &message);

};

namespace Ui {
class WebLoginDialog;
}

class WebLoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WebLoginDialog(ConfigManagerClient *config, QWidget *parent = nullptr);
    ~WebLoginDialog();
    void setStatusMessage(const QString &message, bool error=false) {qDebug() << "Unhandled message: " << message << "error: " << error;}
    void setServerNames(QStringList servers);
    QString currentServerName();
    void showServers(bool show);

signals:

    void loginSuccess(const QString &token, const QString &websocket_url, const QString &user_uuid);
    void loginFailure(const QString &message);

private slots:

    void onCertificateError(const QWebEngineCertificateError &certificateError);
    void onServerSelected(int index);
    void onLoginPageLoaded(bool ok);
    void onLoginPageLoading();

private:
    Ui::WebLoginDialog *ui;
    QWebEngineView *m_webView;
    QWebEnginePage *m_webPage;
    ConfigManagerClient *m_config;
};

#endif // WEBLOGINDIALOG_H
