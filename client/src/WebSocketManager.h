#ifndef WEBSOCKETMANAGER_H
#define WEBSOCKETMANAGER_H

#include <QObject>
#include <QWebSocket>
#include <QAbstractSocket>
#include <QTimer>
#include "Logger.h"

// Protobuf includes
#include "UserRegisterToEvent.pb.h"
#include "TeraMessage.pb.h"
#include "TeraModuleMessage.pb.h"

using namespace opentera::protobuf;

class WebSocketManager : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketManager(QObject *parent = nullptr);
    ~WebSocketManager();

    void connectWebSocket(QString &socketUrl, QString &user_uuid);
    void registerForEvent(const UserRegisterToEvent_EventType event_type);
    void unregisterFromEvent(const UserRegisterToEvent_EventType event_type);

private:
    TeraModuleMessage_Header*   buildMessageHeader();

    quint32 seq_num;

protected:
    QUrl                    m_socketUrl;
    QWebSocket*             m_webSocket;
    QString                 m_user_uuid;

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
