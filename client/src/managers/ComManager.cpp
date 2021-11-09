#include "ComManager.h"
#include <sstream>
#include <QLocale>
#include "Utils.h"

ComManager::ComManager(QUrl serverUrl, bool connectWebsocket, QObject *parent) :
    QObject(parent),
    m_currentUser(TERADATA_USER),
    m_currentSessionType(nullptr)
{
    m_enableWebsocket = connectWebsocket;

    // Initialize communication objects
    m_netManager = new QNetworkAccessManager(this);
    m_netManager->setCookieJar(&m_cookieJar);

    // Setup signals and slots
    if (m_enableWebsocket){
        m_webSocketMan = new WebSocketManager();
        // Websocket manager
        connect(m_webSocketMan, &WebSocketManager::serverDisconnected, this, &ComManager::serverDisconnected); // Pass-thru signal
        connect(m_webSocketMan, &WebSocketManager::websocketError, this, &ComManager::socketError); // Pass-thru signal
        connect(m_webSocketMan, &WebSocketManager::loginResult, this, &ComManager::onWebSocketLoginResult);
    }

    // Network manager
    connect(m_netManager, &QNetworkAccessManager::authenticationRequired, this, &ComManager::onNetworkAuthenticationRequired);
    connect(m_netManager, &QNetworkAccessManager::encrypted, this, &ComManager::onNetworkEncrypted);
    connect(m_netManager, &QNetworkAccessManager::finished, this, &ComManager::onNetworkFinished);
    connect(m_netManager, &QNetworkAccessManager::sslErrors, this, &ComManager::onNetworkSslErrors);

    // Create correct server url
    m_serverUrl.setUrl("https://" + serverUrl.host() + ":" + QString::number(serverUrl.port()));

}

ComManager::~ComManager()
{
    if (m_enableWebsocket){
        m_webSocketMan->disconnectWebSocket();
        m_webSocketMan->deleteLater();
    }
    if (m_currentSessionType)
        delete m_currentSessionType;
}

void ComManager::connectToServer(QString username, QString password)
{
    LOG_DEBUG("ComManager::Connecting to " + m_serverUrl.toString(), "ComManager::connectToServer");

    setCredentials(username, password);

    m_loggingInProgress = true;     // Indicate that a login request was sent, but not processed

    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_WITH_WEBSOCKET, "1");
    doQuery(QString(WEB_LOGIN_PATH), args);

}

void ComManager::disconnectFromServer()
{
    doQuery(QString(WEB_LOGOUT_PATH));
    if (m_enableWebsocket){
        m_webSocketMan->disconnectWebSocket();
    }

    clearCurrentUser();
    m_settedCredentials = false;
}

bool ComManager::processNetworkReply(QNetworkReply *reply)
{
    QString reply_path = reply->url().path();
    QString reply_data = reply->readAll();
    QUrlQuery reply_query = QUrlQuery(reply->url().query());
    //qDebug() << reply_path << " ---> " << reply_data << ": " << reply->url().query();

    bool handled = false;

    if (reply->operation()==QNetworkAccessManager::GetOperation){
        if (reply_path == WEB_LOGIN_PATH){
            // Initialize cookies
            m_cookieJar.cookiesForUrl(reply->url());
            handled=handleLoginReply(reply_data);
        }

        if (m_loggingInProgress && !handled){
            // While not logged in, wait for user and user prefs
            handled = handleLoginSequence(reply_path, reply_data, reply_query);
        }

        if (reply_path == WEB_LOGOUT_PATH){
            emit serverDisconnected();
            handled = true;
        }

        if (reply_path == WEB_FORMS_PATH){
            handled = handleFormReply(reply_query, reply_data);
            if (handled) emit queryResultsOK(reply_path, reply_query);
        }


        /*if (reply_path == WEB_DEVICEDATAINFO_PATH && reply_query.hasQueryItem(WEB_QUERY_DOWNLOAD)){
            //qDebug() << "Download complete.";
            handled = true;

            // Remove reply from current download list, if present
            if (m_currentDownloads.contains(reply)){
                DownloadedFile* file = m_currentDownloads.take(reply);
                emit downloadCompleted(file);
                file->deleteLater();
            }
        }*/

        if (!handled){
            // General case
            handled=handleDataReply(reply_path, reply_data, reply_query);
            if (handled) emit queryResultsOK(reply_path, reply_query);
        }
    }

    if (reply->operation()==QNetworkAccessManager::PostOperation){
        if (reply_path == WEB_SESSIONMANAGER_PATH){
            handled = handleSessionManagerReply(reply_data, reply_query);
        }

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

void ComManager::doQuery(const QString &path, const QUrlQuery &query_args)
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
        LOG_DEBUG("GET: " + path + " with " + query_args.toString(), "ComManager::doQuery");
    else
        LOG_DEBUG("GET: " + path, "ComManager::doQuery");
}

void ComManager::doPost(const QString &path, const QString &post_data)
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
    LOG_DEBUG("POST: " + path + ", with " + post_data, "ComManager::doPost");
#else
    // Strip data from logging in release, since this might contains passwords!
    LOG_DEBUG("POST: " + path, "ComManager::doPost");
#endif
}

