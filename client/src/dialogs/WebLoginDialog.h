#ifndef WEBLOGINDIALOG_H
#define WEBLOGINDIALOG_H

#include <QDialog>
#include "TeraSettings.h"
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

signals:

    void loginSuccess(const QString &token, const QString &websocket_url, const QString& user_uuid);

};

namespace Ui {
class WebLoginDialog;
}

class WebLoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WebLoginDialog(QWidget *parent = nullptr);
    ~WebLoginDialog();
    QString currentServerName() {return QString();}
    void setStatusMessage(const QString &message, bool error=false) {qDebug() << "Unhandled message: " << message << "error: " << error;}


private:

private slots:

    void onCertificateError(const QWebEngineCertificateError &certificateError);

signals:

    void loginSuccess(const QString &token, const QString &websocket_url, const QString &user_uuid);

private:
    Ui::WebLoginDialog *ui;
    QWebEngineView *m_webView;
    QWebEnginePage *m_webPage;
};

#endif // WEBLOGINDIALOG_H
