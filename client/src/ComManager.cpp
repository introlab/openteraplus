#include "ComManager.h"
#include <sstream>

ComManager::ComManager(QUrl serverUrl, QObject *parent) :
    QObject(parent),
    m_currentUser(TERADATA_USER)
{
    // Initialize communication objects
    m_netManager = new QNetworkAccessManager(this);
    m_netManager->setCookieJar(&m_cookieJar);

    m_webSocketMan = new WebSocketManager();

    // Setup signals and slots
    // Websocket manager
    connect(m_webSocketMan, &WebSocketManager::serverDisconnected, this, &ComManager::serverDisconnected); // Pass-thru signal
    connect(m_webSocketMan, &WebSocketManager::websocketError, this, &ComManager::socketError); // Pass-thru signal
    connect(m_webSocketMan, &WebSocketManager::loginResult, this, &ComManager::loginResult); // Pass-thru signal

    // Network manager
    connect(m_netManager, &QNetworkAccessManager::authenticationRequired, this, &ComManager::onNetworkAuthenticationRequired);
    connect(m_netManager, &QNetworkAccessManager::encrypted, this, &ComManager::onNetworkEncrypted);
    connect(m_netManager, &QNetworkAccessManager::finished, this, &ComManager::onNetworkFinished);
    connect(m_netManager, &QNetworkAccessManager::networkAccessibleChanged, this, &ComManager::onNetworkAccessibleChanged);
    connect(m_netManager, &QNetworkAccessManager::sslErrors, this, &ComManager::onNetworkSslErrors);

    // Create correct server url
    m_serverUrl.setUrl("https://" + serverUrl.host() + ":" + QString::number(serverUrl.port()));

}

ComManager::~ComManager()
{
    m_webSocketMan->deleteLater();
}

void ComManager::connectToServer(QString username, QString password)
{
    qDebug() << "ComManager::Connecting to " << m_serverUrl.toString();

    m_username = username;
    m_password = password;

    m_loggingInProgress = true; // Indicate that a login request was sent, but not processed

    doQuery(QString(WEB_LOGIN_PATH));

}

void ComManager::disconnectFromServer()
{
    doQuery(QString(WEB_LOGOUT_PATH));
}

bool ComManager::processNetworkReply(QNetworkReply *reply)
{
    QString reply_path = reply->url().path();
    QString reply_data = reply->readAll();
    QUrlQuery reply_query = QUrlQuery(reply->url().query());
    //qDebug() << reply_path << " ---> " << reply_data << ": " << reply_query;

    bool handled = false;

    if (reply->operation()==QNetworkAccessManager::GetOperation){
        if (reply_path == WEB_LOGIN_PATH){
            // Initialize cookies
            m_cookieJar.cookiesForUrl(reply->url());

            handled=handleLoginReply(reply_data);
        }

        if (reply_path == WEB_LOGOUT_PATH){
            emit serverDisconnected();
            handled = true;
        }

        if (reply_path == WEB_FORMS_PATH){
            handled = handleFormReply(reply_query, reply_data);
            if (handled) emit queryResultsOK(reply_path, reply_query);
        }

        if (reply_path == WEB_DEVICEDATAINFO_PATH && reply_query.hasQueryItem(WEB_QUERY_DOWNLOAD)){
            //qDebug() << "Download complete.";
            handled = true;

            // Remove reply from current download list, if present
            if (m_currentDownloads.contains(reply)){
                DownloadedFile* file = m_currentDownloads.take(reply);
                emit downloadCompleted(file);
                file->deleteLater();
            }
        }

        if (!handled){
            // General case
            handled=handleDataReply(reply_path, reply_data, reply_query);
            if (handled) emit queryResultsOK(reply_path, reply_query);
        }
    }

    if (reply->operation()==QNetworkAccessManager::PostOperation){
        handled=handleDataReply(reply_path, reply_data, reply_query);
        if (handled) emit postResultsOK(reply_path);
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
    m_netManager->get(QNetworkRequest(query));
    emit waitingForReply(true);
    emit querying(path);

    LOG_DEBUG("GET: " + path + ", with " + query_args.toString(), "ComManager::doQuery");
}

void ComManager::doPost(const QString &path, const QString &post_data)
{
    QUrl query = m_serverUrl;

    query.setPath(path);
    QNetworkRequest request(query);
    request.setRawHeader("Content-Type", "application/json");
    m_netManager->post(request, post_data.toUtf8());
    emit waitingForReply(true);
    emit posting(path, post_data);

    LOG_DEBUG("POST: " + path + ", with " + post_data, "ComManager::doPost");
}

void ComManager::doDelete(const QString &path, const int &id)
{
    QUrl query = m_serverUrl;

    query.setPath(path);
    query.setQuery("id=" + QString::number(id));
    QNetworkRequest request(query);
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
}

void ComManager::doDownload(const QString &save_path, const QString &path, const QUrlQuery &query_args)
{
    QUrl query = m_serverUrl;

    query.setPath(path);
    if (!query_args.isEmpty()){
        query.setQuery(query_args);
    }
    QNetworkReply* reply = m_netManager->get(QNetworkRequest(query));
    if (reply){
        DownloadedFile* file_to_download = new DownloadedFile(reply, save_path);
        m_currentDownloads[reply] = file_to_download;

        connect(file_to_download, &DownloadedFile::downloadProgress, this, &ComManager::downloadProgress);
    }

    emit waitingForReply(true);

    LOG_DEBUG("DOWNLOADING: " + path + ", with " + query_args.toString() + ", to " + save_path, "ComManager::doQuery");
}

TeraData &ComManager::getCurrentUser()
{
    return m_currentUser;
}

QString ComManager::getCurrentUserSiteRole(int site_id)
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

QString ComManager::getCurrentUserProjectRole(int project_id)
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
    case TERADATA_SERVICE:
        return &ComManager::servicesReceived;
    default:
        LOG_WARNING("Signal for object " + TeraData::getDataTypeName(data_type) + " unspecified.", "ComManager::getSignalFunctionForDataType");
        return nullptr;
    }
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
    QString web_socket_url = login_info["websocket_url"].toString();
    m_webSocketMan->connectWebSocket(web_socket_url, user_uuid);

    // Query connected user information

    m_currentUser.setFieldValue("user_uuid", QUuid(user_uuid));
    doUpdateCurrentUser();

    return true;
}