void ComManager::doDelete(const QString &path, const int &id)
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

    LOG_DEBUG("DELETE: " + path + ", with id=" + QString::number(id), "ComManager::doDelete");
}

void ComManager::doUpdateCurrentUser()
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_SELF, "");
    args.addQueryItem(WEB_QUERY_WITH_USERGROUPS, "1");
    doQuery(QString(WEB_USERINFO_PATH), args);

    // Update preferences
    args.clear();
    args.addQueryItem(WEB_QUERY_APPTAG, APPLICATION_TAG);
    doQuery(WEB_USERPREFSINFO_PATH, args);
}

void ComManager::doDownload(const QString &save_path, const QString &path, const QUrlQuery &query_args)
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

    QNetworkReply* reply = m_netManager->get(request);
    if (reply){
        DownloadedFile* file_to_download = new DownloadedFile(reply, save_path);
        m_currentDownloads[reply] = file_to_download;

        connect(file_to_download, &DownloadedFile::downloadProgress, this, &ComManager::downloadProgress);
    }

    emit waitingForReply(true);

    LOG_DEBUG("DOWNLOADING: " + path + ", with " + query_args.toString() + ", to " + save_path, "ComManager::doQuery");
}

void ComManager::startSession(const TeraData &session_type, const int &id_session, const QStringList &participants_list, const QStringList &users_list, const QStringList &devices_list, const QJsonDocument &session_config)
{
    if (session_type.getDataType() != TERADATA_SESSIONTYPE){
        LOG_ERROR("Received an invalid session_type object", "ComManager::startSession");
        return;
    }

    if (m_currentSessionType){
        LOG_WARNING("Tried to start a session, but already one started.", "ComManager::startSession");
        m_currentSessionType->deleteLater();
        m_currentSessionType = nullptr;
    }

    TeraSessionCategory::SessionTypeCategories session_type_category = static_cast<TeraSessionCategory::SessionTypeCategories>(session_type.getFieldValue("session_type_category").toInt());

    // Do the correct request to server, depending on the type of the session we want to start
    switch(session_type_category){
    case TeraSessionCategory::SESSION_TYPE_UNKNOWN:
        break;
    case TeraSessionCategory::SESSION_TYPE_DATACOLLECT:
        break;
    case TeraSessionCategory::SESSION_TYPE_FILETRANSFER:
        break;
    case TeraSessionCategory::SESSION_TYPE_PROTOCOL:
        break;
    case TeraSessionCategory::SESSION_TYPE_SERVICE:
       {
        QJsonDocument document;
        QJsonObject base_obj;

        QJsonObject item_obj;
        item_obj.insert("id_service", session_type.getFieldValue("id_service").toInt());
        item_obj.insert("id_session", id_session);
        item_obj.insert("id_session_type", session_type.getId());
        item_obj.insert("action", "start");
        item_obj.insert("parameters", session_type.getFieldValue("session_type_config").toString());

        // Devices
        if (!devices_list.isEmpty()){
            QJsonArray devices;
            for(const QString &device_uuid:devices_list){
                devices.append(QJsonValue(device_uuid));
            }
            item_obj.insert("session_devices", devices);
        }

        // Participants
        if (!participants_list.isEmpty()){
            QJsonArray participants;
            for(const QString &part_uuid:participants_list){
                participants.append(QJsonValue(part_uuid));
            }
            item_obj.insert("session_participants", participants);
        }

        // Always add the current user to the list
        QStringList current_users_list = users_list;
        if (!current_users_list.contains(m_currentUser.getUuid()))
            current_users_list.append(m_currentUser.getUuid());

        if (!current_users_list.isEmpty()){
            QJsonArray users;
            for(const QString &user_uuid:current_users_list){
                users.append(QJsonValue(user_uuid));
            }
            item_obj.insert("session_users", users);
        }

        // Update query
        base_obj.insert("session_manage", item_obj);
        document.setObject(base_obj);
        doPost(WEB_SESSIONMANAGER_PATH, document.toJson());
        }
        break;
    default:
        // TODO: Manage other service category types
        break;
    }

    m_currentSessionType = new TeraData(session_type);
    m_currentSessionConfig = session_config;
    emit sessionStartRequested(session_type);
}

