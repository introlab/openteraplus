#include "BaseComManager.h"

BaseComManager::BaseComManager(QUrl serverUrl, QObject *parent)
    : QObject{parent},
      m_serverUrl(serverUrl)
{

    m_settedCredentials = false;
    m_loggingInProgress = false;

    // Create correct server url
    m_serverUrl.setUrl("https://" + serverUrl.host() + ":" + QString::number(serverUrl.port()));

    // Initialize communication objects
    m_netManager = new QNetworkAccessManager(this);
    m_netManager->setCookieJar(&m_cookieJar);

    // Connect base signals
    connect(m_netManager, &QNetworkAccessManager::finished, this, &BaseComManager::onNetworkFinished);
    connect(m_netManager, &QNetworkAccessManager::sslErrors, this, &BaseComManager::onNetworkSslErrors);
    connect(m_netManager, &QNetworkAccessManager::encrypted, this, &BaseComManager::onNetworkEncrypted);
    connect(m_netManager, &QNetworkAccessManager::authenticationRequired, this, &BaseComManager::onNetworkAuthenticationRequired);

}

BaseComManager::~BaseComManager()
{
    m_netManager->deleteLater();
    qDeleteAll(m_currentDownloads);
    qDeleteAll(m_currentUploads);

}

void BaseComManager::doGet(const QString &path, const QUrlQuery &query_args, const bool &use_token)
{
    QUrl query = m_serverUrl;

    query.setPath(path);
    if (!query_args.isEmpty()){
        query.setQuery(query_args);
    }
    QNetworkRequest request(query);

    setRequestCredentials(request, use_token);
    setRequestLanguage(request);
    setRequestVersions(request);

    m_netManager->get(request);
    emit waitingForReply(true);
    emit querying(path);

    if (!query_args.isEmpty())
        LOG_DEBUG("GET: " + path + " with " + query_args.toString(), QString(this->metaObject()->className()) + "::doQuery");
    else
        LOG_DEBUG("GET: " + path, QString(this->metaObject()->className()) + "::doQuery");
}

void BaseComManager::doPost(const QString &path, const QString &post_data, const bool &use_token)
{
    QUrl query = m_serverUrl;

    query.setPath(path);

    doPost(query, post_data, use_token);
    emit posting(path, post_data);
}

void BaseComManager::doPost(const QUrl &full_url, const QString &post_data, const bool &use_token)
{
    QNetworkRequest request(full_url);
    setRequestCredentials(request, use_token);
    setRequestLanguage(request);
    setRequestVersions(request);

    request.setRawHeader("Content-Type", "application/json");

    m_netManager->post(request, post_data.toUtf8());
    emit waitingForReply(true);

    #ifndef QT_NO_DEBUG
        LOG_DEBUG("POST: " + full_url.toString() + ", with " + post_data, QString(this->metaObject()->className()) + "::doPost");
    #else
        // Strip data from logging in release, since this might contains passwords!
        LOG_DEBUG("POST: " + path, QString(this->metaObject()->className()) + "::doPost");
    #endif
}

void BaseComManager::doDelete(const QString &path, const int &id, const bool &use_token)
{
    QUrl query = m_serverUrl;

    query.setPath(path);
    query.setQuery("id=" + QString::number(id));
    QNetworkRequest request(query);
    setRequestCredentials(request, use_token);
    setRequestLanguage(request);
    setRequestVersions(request);

    m_netManager->deleteResource(request);
    emit waitingForReply(true);
    emit deleting(path);

    LOG_DEBUG("DELETE: " + path + ", with id=" + QString::number(id), QString(this->metaObject()->className()) + "::doDelete");
}

void BaseComManager::doDownload(const QString &save_path, const QString &path, const QUrlQuery &query_args, const bool &use_token)
{
    QUrl query = m_serverUrl;

    query.setPath(path);
    if (!query_args.isEmpty()){
        query.setQuery(query_args);
    }

    QNetworkRequest request(query);
    setRequestCredentials(request, use_token);
    setRequestLanguage(request);
    setRequestVersions(request);

    QNetworkReply* reply = m_netManager->get(request);
    if (reply){
        DownloadingFile* file_to_download = new DownloadingFile(save_path);
        file_to_download->setNetworkReply(reply);
        m_currentDownloads[reply] = file_to_download;

        connect(file_to_download, &DownloadingFile::transferProgress, this, &BaseComManager::onTransferProgress);
        connect(file_to_download, &DownloadingFile::transferComplete, this, &BaseComManager::onTransferCompleted);
        connect(file_to_download, &DownloadingFile::transferAborted, this, &BaseComManager::onTransferAborted);
    }

    emit waitingForReply(true);

    LOG_DEBUG("DOWNLOADING: " + path + ", with " + query_args.toString() + ", to " + save_path, "ComManager::doQuery");
}

