#include "WebSocketManager.h"

WebSocketManager::WebSocketManager(QObject *parent) : QObject(parent)
{
    // Check protobuf version
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // Initialize communication objects
    m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, parent);
    m_connectTimer.setSingleShot(true);
    m_seq_num = 0;

    // Set Keep Alive signal for websocket
    m_keepAliveTimer.setInterval(30000);

    // Setup signals and slots
    // Websocket
    connect(m_webSocket, &QWebSocket::connected, this, &WebSocketManager::onSocketConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &WebSocketManager::onSocketDisconnected);
    connect(m_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
#ifndef OPENTERA_WEBASSEMBLY
    connect(m_webSocket, &QWebSocket::sslErrors, this, &WebSocketManager::onSocketSslErrors);
#endif
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketManager::onSocketTextMessageReceived);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, &WebSocketManager::onSocketBinaryMessageReceived);

    // Other objects
    connect(&m_connectTimer, &QTimer::timeout, this, &WebSocketManager::onTimerConnectTimeout);
    connect(&m_keepAliveTimer, &QTimer::timeout, this, &WebSocketManager::onTimerKeepAliveTimeout);
}

WebSocketManager::~WebSocketManager()
{
    disconnectWebSocket();
    m_webSocket->deleteLater();
}

void WebSocketManager::connectWebSocket(QString &socketUrl, QString &user_uuid)
{
    m_user_uuid = user_uuid;
    m_connectTimer.setInterval(60000); //TODO: Reduce this delay - was set that high because of the time required to connect in MAC OS
    m_connectTimer.start();
    m_socketUrl = QUrl(socketUrl);
    m_webSocket->open(m_socketUrl);
}

void WebSocketManager::disconnectWebSocket()
{
    m_webSocket->close();
    //while(m_webSocket->state() == QAbstractSocket::ConnectedState);
}
/*
void WebSocketManager::registerForEvent(const opentera::protobuf::UserRegisterToEvent_EventType event_type)
{

    // Data
    UserRegisterToEvent user_event;
    user_event.set_event_type(event_type);
    user_event.set_action(UserRegisterToEvent_Action_ACTION_REGISTER);

    sendModuleMessage(user_event);


}

void WebSocketManager::unregisterFromEvent(const UserRegisterToEvent_EventType event_type)
{

    // Data
    UserRegisterToEvent user_event;
    user_event.set_event_type(event_type);
    user_event.set_action(UserRegisterToEvent_Action_ACTION_UNREGISTER);

    sendModuleMessage(user_event);

}


TeraModuleMessage_Header *WebSocketManager::buildMessageHeader()
{
    TeraModuleMessage_Header* tera_msg_head = new TeraModuleMessage_Header();
    tera_msg_head->set_version(1);
    tera_msg_head->set_time(static_cast<double>(QDateTime::currentDateTime().toMSecsSinceEpoch())/1000);
    tera_msg_head->set_seq(++m_seq_num);
    QString dest = "websocket.user." + m_user_uuid;
    QString identity = "client.teraplus." + m_user_uuid;

    tera_msg_head->set_dest(dest.toStdString());
    tera_msg_head->set_source(identity.toStdString());

    return tera_msg_head;
}

void WebSocketManager::sendModuleMessage(const google::protobuf::Message &data_msg)
{
    TeraModuleMessage* tera_msg = new TeraModuleMessage();

    // Header
    TeraModuleMessage_Header* tera_msg_head = buildMessageHeader();
    tera_msg->set_allocated_head(tera_msg_head);

    google::protobuf::Any* data = tera_msg->add_data();
    data->PackFrom(data_msg);

    // Sending
    std::string json_msg;
    google::protobuf::util::MessageToJsonString(*tera_msg, &json_msg);
    m_webSocket->sendTextMessage(QString::fromStdString(json_msg));

    // Clean up - since ownership was passed to the object, deleting it will clear all related ones.
    delete tera_msg;
}*/


//////////////////////////////////////////////////////////////////////
void WebSocketManager::onSocketError(QAbstractSocket::SocketError error)
{
    LOG_DEBUG("Socket error - " + m_webSocket->errorString(), "WebSocketManager::onSocketError");
    emit websocketError(error, m_webSocket->errorString());
}

void WebSocketManager::onSocketConnected()
{
    //qDebug() << "WebSocketManager::onSocketConnected()";
    m_connectTimer.stop();
    m_keepAliveTimer.start();
    emit loginResult(true); // Logged in

    // Test purpose
    // registerForEvent(UserRegisterToEvent_EventType_EVENT_USER_CONNECTED);
}

void WebSocketManager::onSocketDisconnected()
{
    m_connectTimer.stop();
    m_keepAliveTimer.stop();
    LOG_DEBUG("WebSocketManager::Disconnected from " + m_socketUrl.toString(), "WebSocketManager::onSocketDisconnected");
    emit serverDisconnected();
}

#ifndef OPENTERA_WEBASSEMBLY
void WebSocketManager::onSocketSslErrors(const QList<QSslError> &errors)
{
    Q_UNUSED(errors)

    // WARNING: Never ignore SSL errors in production code.
    // The proper way to handle self-signed certificates is to add a custom root
    // to the CA store.
    qDebug() << "WebSocketManager::SSlErrors " << errors;
    m_webSocket->ignoreSslErrors();
}
#endif

