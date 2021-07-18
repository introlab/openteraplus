#include "ParticipantComManager.h"
#include <sstream>
#include <QLocale>
#include "Utils.h"

ParticipantComManager::ParticipantComManager(QUrl serverUrl, bool connectWebsocket, QObject *parent) :
    QObject(parent),
    m_currentParticipant(TERADATA_PARTICIPANT)
{
    m_enableWebsocket = connectWebsocket;

    // Initialize communication objects
    m_netManager = new QNetworkAccessManager(this);
    m_netManager->setCookieJar(&m_cookieJar);

    // Setup signals and slots
    if (m_enableWebsocket){
        m_webSocketMan = new WebSocketManager();
        // Websocket manager
        connect(m_webSocketMan, &WebSocketManager::serverDisconnected, this, &ParticipantComManager::serverDisconnected); // Pass-thru signal
        connect(m_webSocketMan, &WebSocketManager::websocketError, this, &ParticipantComManager::socketError); // Pass-thru signal
        connect(m_webSocketMan, &WebSocketManager::loginResult, this, &ParticipantComManager::onWebSocketLoginResult);
    }

    // Network manager
#ifndef WEBASSEMBLY_COMPILATION
    connect(m_netManager, &QNetworkAccessManager::encrypted, this, &ParticipantComManager::onNetworkEncrypted);
    connect(m_netManager, &QNetworkAccessManager::sslErrors, this, &ParticipantComManager::onNetworkSslErrors);
#endif
    connect(m_netManager, &QNetworkAccessManager::finished, this, &ParticipantComManager::onNetworkFinished);
    // Create correct server url
    m_serverUrl.setUrl("https://" + serverUrl.host() + ":" + QString::number(serverUrl.port()));

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

    doQuery(QString(WEB_PARTICIPANT_LOGIN_PATH));

}

