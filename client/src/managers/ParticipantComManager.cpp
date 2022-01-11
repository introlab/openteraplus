#include "ParticipantComManager.h"
#include <sstream>
#include <QLocale>
#include "Utils.h"

ParticipantComManager::ParticipantComManager(QUrl serverUrl, bool connectWebsocket, QObject *parent) :
    BaseComManager(serverUrl, parent),
    m_currentParticipant(TERADATA_PARTICIPANT)
{
    m_enableWebsocket = connectWebsocket;

    // Setup signals and slots
    if (m_enableWebsocket){
        m_webSocketMan = new WebSocketManager();
        // Websocket manager
        connect(m_webSocketMan, &WebSocketManager::serverDisconnected, this, &ParticipantComManager::serverDisconnected); // Pass-thru signal
        connect(m_webSocketMan, &WebSocketManager::websocketError, this, &ParticipantComManager::socketError); // Pass-thru signal
        connect(m_webSocketMan, &WebSocketManager::loginResult, this, &ParticipantComManager::onWebSocketLoginResult);
    }

}

ParticipantComManager::~ParticipantComManager()
{
    if (m_enableWebsocket){
        m_webSocketMan->disconnectWebSocket();
        m_webSocketMan->deleteLater();
    }
}

void ParticipantComManager::connectToServer(QString token)
{
    LOG_DEBUG("ParticipantComManager::Connecting to " + m_serverUrl.toString(), "ParticipantComManager::connectToServer");

    setCredentials(token);

    doGet(QString(WEB_PARTICIPANT_LOGIN_PATH));

}

void ParticipantComManager::disconnectFromServer()
{
    doGet(QString(WEB_PARTICIPANT_LOGOUT_PATH));
    if (m_enableWebsocket){
        m_webSocketMan->disconnectWebSocket();
    }

    clearCurrentParticipant();
}

bool ParticipantComManager::processNetworkReply(QNetworkReply *reply)
{
    QString reply_path = reply->url().path();
    QString reply_data = reply->readAll();
    QUrlQuery reply_query = QUrlQuery(reply->url().query());
    //qDebug() << reply_path << " ---> " << reply_data << ": " << reply->url().query();

    bool handled = false;

    if (reply->operation()==QNetworkAccessManager::GetOperation){
        if (reply_path == WEB_PARTICIPANT_LOGIN_PATH){
            // Initialize cookies
            m_cookieJar.cookiesForUrl(reply->url());
            handled=handleLoginReply(reply_data);
        }

        if (reply_path == WEB_PARTICIPANT_LOGOUT_PATH){
            emit serverDisconnected();
            handled = true;
        }

        if (!handled){
            // General case
            handled = handleDataReply(reply_path, reply_data, reply_query);
            if (handled) emit queryResultsOK(reply_path, reply_query);
        }
    }

    if (reply->operation()==QNetworkAccessManager::PostOperation){
        if (!handled){
            handled=handleDataReply(reply_path, reply_data, reply_query);
            if (handled) emit postResultsOK(reply_path);
        }
    }

    if (reply->operation()==QNetworkAccessManager::DeleteOperation){
        // Extract id from url
        int id = 0;
        if (reply_query.hasQueryItem("id")){
            id = reply_query.queryItemValue("id").toInt();
        }
        emit deleteResultsOK(reply_path, id);
        handled=true;
    }

    return handled;
}

void ParticipantComManager::doGet(const QString &path, const QUrlQuery &query_args, const bool &use_token)
{
    Q_UNUSED(use_token)
    BaseComManager::doGet(path, query_args, true); // Always use token!
}

void ParticipantComManager::doPost(const QString &path, const QString &post_data, const bool &use_token)
{
    Q_UNUSED(use_token)
    BaseComManager::doPost(path, post_data, true); // Always use token!
}

void ParticipantComManager::doDelete(const QString &path, const int &id, const bool &use_token)
{
    Q_UNUSED(use_token)
    BaseComManager::doDelete(path, id, true); // Always use token!
}

void ParticipantComManager::doUpdateCurrentParticipant()
{
    /*QUrlQuery args;
    doQuery(QString(WEB_PARTICIPANT_PARTICIPANTINFO_PATH), args);*/
}

TeraData &ParticipantComManager::getCurrentParticipant()
{
    return m_currentParticipant;
}