void BaseComManager::doUpload(const QString &path, const QString &file_name, const QVariantMap extra_headers, const QString &label, const bool &use_token)
{
    QUrl query = m_serverUrl;
    query.setPath(path);

    // Prepare request
    QNetworkRequest request(query);
    setRequestCredentials(request, use_token);
    setRequestLanguage(request);
    setRequestVersions(request);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    // Create file to upload
    UploadingFile* file_to_upload = new UploadingFile(file_name);
    QFile* file = file_to_upload->getFile();
    if (!file) // Invalid file
        return;

    // Set filename info and label if required
    if (!label.isEmpty())
        request.setRawHeader("X-Label", label.toLatin1());

    request.setRawHeader("X-Filename", file_to_upload->getFileName().toLatin1());
    request.setHeader(QNetworkRequest::ContentLengthHeader, file_to_upload->totalBytes());

    // Create extra headers
    for(const QVariant &header_value:extra_headers){
        request.setRawHeader(extra_headers.key(header_value).toUtf8(), header_value.toString().toUtf8());
    }

    // Do the post request
    file->open(QIODevice::ReadOnly); // Open the file
    QNetworkReply* reply = m_netManager->post(request, file);
    if (reply){
        file_to_upload->setNetworkReply(reply);
        m_currentUploads[reply] = file_to_upload;
        connect(file_to_upload, &UploadingFile::transferProgress, this, &BaseComManager::onTransferProgress);
        connect(file_to_upload, &UploadingFile::transferComplete, this, &BaseComManager::onTransferCompleted);
        connect(file_to_upload, &UploadingFile::transferAborted, this, &BaseComManager::onTransferAborted);

    }else{
        file_to_upload->deleteLater();
    }

}

void BaseComManager::doUploadWithMultiPart(const QString &path, const QString &file_name, const QString &form_field_name, const QString &form_infos, const QVariantMap extra_headers, const bool &use_token)
{
    // General query initialization
    QUrl query = m_serverUrl;
    query.setPath(path);

    // Prepare request
    QNetworkRequest request(query);
    setRequestCredentials(request, use_token);
    setRequestLanguage(request);
    setRequestVersions(request);

    // Multipart construction
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    // Must remove "/" from boundary or it won't work.
    multiPart->setBoundary(QString(multiPart->boundary()).replace('/', '_').toLatin1());

    QHttpPart formPart;
    formPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    formPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"" + form_field_name + "\""));
    formPart.setBody(form_infos.toUtf8());

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));

    // Create file to upload
    UploadingFile* file_to_upload = new UploadingFile(file_name);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + file_to_upload->getFileName() + "\""));
    QFile* file = file_to_upload->getFile();
    if (!file){ // Invalid file
        delete multiPart;
        return;
    }

    file->open(QIODevice::ReadOnly);
    filePart.setBodyDevice(file);

    multiPart->append(formPart);
    multiPart->append(filePart);

    QNetworkReply* reply = m_netManager->post(request, multiPart);
    if (reply){
        multiPart->setParent(reply); // delete the multiPart with the reply
        file_to_upload->setNetworkReply(reply);
        m_currentUploads[reply] = file_to_upload;
        connect(file_to_upload, &UploadingFile::transferProgress, this, &BaseComManager::onTransferProgress);
        connect(file_to_upload, &UploadingFile::transferComplete, this, &BaseComManager::onTransferCompleted);
        connect(file_to_upload, &UploadingFile::transferAborted, this, &BaseComManager::onTransferAborted);

    }else{
        //file_to_upload->deleteLater();
        multiPart->deleteLater();
    }


}

bool BaseComManager::hasPendingDownloads()
{
    return !m_currentDownloads.isEmpty();
}

bool BaseComManager::hasPendingUploads()
{
    return !m_currentUploads.isEmpty();
}

bool BaseComManager::hasPendingFileTransfers()
{
    return !m_currentDownloads.isEmpty() || !m_currentUploads.isEmpty();
}


