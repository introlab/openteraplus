#ifndef WEBLOGINDIALOG_H
#define WEBLOGINDIALOG_H

#include <QDialog>

#include "managers/ConfigManagerClient.h"
#include "libs/WebPageRequestInterceptor.h"

#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebEngineCertificateError>
#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineLoadingInfo>

#include <QWebChannel>

class WebLoginSharedObject : public QObject {
    Q_OBJECT
public:

    Q_INVOKABLE void debugPrint(const QString &message) {
        qDebug() << "[WebLoginSharedObject::debugPrint] " << message;
    }

    Q_INVOKABLE void sendLoginSuccess(const QString &token, const QString &websocket_url, const QString &user_uuid) {
        //qDebug() << "[WebLoginSharedObject::sendLoginSuccess] " << token << websocket_url;
        emit loginSuccess(token, websocket_url, user_uuid);
    }

    Q_INVOKABLE void sendLoginFailure(const QString &message) {
        emit loginFailure(message);
    }

    Q_INVOKABLE void sendSetupInProgress() {
        emit mfaSetupInProgress();
    }

    Q_INVOKABLE void sendCheckInProgress() {
        emit mfaCheckInProgress();
    }

    Q_INVOKABLE void sendPasswordChangeInProgress(){
        emit passwordChangeInProgress();
    }

    Q_INVOKABLE void sendRedirectToLogin(){
        emit redirectToLogin();
    }

signals:

    void loginSuccess(const QString &token, const QString &websocket_url, const QString& user_uuid);
    void loginFailure(const QString &message);
    void redirectToLogin();

    void mfaSetupInProgress();
    void mfaCheckInProgress();

    void passwordChangeInProgress();

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


    void setStatusMessage(const QString &message, const bool& error=false);
    void setServerNames(QStringList servers);
    QString currentServerName();
    void showServers(bool show);

signals:

    void loginSuccess(const QString &token, const QString &websocket_url, const QString &user_uuid);
    void loginFailure(const QString &message);

    void loginRequest(QString username, QString password, QString server_name);

private slots:

    void onCertificateError(const QWebEngineCertificateError &certificateError);
    void onServerSelected(int index);
    void onLoginPageLoadingChanged(const QWebEngineLoadingInfo &loadingInfo);
    void onLoginSuccess(const QString &token, const QString &websocket_url, const QString &user_uuid);

    void onMfaSetupInProgress();
    void onMfaCheckInProgress();
    void onPasswordChangeInProgress();

    void onLoginFailed(const QString &message);
    void onRedirectToLoginRequest();

    void on_btnCancel_clicked();
    void on_btnRetry_clicked();

    void onStandardLoginRequested(QString username, QString password);

private:
    Ui::WebLoginDialog          *ui;
    QWebEngineView              *m_webView = nullptr;
    QWebEnginePage              *m_webPage = nullptr;
    WebPageRequestInterceptor   *m_requestInterceptor = nullptr;
    ConfigManagerClient         *m_config = nullptr;

    bool                        m_showServers = false;


    void showLargeView(const bool& large);
};


#endif // WEBLOGINDIALOG_H
