#include "BaseServiceComManager.h"

BaseServiceComManager::BaseServiceComManager(ComManager *comManager, QString service_key, QObject *parent)
    : BaseComManager{comManager->getServerUrl(), parent}
    , m_comManager(comManager)
    , m_serviceKey(service_key)
{
    // Set initial user token
    setCredentials(m_comManager->getCurrentToken());

    // Connect signals
    connectSignals();

    // Query service information
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_SERVICE_KEY, m_serviceKey);
    m_comManager->doGet(WEB_SERVICEINFO_PATH, args);

}

void BaseServiceComManager::doGet(const QString &path, const QUrlQuery &query_args, const bool &use_token)
{
    Q_UNUSED(use_token)
    if (!isReady()){
        qWarning() << "ServiceComManager is not ready yet - ignoring GET.";
        return;
    }

    BaseComManager::doGet(getServiceEndpoint(path), query_args, true);
}

void BaseServiceComManager::doPost(const QString &path, const QString &post_data, const bool &use_token)
{
    Q_UNUSED(use_token)
    if (!isReady()){
        qWarning() << "ServiceComManager is not ready yet - ignoring POST.";
        return;
    }
    BaseComManager::doPost(getServiceEndpoint(path), post_data, true);
}

void BaseServiceComManager::doPost(const QUrl &full_url, const QString &post_data, const bool &use_token)
{
    Q_UNUSED(use_token)
    if (!isReady()){
        qWarning() << "ServiceComManager is not ready yet - ignoring POST.";
        return;
    }
    BaseComManager::doPost(getServiceEndpoint(full_url.toString()), post_data, true);
}

void BaseServiceComManager::doPostWithParams(const QString &path, const QString &post_data, const QUrlQuery &query_args, const bool &use_token)
{
    Q_UNUSED(use_token)
    if (!isReady()){
        qWarning() << "ServiceComManager is not ready yet - ignoring POST.";
        return;
    }
    BaseComManager::doPostWithParams(getServiceEndpoint(path), post_data, query_args, true);
}

void BaseServiceComManager::doDelete(const QString &path, const int &id, const bool &use_token)
{
    Q_UNUSED(use_token)
    if (!isReady()){
        qWarning() << "ServiceComManager is not ready yet - ignoring DELETE.";
        return;
    }
    BaseComManager::doDelete(getServiceEndpoint(path), id, true);
}

void BaseServiceComManager::doDownload(const QString &path, const QString &save_path, const QString &download_uuid, const QString &save_filename, const QUrlQuery &query_args, const bool &use_token)
{
    Q_UNUSED(use_token)
    if (!isReady()){
        qWarning() << "ServiceComManager is not ready yet - ignoring DOWNLOAD.";
        return;
    }
    BaseComManager::doDownload(getServiceEndpoint(path), save_path, download_uuid, save_filename, query_args);
}

void BaseServiceComManager::doDownload(const QUrl &full_url, const QString &save_path, const QString &download_uuid, const QString &save_filename, const QUrlQuery &query_args, const bool &use_token)
{
    Q_UNUSED(use_token)
    if (!isReady()){
        qWarning() << "ServiceComManager is not ready yet - ignoring DOWNLOAD.";
        return;
    }
    BaseComManager::doDownload(full_url, save_path, download_uuid, save_filename, query_args);
}

void BaseServiceComManager::doUpload(const QString &path, const QString &file_name, const QVariantMap extra_headers, const QString &label, const bool &use_token)
{
    Q_UNUSED(use_token)
    if (!isReady()){
        qWarning() << "ServiceComManager is not ready yet - ignoring UPLOAD.";
        return;
    }
    BaseComManager::doUpload(getServiceEndpoint(path), file_name, extra_headers, label);
}

void BaseServiceComManager::doUploadWithMultiPart(const QString &path, const QString &file_name, const QString &form_field_name, const QString &form_infos, const QVariantMap extra_headers, const bool &use_token)
{
    Q_UNUSED(use_token)
    if (!isReady()){
        qWarning() << "ServiceComManager is not ready yet - ignoring UPLOAD.";
        return;
    }
    BaseComManager::doUploadWithMultiPart(getServiceEndpoint(path), file_name, form_field_name, form_infos, extra_headers);
}

bool BaseServiceComManager::isReady()
{
    return !m_service.isNew();
}

void BaseServiceComManager::connectSignals()
{
    connect(m_comManager, &ComManager::userTokenUpdated, this, &BaseServiceComManager::handleUserTokenUpdated);
    connect(m_comManager, &ComManager::servicesReceived, this, &BaseServiceComManager::processServicesReply);
}

bool BaseServiceComManager::handleDataReply(const QString &reply_path, const QString &reply_data, const QUrlQuery &reply_query)
{
    QJsonParseError json_error;

    // Process reply
    QString data_str = filterReplyString(reply_data);
    if (preHandleData(reply_path, data_str, reply_query)){
        return true;
    }

    QJsonDocument data_list = QJsonDocument::fromJson(data_str.toUtf8(), &json_error);
    if (json_error.error!= QJsonParseError::NoError){
        LOG_ERROR("Received a JSON string for " + reply_path + " with " + reply_query.toString() + " with error: " + json_error.errorString(), "DashboardsComManager::handleDataReply");
        return false;
    }

    // Browse each items received
    QList<QJsonObject> items;
    if (data_list.isArray()){
        QJsonArray data_list_array = data_list.array();
        for (const QJsonValue &data:std::as_const(data_list_array)){
            items.append(data.toObject());
        }
    }else{
        items.append(data_list.object());
    }

    postHandleData(items, reply_path, reply_query);

    // Always emit generic signal
    emit dataReceived(items, reply_path, reply_query);

    return true;
}

bool BaseServiceComManager::preHandleData(const QString &reply_path, const QString &data, const QUrlQuery &reply_query)
{
    Q_UNUSED(reply_path)
    Q_UNUSED(data)
    Q_UNUSED(reply_query)
    return false; // No prehandling by default
}

QString BaseServiceComManager::getServiceEndpoint(const QString& path)
{
    return m_service.getFieldValue("service_clientendpoint").toString() + "/" + path;
}

bool BaseServiceComManager::processNetworkReply(QNetworkReply *reply)
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
            handled = handleDataReply(reply_path, reply_data, reply_query);
            if (handled) emit postResultsOK(reply_path, reply_data);
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

void BaseServiceComManager::handleUserTokenUpdated()
{
    // Update token
    setCredentials(m_comManager->getCurrentToken());

    emit userTokenUpdated();
}

void BaseServiceComManager::processServicesReply(QList<TeraData> services, QUrlQuery reply_query)
{
    for(const TeraData &service:services){
        if (service.hasFieldName("service_key")){
            if (service.getFieldValue("service_key").toString() == m_serviceKey){
                m_service = service;
                emit readyChanged(true);
                disconnect(m_comManager, &ComManager::servicesReceived, this, &BaseServiceComManager::processServicesReply);
                break;
            }
        }
    }
}