void ComManager::joinSession(const TeraData &session_type, const int &id_session)
{
    if (session_type.getDataType() != TERADATA_SESSIONTYPE){
        LOG_ERROR("Received an invalid session_type object", "ComManager::joinSession");
        return;
    }

    if (m_currentSessionType){
        LOG_WARNING("Tried to join a session, but already one in progress.", "ComManager::joinSession");
        m_currentSessionType->deleteLater();
        m_currentSessionType = nullptr;
    }

    m_currentSessionType = new TeraData(session_type);
    emit sessionStarted(session_type, id_session);

}

void ComManager::stopSession(const TeraData &session, const int &id_service)
{
    if (session.getDataType() != TERADATA_SESSION)
        LOG_ERROR("Received an invalid session object", "ComManager::stopSession");

    if (id_service > 0){
        // Do a request to stop the current session on the service
        QJsonDocument document;
        QJsonObject base_obj;

        QJsonObject item_obj;
        item_obj.insert("id_service", id_service);
        item_obj.insert("id_session", session.getId());
        item_obj.insert("action", "stop");

        // Update query
        base_obj.insert("session_manage", item_obj);
        document.setObject(base_obj);
        doPost(WEB_SESSIONMANAGER_PATH, document.toJson());
    }else{
        // Changes the session status only.
        delete m_currentSessionType;
        m_currentSessionType = nullptr;
        m_currentSessionConfig = QJsonDocument();
    }
}

void ComManager::leaveSession(const int &id_session, bool signal_server)
{
    if (signal_server){
        QJsonDocument document;
        QJsonObject base_obj;

        QJsonObject item_obj;
        item_obj.insert("id_session", id_session);
        item_obj.insert("action", "remove");

        // Add ourself to the list
        QJsonArray users;
        users.append(QJsonValue(m_currentUser.getUuid()));
        item_obj.insert("session_users", users);

        // Update query
        base_obj.insert("session_manage", item_obj);
        document.setObject(base_obj);
        doPost(WEB_SESSIONMANAGER_PATH, document.toJson());
    }
    emit sessionStopped(id_session);
}

void ComManager::sendJoinSessionReply(const QString &session_uuid, const JoinSessionReplyEvent::ReplyType reply_type, const QString &join_msg)
{
    QJsonDocument document;
    QJsonObject base_obj;

    QJsonObject item_obj;
    item_obj.insert("session_uuid", session_uuid);
    item_obj.insert("action", "invite_reply");
    QJsonObject parameters;
    parameters.insert("reply_code", reply_type);
    if (!join_msg.isEmpty())
        parameters.insert("reply_msg", join_msg);
    item_obj.insert("parameters", parameters);

    // Update query
    base_obj.insert("session_manage", item_obj);
    document.setObject(base_obj);

    doPost(WEB_SESSIONMANAGER_PATH, document.toJson());
}

TeraData &ComManager::getCurrentUser()
{
    return m_currentUser;
}

TeraPreferences &ComManager::getCurrentPreferences()
{
   return m_currentPreferences;
}