void ParticipantComManager::disconnectFromServer()
{
    doQuery(QString(WEB_PARTICIPANT_LOGOUT_PATH));
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

void ParticipantComManager::doQuery(const QString &path, const QUrlQuery &query_args)
{
    QUrl query = m_serverUrl;

    query.setPath(path);
    if (!query_args.isEmpty()){
        query.setQuery(query_args);
    }
    QNetworkRequest request(query);

    setRequestCredentials(request);
    setRequestLanguage(request);
    setRequestVersions(request);

    m_netManager->get(request);
    emit waitingForReply(true);
    emit querying(path);

    if (!query_args.isEmpty())
        LOG_DEBUG("GET: " + path + " with " + query_args.toString(), "ParticipantComManager::doQuery");
    else
        LOG_DEBUG("GET: " + path, "ParticipantComManager::doQuery");
}

void ParticipantComManager::doPost(const QString &path, const QString &post_data)
{
    QUrl query = m_serverUrl;

    query.setPath(path);
    QNetworkRequest request(query);
    setRequestCredentials(request);
    setRequestLanguage(request);
    setRequestVersions(request);

    request.setRawHeader("Content-Type", "application/json");

    m_netManager->post(request, post_data.toUtf8());
    emit waitingForReply(true);
    emit posting(path, post_data);

#ifndef QT_NO_DEBUG
    LOG_DEBUG("POST: " + path + ", with " + post_data, "ParticipantComManager::doPost");
#else
    // Strip data from logging in release, since this might contains passwords!
    LOG_DEBUG("POST: " + path, "ComManager::doPost");
#endif
}

void ParticipantComManager::doDelete(const QString &path, const int &id)
{
    QUrl query = m_serverUrl;

    query.setPath(path);
    query.setQuery("id=" + QString::number(id));
    QNetworkRequest request(query);
    setRequestCredentials(request);
    setRequestLanguage(request);
    setRequestVersions(request);

    m_netManager->deleteResource(request);
    emit waitingForReply(true);
    emit deleting(path);

    LOG_DEBUG("DELETE: " + path + ", with id=" + QString::number(id), "ParticipantComManager::doDelete");
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

void ParticipantComManager::setCredentials(const QString &token)
{
    m_token = token;
}

QUrl ParticipantComManager::getServerUrl() const
{
    return m_serverUrl;
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
#if 0
    case TERADATA_PARTICIPANT:
        return &ParticipantComManager::participantsReceived;
#endif
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
#if 0
    case TERADATA_PARTICIPANT:
        emit participantsReceived(items, reply_query);
        break;
#endif
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
    m_token = "";
}

QString ParticipantComManager::filterReplyString(const QString &data_str)
{
    QString filtered_str = data_str;
    if (data_str.isEmpty() || data_str == "\n" || data_str == "null\n")
        filtered_str = "[]"; // Replace empty string with empty list!

    return filtered_str;
}



#ifndef WEBASSEMBLY_COMPILATION
void ParticipantComManager::onNetworkEncrypted(QNetworkReply *reply)
{
    Q_UNUSED(reply)
    //qDebug() << "ComManager::onNetworkEncrypted";
}
#endif

void ParticipantComManager::onNetworkFinished(QNetworkReply *reply)
{
    emit waitingForReply(false);

    if (reply->error() == QNetworkReply::NoError)
    {
        if (!processNetworkReply(reply)){
            LOG_WARNING("Unhandled reply - " + reply->url().path(), "ParticipantComManager::onNetworkFinished");
        }
    }
    else {
        QByteArray reply_data = reply->readAll();

        QString reply_msg = QString::fromUtf8(reply_data).replace("\"", "");

        // Convert in-string unicode characters
        Utils::inStringUnicodeConverter(&reply_msg);

        if (reply_msg.isEmpty() || reply_msg.startsWith("\"\"") || reply_msg == "\n"){
            //reply_msg = tr("Erreur non-détaillée.");
            reply_msg = reply->errorString();
        }

        int status_code = -1;
        if (reply->attribute( QNetworkRequest::HttpStatusCodeAttribute).isValid())
            status_code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute).toInt();
        LOG_ERROR("ParticipantComManager::onNetworkFinished - Reply error: " + reply->errorString() + ", Reply message: " + reply_msg, "ParticipantComManager::onNetworkFinished");
        emit networkError(reply->error(), reply_msg, reply->operation(), status_code);
    }

    reply->deleteLater();
}

#ifndef WEBASSEMBLY_COMPILATION
void ParticipantComManager::onNetworkSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    Q_UNUSED(reply)
    Q_UNUSED(errors)
    LOG_WARNING("Ignoring SSL errors, this is unsafe", "ParticipantComManager::onNetworkSslErrors");
    reply->ignoreSslErrors();
    for(const QSslError &error : errors){
        LOG_WARNING("Ignored: " + error.errorString(), "ParticipantComManager::onNetworkSslErrors");
    }
}
#endif

void ParticipantComManager::onWebSocketLoginResult(bool logged_in)
{
    if (!logged_in){
        clearCurrentParticipant();
    }

    emit loginResult(logged_in);

    //doUpdateCurrentParticipant();
}


void ParticipantComManager::setRequestLanguage(QNetworkRequest &request) {
    //Locale will be initialized with default locale
    QString localeString = QLocale().bcp47Name();
    //qDebug() << "localeString : " << localeString;
    request.setRawHeader(QByteArray("Accept-Language"), localeString.toUtf8());
}

void ParticipantComManager::setRequestCredentials(QNetworkRequest &request)
{
    //Needed?
    request.setAttribute(QNetworkRequest::AuthenticationReuseAttribute, false);


    // Pack in credentials
    QString headerData = "OpenTera " + m_token;
    request.setRawHeader( "Authorization", headerData.toLocal8Bit() );

}

void ParticipantComManager::setRequestVersions(QNetworkRequest &request)
{
    request.setRawHeader("X-Client-Name", QByteArray(OPENTERAPLUS_CLIENT_NAME));
    request.setRawHeader("X-Client-Version", QByteArray(OPENTERAPLUS_VERSION));
}
