#ifndef WEBSOCKETMANAGER_H
#define WEBSOCKETMANAGER_H

#include <QObject>
#include <QWebSocket>
#include <QAbstractSocket>
#include <QTimer>
#include "Logger.h"

// Protobuf includes
#include "UserRegisterToEvent.pb.h"
#include "UserEvent.pb.h"
#include "ParticipantEvent.pb.h"
#include "JoinSessionEvent.pb.h"
#include "JoinSessionReply.pb.h"
#include "TeraEvent.pb.h"
#include "TeraMessage.pb.h"
#include "TeraModuleMessage.pb.h"
#include "DatabaseEvent.pb.h"

#include "google/protobuf/any.h"
#include "google/protobuf/util/json_util.h"

using namespace opentera::protobuf;

class WebSocketManager : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketManager(QObject *parent = nullptr);
    ~WebSocketManager();

    void connectWebSocket(QString &socketUrl, QString &user_uuid);
    void disconnectWebSocket();
    void registerForEvent(const UserRegisterToEvent_EventType event_type);
    void unregisterFromEvent(const UserRegisterToEvent_EventType event_type);

    void sendJoinSessionReply(const QString& session_uuid, const JoinSessionReply::ReplyType reply_type);

private:
    TeraModuleMessage_Header*   buildMessageHeader();
    void sendModuleMessage(const google::protobuf::Message &data_msg);

    quint32                 m_seq_num;

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

    void userEventReceived(UserEvent event);
    void participantEventReceived(ParticipantEvent event);
    void joinSessionEventReceived(JoinSessionEvent event);
    void databaseEventReceived(DatabaseEvent event);


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
