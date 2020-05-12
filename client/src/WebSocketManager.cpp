#include "WebSocketManager.h"
#include <QDebug>

WebSocketManager::WebSocketManager(QObject *parent) : QObject(parent)
{
    // Check protobuf version
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // Initialize communication objects
    m_webSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, parent);
    m_connectTimer.setSingleShot(true);

    // Set Keep Alive signal for websocket
    m_keepAliveTimer.setInterval(30000);

    // Setup signals and slots
    // Websocket
    connect(m_webSocket, &QWebSocket::connected, this, &WebSocketManager::onSocketConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &WebSocketManager::onSocketDisconnected);
    connect(m_webSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));
    connect(m_webSocket, &QWebSocket::sslErrors, this, &WebSocketManager::onSocketSslErrors);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketManager::onSocketTextMessageReceived);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, &WebSocketManager::onSocketBinaryMessageReceived);

    // Other objects
    connect(&m_connectTimer, &QTimer::timeout, this, &WebSocketManager::onTimerConnectTimeout);
    connect(&m_keepAliveTimer, &QTimer::timeout, this, &WebSocketManager::onTimerKeepAliveTimeout);
}

WebSocketManager::~WebSocketManager()
{
    m_webSocket->deleteLater();
}

void WebSocketManager::connectWebSocket(QString &socketUrl)
{
    m_connectTimer.setInterval(60000); //TODO: Reduce this delay - was set that high because of the time required to connect in MAC OS
    m_connectTimer.start();
    m_socketUrl = QUrl(socketUrl);
    m_webSocket->open(m_socketUrl);
}

void WebSocketManager::registerForEvent(const opentera::protobuf::UserRegisterToEvent_EventType event_type)
{
    opentera::protobuf::UserRegisterToEvent user_event;

    user_event.set_event_type(event_type);
    user_event.set_action(opentera::protobuf::UserRegisterToEvent_Action_REGISTER);

    qDebug() << QString::fromStdString(user_event.DebugString());

}

void WebSocketManager::unregisterFromEvent(const opentera::protobuf::UserRegisterToEvent_EventType event_type)
{
    opentera::protobuf::UserRegisterToEvent user_event;

    user_event.set_event_type(event_type);
    user_event.set_action(opentera::protobuf::UserRegisterToEvent_Action_UNREGISTER);
    qDebug() << QString::fromStdString(user_event.DebugString());
}



//////////////////////////////////////////////////////////////////////
void WebSocketManager::onSocketError(QAbstractSocket::SocketError error)
{
    qDebug() << "ComManager::Socket error - " << error;
    emit websocketError(error, m_webSocket->errorString());
}

void WebSocketManager::onSocketConnected()
{
    m_connectTimer.stop();
    m_keepAliveTimer.start();
    emit loginResult(true); // Logged in

    // Test purpose
    registerForEvent(opentera::protobuf::UserRegisterToEvent_EventType_USER_CONNECTED);
}

void WebSocketManager::onSocketDisconnected()
{
    m_connectTimer.stop();
    m_keepAliveTimer.stop();
    qDebug() << "WebSocketManager::Disconnected from " << m_socketUrl.toString();
    emit serverDisconnected();
}

void WebSocketManager::onSocketSslErrors(const QList<QSslError> &errors)
{
    Q_UNUSED(errors)

    // WARNING: Never ignore SSL errors in production code.
    // The proper way to handle self-signed certificates is to add a custom root
    // to the CA store.
    qDebug() << "WebSocketManager::SSlErrors " << errors;
    m_webSocket->ignoreSslErrors();
}

void WebSocketManager::onSocketTextMessageReceived(const QString &message)
{
    LOG_DEBUG(message, "WebSocketManager::onSocketTextMessageReceived");
}

void WebSocketManager::onSocketBinaryMessageReceived(const QByteArray &message)
{
    Q_UNUSED(message)
}


void WebSocketManager::onTimerConnectTimeout()
{
    // Connection timeout
    if (m_webSocket){
        qDebug() << "ComManager::onTimerConnectTimeout()";
        m_webSocket->abort();
        emit websocketError(QAbstractSocket::SocketTimeoutError, tr("Le serveur ne répond pas."));
    }

}

void WebSocketManager::onTimerKeepAliveTimeout()
{
    if (m_webSocket){
        m_webSocket->ping();
    }
}
