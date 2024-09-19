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
    Q_INVOKABLE void handleToken(const QString &token) {
        qDebug() << "Token received from web page: " << token;
        // Use the token in your Qt application
    }
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

private:
    Ui::WebLoginDialog *ui;
    QWebEngineView *m_webView;
    QWebEnginePage *m_webPage;
};

#endif // WEBLOGINDIALOG_H