QString ComManager::getCurrentUserSiteRole(const int &site_id)
{
    QString rval = "";

    if (m_currentUser.hasFieldName("sites")){
        QVariantList sites_list = m_currentUser.getFieldValue("sites").toList();
        for (int i=0; i<sites_list.count(); i++){
            QVariantMap site_info = sites_list.at(i).toMap();
            if (site_info.contains("id_site")){
                if (site_info["id_site"].toInt() == site_id){
                    if (site_info.contains("site_role")){
                        rval = site_info["site_role"].toString();
                        break;
                    }
                }
            }
        }
    }

    return rval;
}

QString ComManager::getCurrentUserProjectRole(const int &project_id)
{
    QString rval = "";

    if (m_currentUser.hasFieldName("projects")){
        QVariantList proj_list = m_currentUser.getFieldValue("projects").toList();
        for (int i=0; i<proj_list.count(); i++){
            QVariantMap proj_info = proj_list.at(i).toMap();
            if (proj_info.contains("id_project")){
                if (proj_info["id_project"].toInt() == project_id){
                    if (proj_info.contains("project_role")){
                        rval = proj_info["project_role"].toString();
                        break;
                    }
                }
            }
        }
    }

    return rval;
}

bool ComManager::isCurrentUserProjectAdmin(const int &project_id)
{
    return getCurrentUserProjectRole(project_id) == "admin";
}

bool ComManager::isCurrentUserSiteAdmin(const int &site_id)
{
    return getCurrentUserSiteRole(site_id) == "admin";
}

bool ComManager::isCurrentUserSuperAdmin()
{
    bool rval = false;
    if (m_currentUser.hasFieldName("user_superadmin")){
        rval = m_currentUser.getFieldValue("user_superadmin").toBool();
    }
    return rval;
}

bool ComManager::hasPendingDownloads()
{
    return !m_currentDownloads.isEmpty();
}

void ComManager::setCredentials(const QString &username, const QString &password)
{
    m_username = username;
    m_password = password;

    m_settedCredentials = false;  // Credentials were changed, must update on next request
}

QUrl ComManager::getServerUrl() const
{
    return m_serverUrl;
}

WebSocketManager *ComManager::getWebSocketManager()
{
    return m_webSocketMan;
}

ComManager::signal_ptr ComManager::getSignalFunctionForDataType(const TeraDataTypes &data_type)
{
    switch(data_type){
    case TERADATA_NONE:
        LOG_ERROR("Unknown object - no signal associated.", "ComManager::getSignalFunctionForDataType");
        return nullptr;
    case TERADATA_USER:
        return &ComManager::usersReceived;
    case TERADATA_USERGROUP:
        return &ComManager::userGroupsReceived;
    case TERADATA_SITE:
        return &ComManager::sitesReceived;
    case TERADATA_SESSIONTYPE:
        return &ComManager::sessionTypesReceived;
    case TERADATA_TESTDEF:
        return &ComManager::testDefsReceived;
    case TERADATA_PROJECT:
        return &ComManager::projectsReceived;
    case TERADATA_DEVICE:
        return &ComManager::devicesReceived;
    case TERADATA_GROUP:
        return &ComManager::groupsReceived;
    case TERADATA_PARTICIPANT:
        return &ComManager::participantsReceived;
    case TERADATA_SITEACCESS:
        return &ComManager::siteAccessReceived;
    case TERADATA_PROJECTACCESS:
        return &ComManager::projectAccessReceived;
    case TERADATA_SESSION:
        return &ComManager::sessionsReceived;
    case TERADATA_DEVICESUBTYPE:
        return &ComManager::deviceSubtypesReceived;
    case TERADATA_DEVICETYPE:
        return &ComManager::deviceTypesReceived;
    case TERADATA_SERVICE:
        return &ComManager::servicesReceived;
    case TERADATA_SERVICE_ACCESS:
        return &ComManager::servicesAccessReceived;
    case TERADATA_SERVICE_PROJECT:
        return &ComManager::servicesProjectsReceived;
    case TERADATA_SESSIONTYPEPROJECT:
        return &ComManager::sessionTypesProjectsReceived;
    case TERADATA_SERVICE_CONFIG:
        return &ComManager::servicesConfigReceived;
    default:
        LOG_WARNING("Signal for object " + TeraData::getDataTypeName(data_type) + " unspecified.", "ComManager::getSignalFunctionForDataType");
        return nullptr;
    }
}