bool ComManager::handleDataReply(const QString& reply_path, const QString &reply_data, const QUrlQuery &reply_query)
{
    QJsonParseError json_error;

    // Process reply
    QString data_str = reply_data;
    if (data_str.isEmpty())
        data_str = "[]"; // Replace empty string with empty list!

    QJsonDocument data_list = QJsonDocument::fromJson(data_str.toUtf8(), &json_error);
    if (json_error.error!= QJsonParseError::NoError){
        LOG_ERROR("Received a JSON string for " + reply_path + " with " + reply_query.toString() + " with error: " + json_error.errorString(), "ComManager::handleDataReply");
        return false;
    }

    // Browse each items received
    QList<TeraData> items;
    TeraDataTypes items_type = TeraData::getDataTypeFromPath(reply_path);
    for (QJsonValue data:data_list.array()){
        TeraData item_data(items_type, data);

        // Check if the currently connected user was updated and not requesting a list (limited information)
        if (items_type == TERADATA_USER){
            if (m_currentUser.getFieldValue("user_uuid").toUuid() == item_data.getFieldValue("user_uuid").toUuid() &&
                    !reply_query.hasQueryItem(WEB_QUERY_LIST)){
                //m_currentUser = item_data;
                // Update fields that we received with the new values
                m_currentUser.updateFrom(item_data);
                emit currentUserUpdated();
            }
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
    case TERADATA_SESSIONTYPEDEVICETYPE:
        emit sessionTypesDeviceTypesReceived(items, reply_query);
        break;
    case TERADATA_DEVICESUBTYPE:
        emit deviceSubtypesReceived(items, reply_query);
        break;
    case TERADATA_DEVICEDATA:
        emit deviceDatasReceived(items, reply_query);
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
/*    default:
        emit getSignalFunctionForDataType(items_type);*/

    }

    // Always emit generic signal
    emit dataReceived(items_type, items, reply_query);

    return true;
}

bool ComManager::handleFormReply(const QUrlQuery &reply_query, const QString &reply_data)
{
    emit formReceived(reply_query.toString(), reply_data);
    return true;
}



/////////////////////////////////////////////////////////////////////////////////////
void ComManager::onNetworkAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
    Q_UNUSED(reply)
    if (m_loggingInProgress){
        LOG_DEBUG("Sending authentication request...", "ComManager::onNetworkAuthenticationRequired");
        authenticator->setUser(m_username);
        authenticator->setPassword(m_password);
        m_loggingInProgress = false; // Not logging anymore - we sent the credentials
    }else{
        LOG_DEBUG("Authentication error", "ComManager::onNetworkAuthenticationRequired");
        emit loginResult(false);
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
        QString reply_msg = QString::fromUtf8(reply->readAll()).replace("\"", "");
        if (reply_msg.isEmpty() || reply_msg.startsWith("\"\"") || reply_msg == "\n"){
            //reply_msg = tr("Erreur non-détaillée.");
            reply_msg = reply->errorString();
        }
        qDebug() << "ComManager::onNetworkFinished - Reply error: " << reply->error() << ", Reply message: " << reply_msg;
        /*if (reply_msg.isEmpty())
            reply_msg = reply->errorString();*/
        emit networkError(reply->error(), reply_msg);
    }

    reply->deleteLater();
}

void ComManager::onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible)
{
    Q_UNUSED(accessible)

    qDebug() << "ComManager::onNetworkAccessibleChanged";
    //TODO: Emit signal
}

void ComManager::onNetworkSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    Q_UNUSED(reply)
    Q_UNUSED(errors)
    LOG_WARNING("Ignoring SSL errors, this is unsafe", "ComManager::onNetworkSslErrors");
    reply->ignoreSslErrors();
    for(QSslError error : errors){
        LOG_WARNING("Ignored: " + error.errorString(), "ComManager::onNetworkSslErrors");
    }
}
