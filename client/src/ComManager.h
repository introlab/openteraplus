#ifndef COMMANAGER_H
#define COMMANAGER_H

#include <QObject>
#include <QWebSocket>
#include <QAbstractSocket>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QAuthenticator>
#include <QNetworkProxy>
#include <QSslPreSharedKeyAuthenticator>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QWebSocket>

#include <QJsonDocument>
#include <QJsonParseError>

#include <QUrl>
#include <QTimer>
#include <QUuid>

#include "Logger.h"
#include "WebAPI.h"

#include "TeraUser.h"


class ComManager : public QObject
{
    Q_OBJECT
public:
    explicit ComManager(QUrl serverUrl, QObject *parent = nullptr);
    ~ComManager();

    void connectToServer(QString username, QString password);

    bool processNetworkReply(QNetworkReply* reply);

    TeraUser &getCurrentUser();

protected:
    QUrl                    m_serverUrl;
    QNetworkAccessManager*  m_netManager;
    QNetworkCookieJar       m_cookieJar;
    QWebSocket*             m_webSocket;

    bool                    m_loggingInProgress;

    QString                 m_username;
    QString                 m_password;

    QTimer                  m_connectTimer;

    TeraUser                m_currentUser;

signals:
    void serverDisconnected();
    void serverError(QAbstractSocket::SocketError, QString);
    void networkError(QNetworkReply::NetworkError, QString);

    void loginResult(bool logged_in);

public slots:

private slots:
    // Websockets
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketSslErrors(const QList<QSslError> &errors);
    void onSocketTextMessageReceived(const QString &message);
    void onSocketBinaryMessageReceived(const QByteArray &message);

    // Network
    void onNetworkAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
    void onNetworkEncrypted(QNetworkReply *reply);
    void onNetworkFinished(QNetworkReply *reply);
    void onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);
    void onNetworkSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

    void onTimerConnectTimeout();
};

#endif // COMMANAGER_H