QJsonDocument ComManager::getCurrentSessionConfig()
{
    return m_currentSessionConfig;
}

TeraData *ComManager::getCurrentSessionType()
{
    return m_currentSessionType;
}

bool ComManager::handleLoginReply(const QString &reply_data)
{
    QJsonParseError json_error;

    // Process reply
    QJsonDocument login_info = QJsonDocument::fromJson(reply_data.toUtf8(), &json_error);
    if (json_error.error!= QJsonParseError::NoError)
        return false;

    // Connect websocket
    QString user_uuid = login_info["user_uuid"].toString();
    if (m_enableWebsocket){
        QString web_socket_url = login_info["websocket_url"].toString();
        if (!web_socket_url.isEmpty())
            m_webSocketMan->connectWebSocket(web_socket_url, user_uuid);
        else{
            clearCurrentUser();
            emit loginResult(false, tr("L'utilisateur est déjà connecté."));
            return true;
        }
    }else{
        onWebSocketLoginResult(true); // Simulate successful login with websocket
    }

    // Query connected user information

    m_currentUser.setFieldValue("user_uuid", user_uuid);
    //doUpdateCurrentUser();

    // Query versions informations
    doQuery(WEB_VERSIONSINFO_PATH);

    return true;
}

bool ComManager::handleLoginSequence(const QString &reply_path, const QString &reply_data, const QUrlQuery &reply_query)
{
    QJsonParseError json_error;

    // Process reply
    QString data_str = filterReplyString(reply_data);

    QJsonDocument data_list = QJsonDocument::fromJson(data_str.toUtf8(), &json_error);
    if (json_error.error!= QJsonParseError::NoError){
        LOG_ERROR("Received a JSON string for " + reply_path + " with " + reply_query.toString() + " with error: " + json_error.errorString(), "ComManager::handleDataReply");
        return false;
    }

    // Versions information reply
    if (reply_path == WEB_VERSIONSINFO_PATH){
        return handleVersionsReply(data_list);
    }

    //QList<TeraData> items;
    TeraDataTypes items_type = TeraData::getDataTypeFromPath(reply_path);

    if (items_type != TERADATA_USER && items_type != TERADATA_USERPREFERENCE)
        return false;

    QJsonValue data = data_list.array().first();

    if (items_type == TERADATA_USER){
        TeraData item_data(items_type, data);
        updateCurrentUser(item_data);
    }

    if (items_type == TERADATA_USERPREFERENCE){
        if (data.isUndefined()){
            // No preference for that user, will use default.
            m_currentPreferences.setSet(true);
            emit preferencesUpdated();
        }else{
            TeraData item_data(items_type, data);
            updateCurrentPrefs(item_data);
        }
    }

    // Check if we received current user and preference, when login, before emitting signal
    if (m_loggingInProgress){
        //qDebug() << "Still logging in...";
        if (m_currentPreferences.isSet()){
            //qDebug() << "All set!";
            emit loginResult(true, "");
            m_loggingInProgress = false;
        }
    }

    return true;
}