WebSocketManager *ParticipantComManager::getWebSocketManager()
{
    return m_webSocketMan;
}

ParticipantComManager::signal_ptr ParticipantComManager::getSignalFunctionForDataType(const TeraDataTypes &data_type)
{
    switch(data_type){
    case TERADATA_NONE:
        LOG_ERROR("Unknown object - no signal associated.", "ParticipantComManager::getSignalFunctionForDataType");
        return nullptr;
    case TERADATA_PARTICIPANT:
        return &ParticipantComManager::participantsReceived;
    default:
        LOG_WARNING("Signal for object " + TeraData::getDataTypeName(data_type) + " unspecified.", "ParticipantComManager::getSignalFunctionForDataType");
        return nullptr;
    }
}

bool ParticipantComManager::handleLoginReply(const QString &reply_data)
{
    QJsonParseError json_error;

    // Process reply
    QJsonDocument login_info = QJsonDocument::fromJson(reply_data.toUtf8(), &json_error);
    if (json_error.error!= QJsonParseError::NoError)
        return false;

    QString participant_uuid = login_info["participant_uuid"].toString();
    m_currentParticipant.setFieldValue("participant_uuid", participant_uuid);
    m_currentParticipant.setFieldValue("participant_name", login_info["participant_name"].toString());

    // Connect websocket
    if (m_enableWebsocket){
        QString web_socket_url = login_info["websocket_url"].toString();
        m_webSocketMan->connectWebSocket(web_socket_url, participant_uuid);
    }else{
        onWebSocketLoginResult(true); // Simulate successful login with websocket
    }

    return true;
}

bool ParticipantComManager::handleDataReply(const QString& reply_path, const QString &reply_data, const QUrlQuery &reply_query)
{
    QJsonParseError json_error;

    // Process reply
    QString data_str = filterReplyString(reply_data);

    QJsonDocument data_list = QJsonDocument::fromJson(data_str.toUtf8(), &json_error);
    if (json_error.error!= QJsonParseError::NoError){
        LOG_ERROR("Received a JSON string for " + reply_path + " with " + reply_query.toString() + " with error: " + json_error.errorString(), "ParticipantComManager::handleDataReply");
        return false;
    }

    // Browse each items received
    QList<TeraData> items;
    TeraDataTypes items_type = TeraData::getDataTypeFromPath(reply_path);
    if (data_list.isArray()){
        for (const QJsonValue &data:data_list.array()){
            TeraData item_data(items_type, data);

            // Check if the currently connected user was updated and not requesting a list (limited information)
            if (items_type == TERADATA_PARTICIPANT){
                if (!reply_query.hasQueryItem(WEB_PARTICIPANT_QUERY_LIST))
                    updateCurrentParticipant(item_data);
            }

            items.append(item_data);
        }
    }else{
        TeraData item_data(items_type, data_list.object());
        if (items_type == TERADATA_PARTICIPANT){
            if (!reply_query.hasQueryItem(WEB_QUERY_LIST))
                updateCurrentParticipant(item_data);
        }
        items.append(item_data);
    }

    // Emit signal
    switch (items_type) {
    case TERADATA_NONE:
        LOG_ERROR("Unknown object - don't know what to do with it.", "ParticipantComManager::handleDataReply");
        break;
    case TERADATA_PARTICIPANT:
        emit participantsReceived(items, reply_query);
        break;
    default:
//        emit getSignalFunctionForDataType(items_type);
        break;

    }

    // Always emit generic signal
    emit dataReceived(items_type, items, reply_query);

    return true;
}

void ParticipantComManager::updateCurrentParticipant(const TeraData &part_data)
{
    if (m_currentParticipant.getUuid() == part_data.getUuid()){
        // Update fields that we received with the new values
        m_currentParticipant.updateFrom(part_data);
        emit currentParticipantUpdated();
    }
}

void ParticipantComManager::clearCurrentParticipant()
{
    m_currentParticipant = TeraData(TERADATA_PARTICIPANT);
    clearCredentials();
}

/////////////////////////////////////////////////////////////////////////////////////
void ParticipantComManager::onWebSocketLoginResult(bool logged_in)
{
    if (!logged_in){
        clearCurrentParticipant();
    }

    emit loginResult(logged_in);

    //doUpdateCurrentParticipant();
}
