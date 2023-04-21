#include "ComManager.h"
#include <sstream>
#include <QLocale>

ComManager::ComManager(QUrl serverUrl, bool connectWebsocket, QObject *parent) :
    BaseComManager(serverUrl, parent),
    m_currentUser(TERADATA_USER),
    m_currentSessionType(nullptr)
{
    m_enableWebsocket = connectWebsocket;

    // Setup signals and slots
    if (m_enableWebsocket){
        m_webSocketMan = new WebSocketManager();
        // Websocket manager
        connect(m_webSocketMan, &WebSocketManager::serverDisconnected, this, &ComManager::serverDisconnected); // Pass-thru signal
        connect(m_webSocketMan, &WebSocketManager::websocketError, this, &ComManager::socketError); // Pass-thru signal
        connect(m_webSocketMan, &WebSocketManager::loginResult, this, &ComManager::onWebSocketLoginResult);
    }

    // Network manager
    connect(this, &ComManager::networkAuthFailed, this, &ComManager::onNetworkAuthenticationFailed);

    // Initialize token refresher timer @ each 15 minutes
    m_tokenRefreshTimer.setInterval(1000*60*15);
    m_tokenRefreshTimer.setSingleShot(false);
    connect(&m_tokenRefreshTimer, &QTimer::timeout, this, &ComManager::refreshUserToken);

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
    doGet(QString(WEB_LOGIN_PATH), args, false);

}

void ComManager::disconnectFromServer()
{
    doGet(QString(WEB_LOGOUT_PATH));
    if (m_enableWebsocket){
        m_webSocketMan->disconnectWebSocket();
    }

    clearCurrentUser();

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

        if (reply_path == WEB_DISCONNECT_PATH)
            handled = true;

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

void ComManager::doUpdateCurrentUser()
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_SELF, "");
    args.addQueryItem(WEB_QUERY_WITH_USERGROUPS, "1");
    doGet(QString(WEB_USERINFO_PATH), args);

    // Update preferences
    args.clear();
    args.addQueryItem(WEB_QUERY_APPTAG, APPLICATION_TAG);
    doGet(WEB_USERPREFSINFO_PATH, args);
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

void ComManager::sendJoinSessionReply(const QString &session_uuid, const opentera::protobuf::JoinSessionReplyEvent::ReplyType reply_type, const QString &join_msg)
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
    case TERADATA_TESTTYPE:
        return &ComManager::testTypesReceived;
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
    case TERADATA_SESSIONTYPESITE:
        return &ComManager::sessionTypesSitesReceived;
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

    // Set current user values
    m_currentUser.setFieldValue("user_uuid", user_uuid);
    setCredentials(login_info["user_token"].toString());
    m_tokenRefreshTimer.start();

    // Query versions informations
    doGet(WEB_VERSIONSINFO_PATH);

    doUpdateCurrentUser();

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

    // Refresh token information reply
    if (reply_path == WEB_REFRESH_TOKEN_PATH){
        return handleTokenRefreshReply(data_list);
    }

    // Browse each items received
    QList<TeraData> items;
    TeraDataTypes items_type = TeraData::getDataTypeFromPath(reply_path);
    if (data_list.isArray()){
        const QJsonArray data_array = data_list.array();
        for (const QJsonValue data:data_array){
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
    case TERADATA_TESTTYPE:
        emit testTypesReceived(items, reply_query);
        break;
    case TERADATA_TESTTYPEPROJECT:
        emit testTypesProjectsReceived(items, reply_query);
        break;
    case TERADATA_TESTTYPESITE:
        emit testTypesSitesReceived(items, reply_query);
        break;
    case TERADATA_TEST:
        emit testsReceived(items, reply_query);
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
    case TERADATA_SESSIONTYPESITE:
        emit sessionTypesSitesReceived(items, reply_query);
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
    case TERADATA_SERVICE_SITE:
        emit servicesSitesReceived(items, reply_query);
        break;
    case TERADATA_SERVICE_ACCESS:
        emit servicesAccessReceived(items, reply_query);
        break;
    case TERADATA_SERVICE_CONFIG:
        emit servicesConfigReceived(items, reply_query);
        break;
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
    case TERADATA_ASSET:
        emit assetsReceived(items, reply_query);
        break;
    case TERADATA_LOG_LOG:
        emit logsLogsReceived(items, reply_query);
        break;
    case TERADATA_LOG_LOGIN:
        emit logsLoginReceived(items, reply_query);
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

bool ComManager::handleTokenRefreshReply(const QJsonDocument &refresh_data)
{
    if (!refresh_data.object().contains("refresh_token"))
        return false;
    setCredentials(refresh_data["refresh_token"].toString());
    emit userTokenUpdated();
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
    m_tokenRefreshTimer.stop();
    m_currentUser = TeraData(TERADATA_USER);
    clearCredentials();
}

void ComManager::refreshUserToken()
{
    //qDebug() << "Refreshing user token...";
    doGet(WEB_REFRESH_TOKEN_PATH, QUrlQuery(), true);
}



void ComManager::onNetworkAuthenticationFailed()
{
    emit loginResult(false, tr("Utilisateur ou mot de passe invalide."));
}



/////////////////////////////////////////////////////////////////////////////////////
void ComManager::onWebSocketLoginResult(bool logged_in)
{
    if (!logged_in){
        clearCurrentUser();
        emit loginResult(false, tr("La communication avec le serveur n'a pu être établie."));
        return;
    }
}