bool ComManager::handleDataReply(const QString& reply_path, const QString &reply_data, const QUrlQuery &reply_query)
{
    QJsonParseError json_error;

    // Process reply
    QString data_str = filterReplyString(reply_data);

    QJsonDocument data_list = QJsonDocument::fromJson(data_str.toUtf8(), &json_error);
    if (json_error.error!= QJsonParseError::NoError){
        LOG_ERROR("Received a JSON string for " + reply_path + " with " + reply_query.toString() + " with error: " + json_error.errorString(), "ComManager::handleDataReply");
        return false;
    }

    // Versions information reply
    if (reply_path == WEB_VERSIONSINFO_PATH){
        return handleVersionsReply(data_list);
    }

    // Browse each items received
    QList<TeraData> items;
    TeraDataTypes items_type = TeraData::getDataTypeFromPath(reply_path);
    if (data_list.isArray()){
        for (const QJsonValue &data:data_list.array()){
            TeraData item_data(items_type, data);

            // Check if the currently connected user was updated and not requesting a list (limited information)
            if (items_type == TERADATA_USER){
                if (!reply_query.hasQueryItem(WEB_QUERY_LIST))
                    updateCurrentUser(item_data);
            }

            // Check if we received user preferences that need to be updated
            if (items_type == TERADATA_USERPREFERENCE){
                updateCurrentPrefs(item_data);
            }

            items.append(item_data);
        }
    }else{
        TeraData item_data(items_type, data_list.object());
        if (items_type == TERADATA_USER){
            if (!reply_query.hasQueryItem(WEB_QUERY_LIST))
                updateCurrentUser(item_data);
        }
        if (items_type == TERADATA_USERPREFERENCE){
            updateCurrentPrefs(item_data);
        }
        items.append(item_data);
    }

    // Emit signal
    switch (items_type) {
    case TERADATA_NONE:
        LOG_ERROR("Unknown object - don't know what to do with it.", "ComManager::handleDataReply");
        break;
    case TERADATA_USER:
        emit usersReceived(items, reply_query);
        break;
    case TERADATA_USERGROUP:
        emit userGroupsReceived(items, reply_query);
        break;
    case TERADATA_USERUSERGROUP:
        emit userUserGroupsReceived(items, reply_query);
        break;
    case TERADATA_USERPREFERENCE:
        emit userPreferencesReceived(items, reply_query);
        break;
    case TERADATA_SITE:
        emit sitesReceived(items, reply_query);
        break;
    case TERADATA_SESSIONTYPE:
        emit sessionTypesReceived(items, reply_query);
        break;
    case TERADATA_TESTDEF:
        emit testDefsReceived(items, reply_query);
        break;
    case TERADATA_PROJECT:
        emit projectsReceived(items, reply_query);
        break;
    case TERADATA_DEVICE:
        emit devicesReceived(items, reply_query);
        break;
    case TERADATA_PARTICIPANT:
        emit participantsReceived(items, reply_query);
        break;
    case TERADATA_GROUP:
        emit groupsReceived(items, reply_query);
        break;
    case TERADATA_SITEACCESS:
        emit siteAccessReceived(items, reply_query);
        break;
    case TERADATA_PROJECTACCESS:
        emit projectAccessReceived(items, reply_query);
        break;
    case TERADATA_SESSION:
        emit sessionsReceived(items, reply_query);
        break;
    case TERADATA_DEVICESITE:
        emit deviceSitesReceived(items, reply_query);
        break;
    case TERADATA_DEVICEPROJECT:
        emit deviceProjectsReceived(items, reply_query);
        break;
    case TERADATA_DEVICEPARTICIPANT:
        emit deviceParticipantsReceived(items, reply_query);
        break;
    case TERADATA_DEVICESUBTYPE:
        emit deviceSubtypesReceived(items, reply_query);
        break;
    case TERADATA_DEVICETYPE:
        emit deviceTypesReceived(items, reply_query);
        break;
    case TERADATA_SESSIONTYPEPROJECT:
        emit sessionTypesProjectsReceived(items, reply_query);
        break;
    case TERADATA_SESSIONEVENT:
        emit sessionEventsReceived(items, reply_query);
        break;
    case TERADATA_SERVICE:
        emit servicesReceived(items, reply_query);
        break;
    case TERADATA_SERVICE_PROJECT:
        emit servicesProjectsReceived(items, reply_query);
        break;
    case TERADATA_SERVICE_ACCESS:
        emit servicesAccessReceived(items, reply_query);
        break;
    case TERADATA_SERVICE_CONFIG:
        emit servicesConfigReceived(items, reply_query);
    case TERADATA_STATS:
        if (items.count() > 0)
            emit statsReceived(items.first(), reply_query);
        break;
    case TERADATA_ONLINE_DEVICE:
        emit onlineDevicesReceived(items, reply_query);
        break;
    case TERADATA_ONLINE_PARTICIPANT:
        emit onlineParticipantsReceived(items, reply_query);
        break;
    case TERADATA_ONLINE_USER:
        emit onlineUsersReceived(items, reply_query);
        break;
/*    default:
        emit getSignalFunctionForDataType(items_type);*/

    }

    // Always emit generic signal
    emit dataReceived(items_type, items, reply_query);

    return true;
}

