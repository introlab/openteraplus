#ifndef WEBSOCKETMANAGER_H
#define WEBSOCKETMANAGER_H

#include <QObject>
#include <QWebSocket>
#include <QAbstractSocket>
#include <QTimer>
#include "Logger.h"
#include "UserRegisterToEvent.pb.h"


class WebSocketManager : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketManager(QObject *parent = nullptr);
    ~WebSocketManager();

    void connectWebSocket(QString &socketUrl);
    void registerForEvent(const opentera::protobuf::UserRegisterToEvent_EventType event_type);
    void unregisterFromEvent(const opentera::protobuf::UserRegisterToEvent_EventType event_type);


protected:
    QUrl                    m_socketUrl;
    QWebSocket*             m_webSocket;

    QTimer                  m_connectTimer;
    QTimer                  m_keepAliveTimer;


signals:
    void serverDisconnected();
    void websocketError(QAbstractSocket::SocketError, QString);
    void loginResult(bool logged_in);


private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketSslErrors(const QList<QSslError> &errors);
    void onSocketTextMessageReceived(const QString &message);
    void onSocketBinaryMessageReceived(const QByteArray &message);

    void onTimerConnectTimeout();
    void onTimerKeepAliveTimeout();


};

#endif // WEBSOCKETMANAGER_H