void BaseComManager::onNetworkFinished(QNetworkReply *reply){
    emit waitingForReply(false);

    if (reply->error() == QNetworkReply::NoError)
    {
        // Process network reply
        if (!processNetworkReply(reply)){
            LOG_WARNING("Unhandled reply - " + reply->url().path(), QString(this->metaObject()->className()) + "::onNetworkFinished");
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
        LOG_ERROR(QString(this->metaObject()->className()) + "::onNetworkFinished - Reply error: " + reply->errorString() + ", Reply message: " + reply_msg,
                  QString(this->metaObject()->className()) + "::onNetworkFinished");
        /*if (reply_msg.isEmpty())
            reply_msg = reply->errorString();*/
        emit networkError(reply->error(), reply_msg, reply->operation(), status_code);
    }

    reply->deleteLater();

}

void BaseComManager::onNetworkSslErrors(QNetworkReply *reply, const QList<QSslError> &errors){
    Q_UNUSED(reply)
    Q_UNUSED(errors)
    LOG_WARNING("Ignoring SSL errors, this is unsafe", QString(this->metaObject()->className()) + "::onNetworkSslErrors");
    reply->ignoreSslErrors();
    for(const QSslError &error : errors){
        LOG_WARNING("Ignored: " + error.errorString(), QString(this->metaObject()->className()) + "::onNetworkSslErrors");
    }

}

void BaseComManager::onNetworkEncrypted(QNetworkReply *reply){
    Q_UNUSED(reply)
    //qDebug() << QString(this->metaObject()->className()) + "::onNetworkEncrypted";
}

void BaseComManager::onNetworkAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
    Q_UNUSED(reply)
    // If we are logging in, credentials were already sent, and if we get here, it's because they were
    // rejected
    if (!m_settedCredentials && !m_loggingInProgress){
        LOG_DEBUG("Sending authentication request...", QString(this->metaObject()->className()) + "::onNetworkAuthenticationRequired");
        authenticator->setUser(m_username);
        authenticator->setPassword(m_password);
        m_settedCredentials = true; // We setted the credentials in authentificator
    }else{
        LOG_DEBUG("Authentication error", QString(this->metaObject()->className()) + "::onNetworkAuthenticationRequired");
        emit networkAuthFailed();
    }
}

void BaseComManager::onTransferProgress(TransferringFile *file)
{
    if (dynamic_cast<DownloadingFile*>(file)){
        emit downloadProgress(dynamic_cast<DownloadingFile*>(file));
    }

    if (dynamic_cast<UploadingFile*>(file)){
        emit uploadProgress(dynamic_cast<UploadingFile*>(file));
    }

    emit transferProgress(file);

}

void BaseComManager::onTransferCompleted(TransferringFile *file)
{
    QNetworkReply* reply = file->getNetworkReply();

    // Process file transfers
    if (m_currentDownloads.contains(reply)){
        DownloadingFile* file = m_currentDownloads.take(reply);
        emit downloadCompleted(file);
        file->deleteLater();
    }

    if (m_currentUploads.contains(reply)){
        UploadingFile* file = m_currentUploads.take(reply);
        //qDebug() << "Upload completed.";
        emit uploadCompleted(file);
        file->deleteLater();
    }

    emit transferCompleted(file);
}

void BaseComManager::onTransferAborted(TransferringFile *file)
{
    QNetworkReply* reply = file->getNetworkReply();

    // Process file transfers
    if (m_currentDownloads.contains(reply)){
        DownloadingFile* file = m_currentDownloads.take(reply);
        file->deleteLater();
    }

    if (m_currentUploads.contains(reply)){
        UploadingFile* file = m_currentUploads.take(reply);
        file->deleteLater();
    }

    emit transferAborted(file);
}

void BaseComManager::setRequestLanguage(QNetworkRequest &request) {
    //Locale will be initialized with default locale
    QString localeString = QLocale().bcp47Name();
    //qDebug() << "localeString : " << localeString;
    request.setRawHeader(QByteArray("Accept-Language"), localeString.toUtf8());
}

void BaseComManager::setRequestCredentials(QNetworkRequest &request, const bool &use_token)
{
    //Needed?
    request.setAttribute(QNetworkRequest::AuthenticationReuseAttribute, false);

    // Pack in credentials
    QString headerData;
    if (!use_token){
        QString concatenatedCredentials = m_username + ":" + m_password;
        QByteArray data = concatenatedCredentials.toLocal8Bit().toBase64();
        headerData = "Basic " + data;
    }else
        headerData = "OpenTera " + m_token;


    // Pack in credentials
    request.setRawHeader( "Authorization", headerData.toLocal8Bit());

}

void BaseComManager::setRequestVersions(QNetworkRequest &request)
{
    request.setRawHeader("X-Client-Name", QByteArray(OPENTERAPLUS_CLIENT_NAME));
    request.setRawHeader("X-Client-Version", QByteArray(OPENTERAPLUS_VERSION));
}

void BaseComManager::setCredentials(const QString &username, const QString &password){
    m_username = username;
    m_password = password;

    m_settedCredentials = false; // username - password credentials are now unset - will update Authorization on next update
}

void BaseComManager::setCredentials(const QString &token){
    m_token = token;
}

void BaseComManager::clearCredentials()
{
    m_username.clear();
    m_password.clear();
    m_token.clear();
    m_settedCredentials = false;
}

QString BaseComManager::getCurrentToken()
{
    return m_token;
}

QUrl BaseComManager::getServerUrl() const{
    return m_serverUrl;
}

QString BaseComManager::filterReplyString(const QString &data_str)
{
    QString filtered_str = data_str;
    if (data_str.isEmpty() || data_str == "\n" || data_str == "null\n")
        filtered_str = "[]"; // Replace empty string with empty list!

    return filtered_str;
}