bool ComManager::handleSessionManagerReply(const QString &reply_data, const QUrlQuery &reply_query)
{
    if (!m_currentSessionType){
        LOG_WARNING("Received a Session Manager reply, but no session type was defined", "ComManager::handleSessionManagerReply");
        return false;
    }
    QJsonParseError json_error;

    // Process reply
    QString data_str = filterReplyString(reply_data);

    // Convert reply to json structure
    QJsonDocument data_list = QJsonDocument::fromJson(data_str.toUtf8(), &json_error);
    if (json_error.error!= QJsonParseError::NoError){
        LOG_ERROR("Received a JSON string from SessionManager with " + reply_query.toString() + " with error: " + json_error.errorString(), "ComManager::handleSessionManagerReply");
        return false;
    }

    // Check the status in the reply
    QJsonObject reply_json = data_list.object();
    if (reply_json.contains("status")){
        QString status = reply_json["status"].toString();
        if (status == "started"){
            if (reply_json.contains("session")){
                QJsonObject session_obj = reply_json["session"].toObject();
                if (session_obj.contains("id_session")){
                    emit sessionStarted(*m_currentSessionType, session_obj["id_session"].toInt());
                }else{
                    LOG_WARNING("Received a start session event, but no id_session into it", "ComManager::handleSessionManagerReply");
                }
            }else{
                LOG_ERROR("Received 'started' status but without any session", "ComManager::handleSessionManagerReply");
            }
            return true;
        }

        if (status == "stopped"){
            if (reply_json.contains("session")){
                QJsonObject session_obj = reply_json["session"].toObject();
                //qDebug() << reply_json;
                if (session_obj.contains("id_session")){
                    emit sessionStopped(session_obj["id_session"].toInt());
                }else{
                    LOG_WARNING("Received a stop session event, but no id_session into it", "ComManager::handleSessionManagerReply");
                }
            }else{
                LOG_ERROR("Received 'stopped' status but without any session", "ComManager::handleSessionManagerReply");
            }
            // Delete current session type infos
            m_currentSessionType->deleteLater();
            m_currentSessionType = nullptr;
            return true;
        }

        if (status == "error"){
            QString err_msg = tr("Erreur inconnue");
            if (reply_json.contains("error_text")){
                err_msg = reply_json["error_text"].toString();
            }
            emit sessionError(err_msg);
            return true;
        }
    }else{
        LOG_ERROR("Received a Session Manager reply, but no status in it.", "ComManager::handleSessionManagerReply");
        return false;
    }

    return true; // Consider all other unmanaged messages to be handled for now!

}

bool ComManager::handleFormReply(const QUrlQuery &reply_query, const QString &reply_data)
{
    emit formReceived(reply_query.toString(), reply_data);
    return true;
}

bool ComManager::handleVersionsReply(const QJsonDocument &version_data)
{
    QJsonObject clients = version_data["clients"].toObject();
    if (!clients.isEmpty()){
        QJsonObject openteraplus = clients["OpenTeraPlus"].toObject();
        if (!openteraplus.isEmpty()){
            QString current_client_version = openteraplus["client_version"].toString();
            if (current_client_version != OPENTERAPLUS_VERSION){
                // New version available... maybe!
                QString current_version_url;
#ifdef Q_OS_WIN
                current_version_url = openteraplus["client_windows_download_url"].toString();
#endif
#ifdef Q_OS_LINUX
                current_version_url = openteraplus["client_linux_download_url"].toString();
#endif
#ifdef Q_OS_MACOS
                current_version_url = openteraplus["client_mac_download_url"].toString();
#endif
                emit newVersionAvailable(current_client_version, current_version_url);
            }
        }
    }
    return true;
}

void ComManager::updateCurrentUser(const TeraData &user_data)
{
    //qDebug() << "ComManager::updateCurrentUser";
    if (m_currentUser.getUuid() == user_data.getUuid()){
        // Update fields that we received with the new values
        //qDebug() << "Updating user...";
        m_currentUser.updateFrom(user_data);

        // Update credentials
        emit currentUserUpdated();
    }
}

