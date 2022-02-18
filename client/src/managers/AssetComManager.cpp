#include "AssetComManager.h"

AssetComManager::AssetComManager(ComManager *comManager, QObject *parent)
    : BaseComManager(comManager->getServerUrl(), parent),
      m_comManager(comManager)
{
    // Set initial user token
    setCredentials(m_comManager->getCurrentToken());

    // Connect signals
    connectSignals();
}

void AssetComManager::doGet(const QString &path, const QUrlQuery &query_args, const bool &use_token)
{
    Q_UNUSED(use_token)
    BaseComManager::doGet(path, query_args, true); // Always use token!
}

void AssetComManager::doPost(const QString &path, const QString &post_data, const bool &use_token)
{
    Q_UNUSED(use_token)
    BaseComManager::doPost(path, post_data, true); // Always use token!
}

void AssetComManager::doPost(const QUrl &full_url, const QString &post_data, const bool &use_token)
{
    Q_UNUSED(use_token)
    BaseComManager::doPost(full_url, post_data, true); // Always use token!
}

void AssetComManager::doDelete(const QString &path, const int &id, const bool &use_token)
{
    Q_UNUSED(use_token)
    BaseComManager::doDelete(path, id, true); // Always use token!
}

void AssetComManager::doDownload(const QUrl &full_url, const QString &save_path, const QString &save_filename, const QUrlQuery &query_args, const bool &use_token)
{
    Q_UNUSED(use_token)
    BaseComManager::doDownload(full_url, save_path, save_filename, query_args, true); // Always use token
}

void AssetComManager::doUploadWithMultiPart(const QString &path, const QString &file_name, const QString &form_field_name, const QString &form_infos, const QVariantMap extra_headers, const bool &use_token)
{
    Q_UNUSED(use_token)
    BaseComManager::doUploadWithMultiPart(path, file_name, form_field_name, form_infos, extra_headers, true); // Always use token
}

void AssetComManager::doDelete(const QUrl &full_url, const QString &uuid, const QString& access_token)
{
    // Delete an asset with the specified uuid
    QUrl query = full_url;
    query.setQuery("uuid=" + uuid + "&access_token=" + access_token);
    QNetworkRequest request(query);
    setRequestCredentials(request, true);
    setRequestLanguage(request);
    setRequestVersions(request);

    m_netManager->deleteResource(request);
    emit waitingForReply(true);
    emit deleting(query.path());

    LOG_DEBUG("DELETE: " + full_url.toString() + ", with uuid=" + uuid, QString(this->metaObject()->className()) + "::doDelete");
}

void AssetComManager::connectSignals()
{
    connect(m_comManager, &ComManager::userTokenUpdated, this, &AssetComManager::handleUserTokenUpdated);
}

bool AssetComManager::handleDataReply(const QString &reply_path, const QString &reply_data, const QUrlQuery &reply_query)
{
    QJsonParseError json_error;

    // Process reply
    QString data_str = filterReplyString(reply_data);

    QJsonDocument data_list = QJsonDocument::fromJson(data_str.toUtf8(), &json_error);
    if (json_error.error!= QJsonParseError::NoError){
        LOG_ERROR("Received a JSON string for " + reply_path + " with " + reply_query.toString() + " with error: " + json_error.errorString(), "AssetComManager::handleDataReply");
        return false;
    }

    // Browse each items received
    QList<QJsonObject> items;
    if (data_list.isArray()){
        QJsonArray data_list_array = data_list.array();
        for (const QJsonValue &data:qAsConst(data_list_array)){
            items.append(data.toObject());
        }
    }else{
        items.append(data_list.object());
    }

    // Check to emit correct signals for specific data types
    if (reply_path.endsWith("/assets/infos")){
        emit assetsInfosReceived(items, reply_query, reply_path);
    }

    // Always emit generic signal
    emit dataReceived(items, reply_query);

    return true;
}

bool AssetComManager::processNetworkReply(QNetworkReply *reply)
{
    QString reply_path = reply->url().path();
    QString reply_data = reply->readAll();
    QUrlQuery reply_query = QUrlQuery(reply->url().query());
    //qDebug() << reply_path << " ---> " << reply_data << ": " << reply->url().query();

    bool handled = false;

    if (reply->operation()==QNetworkAccessManager::GetOperation){
        if (!handled){
            // General case
            handled = handleDataReply(reply_path, reply_data, reply_query);
            if (handled) emit queryResultsOK(reply_path, reply_query);
        }
    }

    if (reply->operation()==QNetworkAccessManager::PostOperation){
        if (!handled){
            if (reply_path.endsWith("/assets")){
                // We posted a new asset - we received its info, change the path so it is handled properly
                reply_path = reply_path.append("/infos");
            }
            handled=handleDataReply(reply_path, reply_data, reply_query);
            if (handled) emit postResultsOK(reply_path);
        }
    }

    if (reply->operation()==QNetworkAccessManager::DeleteOperation){
        if (reply_path.endsWith("/assets")){
            QString uuid;
            if (reply_query.hasQueryItem("uuid")){
                uuid = reply_query.queryItemValue("uuid");
            }
            emit deleteUuidResultOK(reply_path, uuid);
        }else{
            // Extract id from url
            int id = 0;
            if (reply_query.hasQueryItem("id")){
                id = reply_query.queryItemValue("id").toInt();
            }
            emit deleteResultsOK(reply_path, id);
        }

        handled=true;
    }

    return handled;
}

void AssetComManager::handleUserTokenUpdated()
{
    // Update token
    setCredentials(m_comManager->getCurrentToken());
}