void WebSocketManager::onSocketTextMessageReceived(const QString &message)
{
    LOG_DEBUG(message, "WebSocketManager::onSocketTextMessageReceived");

    // Handle received message depending on type
    TeraMessage tera_msg;
    if (google::protobuf::util::JsonStringToMessage(message.toStdString(), &tera_msg).ok()){
        // TeraMessage
        if (tera_msg.message().Is<TeraEvent>()){
            TeraEvent tera_event;
            if (!tera_msg.message().UnpackTo(&tera_event)){
                LOG_ERROR("Error unpacking TeraEvent from TeraMessage.", "WebSocketManager::onSocketTextMessageReceived");
                return;
            }

            // Process received events
           for(int i=0; i< tera_event.events_size(); i++){
               ///// User event
               if (tera_event.events(i).Is<UserEvent>()){
                   UserEvent user_event;
                   if (tera_event.events(i).UnpackTo(&user_event)){
                       emit userEventReceived(user_event);
                   }else{
                       LOG_ERROR("Error unpacking UserEvent from TeraEvent.", "WebSocketManager::onSocketTextMessageReceived");
                   }
               }

               ///// Participant event
               if (tera_event.events(i).Is<ParticipantEvent>()){
                   ParticipantEvent part_event;
                   if (tera_event.events(i).UnpackTo(&part_event)){
                       emit participantEventReceived(part_event);
                   }else{
                       LOG_ERROR("Error unpacking ParticipantEvent from TeraEvent.", "WebSocketManager::onSocketTextMessageReceived");
                   }
               }

               ///// Device event
               if (tera_event.events(i).Is<DeviceEvent>()){
                   DeviceEvent device_event;
                   if (tera_event.events(i).UnpackTo(&device_event)){
                       emit deviceEventReceived(device_event);
                   }else{
                       LOG_ERROR("Error unpacking DeviceEvent from TeraEvent.", "WebSocketManager::onSocketTextMessageReceived");
                   }
               }

               ///// Join session event
               if (tera_event.events(i).Is<JoinSessionEvent>()){
                   JoinSessionEvent event;
                   if (tera_event.events(i).UnpackTo(&event)){
                       emit joinSessionEventReceived(event);
                   }else{
                       LOG_ERROR("Error unpacking JoinSessionEvent from TeraEvent.", "WebSocketManager::onSocketTextMessageReceived");
                   }
               }

               ///// Leave session event
               if (tera_event.events(i).Is<LeaveSessionEvent>()){
                   LeaveSessionEvent event;
                   if (tera_event.events(i).UnpackTo(&event)){
                       emit leaveSessionEventReceived(event);
                   }else{
                       LOG_ERROR("Error unpacking LeaveSessionEvent from TeraEvent.", "WebSocketManager::onSocketTextMessageReceived");
                   }
               }

               ///// Stop session event
               if (tera_event.events(i).Is<StopSessionEvent>()){
                   StopSessionEvent event;
                   if (tera_event.events(i).UnpackTo(&event)){
                       emit stopSessionEventReceived(event);
                   }else{
                       LOG_ERROR("Error unpacking StopSessionEvent from TeraEvent.", "WebSocketManager::onSocketTextMessageReceived");
                   }
               }

               ///// Database Event
               if (tera_event.events(i).Is<DatabaseEvent>()){
                   DatabaseEvent event;
                   if (tera_event.events(i).UnpackTo(&event)){
                       emit databaseEventReceived(event);
                   }else{
                       LOG_ERROR("Error unpacking DatabaseEvent from TeraEvent.", "WebSocketManager::onSocketTextMessageReceived");
                   }
               }

               ///// Join Session Reply Event
               if (tera_event.events(i).Is<JoinSessionReplyEvent>()){
                   JoinSessionReplyEvent event;
                   if (tera_event.events(i).UnpackTo(&event)){
                       emit joinSessionReplyEventReceived(event);
                   }else{
                       LOG_ERROR("Error unpacking JoinSessionReplyEvent from TeraEvent.", "WebSocketManager::onSocketTextMessageReceived");
                   }
               }

               emit genericEventReceived(tera_event);
               return;
           }
        }else{
            LOG_WARNING("Got unhandled message on websocket", "WebSocketManager::onSocketTextMessageReceived");
        }
    }else{
        LOG_ERROR("Unable to decode received protobuf message", "WebSocketManager::onSocketTextMessageReceived");
    }
}

void WebSocketManager::onSocketBinaryMessageReceived(const QByteArray &message)
{
    Q_UNUSED(message)
}


void WebSocketManager::onTimerConnectTimeout()
{
    // Connection timeout
    if (m_webSocket){
        //qDebug() << "ComManager::onTimerConnectTimeout()";
        m_webSocket->abort();
        emit websocketError(QAbstractSocket::SocketTimeoutError, tr("Le serveur ne répond pas."));
    }

}

void WebSocketManager::onTimerKeepAliveTimeout()
{
    // qDebug() << "Ping";
    if (m_webSocket){
        m_webSocket->ping();
    }
}