void ComManager::updateCurrentPrefs(const TeraData &user_prefs)
{
    //qDebug() << "ComManager::updateCurrentPrefs";

    if ((!m_currentUser.hasFieldName("id_user") || m_currentUser.getId() == user_prefs.getFieldValue("id_user").toInt())
         && user_prefs.getFieldValue("user_preference_app_tag").toString() == APPLICATION_TAG){
        // Update preferences
        //qDebug() << "Updating preferences...";
        m_currentPreferences.load(user_prefs);
        emit preferencesUpdated();
    }
}

void ComManager::clearCurrentUser()
{
    m_currentPreferences.clear();
    m_currentUser = TeraData(TERADATA_USER);
}

QString ComManager::filterReplyString(const QString &data_str)
{
    QString filtered_str = data_str;
    if (data_str.isEmpty() || data_str == "\n" || data_str == "null\n")
        filtered_str = "[]"; // Replace empty string with empty list!

    return filtered_str;
}



/////////////////////////////////////////////////////////////////////////////////////
void ComManager::onNetworkAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
    Q_UNUSED(reply)
    // If we are logging in, credentials were already sent, and if we get here, it's because they were
    // rejected
    if (!m_settedCredentials && !m_loggingInProgress){
        LOG_DEBUG("Sending authentication request...", "ComManager::onNetworkAuthenticationRequired");
        authenticator->setUser(m_username);
        authenticator->setPassword(m_password);
        m_settedCredentials = true; // We setted the credentials in authentificator
    }else{
        LOG_DEBUG("Authentication error", "ComManager::onNetworkAuthenticationRequired");
        emit loginResult(false, tr("Utilisateur ou mot de passe invalide."));
    }
}

void ComManager::onNetworkEncrypted(QNetworkReply *reply)
{
    Q_UNUSED(reply)
    //qDebug() << "ComManager::onNetworkEncrypted";
}

void ComManager::onNetworkFinished(QNetworkReply *reply)
{
    emit waitingForReply(false);

    if (reply->error() == QNetworkReply::NoError)
    {
        if (!processNetworkReply(reply)){
            LOG_WARNING("Unhandled reply - " + reply->url().path(), "ComManager::onNetworkFinished");
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
        //qDebug() << "ComManager::onNetworkFinished - Reply error: " << reply->error() << ", Reply message: " << reply_msg;
        LOG_ERROR("ComManager::onNetworkFinished - Reply error: " + reply->errorString() + ", Reply message: " + reply_msg, "ComManager::onNetworkFinished");
        /*if (reply_msg.isEmpty())
            reply_msg = reply->errorString();*/
        emit networkError(reply->error(), reply_msg, reply->operation(), status_code);
    }

    reply->deleteLater();
}

void ComManager::onNetworkSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    Q_UNUSED(reply)
    Q_UNUSED(errors)
    LOG_WARNING("Ignoring SSL errors, this is unsafe", "ComManager::onNetworkSslErrors");
    reply->ignoreSslErrors();
    for(const QSslError &error : errors){
        LOG_WARNING("Ignored: " + error.errorString(), "ComManager::onNetworkSslErrors");
    }
}

void ComManager::onWebSocketLoginResult(bool logged_in)
{
    if (!logged_in){
        clearCurrentUser();
        emit loginResult(false, tr("La communication avec le serveur n'a pu être établie."));
        return;
    }

    doUpdateCurrentUser();
}


void ComManager::setRequestLanguage(QNetworkRequest &request) {
    //Locale will be initialized with default locale
    QString localeString = QLocale().bcp47Name();
    //qDebug() << "localeString : " << localeString;
    request.setRawHeader(QByteArray("Accept-Language"), localeString.toUtf8());
}

void ComManager::setRequestCredentials(QNetworkRequest &request)
{
    //Needed?
    request.setAttribute(QNetworkRequest::AuthenticationReuseAttribute, false);

    // Pack in credentials
    QString concatenatedCredentials = m_username + ":" + m_password;
    QByteArray data = concatenatedCredentials.toLocal8Bit().toBase64();
    QString headerData = "Basic " + data;
    request.setRawHeader( "Authorization", headerData.toLocal8Bit() );

}

void ComManager::setRequestVersions(QNetworkRequest &request)
{
    request.setRawHeader("X-Client-Name", QByteArray(OPENTERAPLUS_CLIENT_NAME));
    request.setRawHeader("X-Client-Version", QByteArray(OPENTERAPLUS_VERSION));
}
