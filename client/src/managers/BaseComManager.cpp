#include "BaseComManager.h"
#include <QOperatingSystemVersion>

BaseComManager::BaseComManager(QUrl serverUrl, QObject *parent)
    : QObject{parent},
      m_serverUrl(serverUrl)
{

    m_settedCredentials = false;
    m_loggingInProgress = false;

    // Get Operating system information to send to server for logging
    QOperatingSystemVersion os = QOperatingSystemVersion::current();
    m_osName = os.name();
    m_osVersion = QString::number(os.majorVersion()) + "." + QString::number(os.minorVersion()) + "." + QString::number(os.microVersion());

    // Create correct server url
    m_serverUrl.setUrl("https://" + serverUrl.host() + ":" + QString::number(serverUrl.port()));

    // Initialize communication objects
    m_netManager = new QNetworkAccessManager(this);
    m_netManager->setCookieJar(&m_cookieJar);

    // Connect base signals
    connect(m_netManager, &QNetworkAccessManager::finished, this, &BaseComManager::onNetworkFinished);
#ifndef OPENTERA_WEBASSEMBLY
    connect(m_netManager, &QNetworkAccessManager::sslErrors, this, &BaseComManager::onNetworkSslErrors);
    connect(m_netManager, &QNetworkAccessManager::encrypted, this, &BaseComManager::onNetworkEncrypted);
#endif
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
        LOG_DEBUG("POST: " + full_url.toString(), QString(this->metaObject()->className()) + "::doPost");
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

void BaseComManager::doDownload(const QString &path, const QString &save_path, const QString& download_uuid, const QString &save_filename, const QUrlQuery &query_args, const bool &use_token)
{
    QUrl query = m_serverUrl;

    query.setPath(path);

    doDownload(query, save_path, download_uuid, save_filename, query_args, use_token);

}

void BaseComManager::doDownload(const QUrl &full_url, const QString &save_path, const QString &download_uuid, const QString &save_filename, const QUrlQuery &query_args, const bool &use_token)
{
    QUrl query = full_url;

    if (!query_args.isEmpty()){
        query.setQuery(query_args);
    }

    QNetworkRequest* request = new QNetworkRequest(query);
    setRequestCredentials(*request, use_token);
    setRequestLanguage(*request);
    setRequestVersions(*request);

    DownloadingFile* file_to_download = new DownloadingFile(save_path, save_filename);
    file_to_download->setAssociatedUuid(download_uuid);

    // Do the get request
    if (m_currentDownloads.count() < MAX_SIMULTANEOUS_DOWNLOADS){
        startFileDownload(file_to_download, request);
    }else{
        m_waitingDownloads[request] = file_to_download;
    }
    emit downloadProgress(file_to_download); // Emit a request to indicate the file is waiting
}

void BaseComManager::doUpload(const QString &path, const QString &file_name, const QVariantMap extra_headers, const QString &label, const bool &use_token)
{
    QUrl query = m_serverUrl;    query.setPath(path);

    // Prepare request
    QNetworkRequest* request = new QNetworkRequest(query);
    setRequestCredentials(*request, use_token);
    setRequestLanguage(*request);
    setRequestVersions(*request);
    request->setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    // Create file to upload
    UploadingFile* file_to_upload = new UploadingFile(file_name);

    QFile* file = file_to_upload->getFile();
    if (!file) // Invalid file
        return;

    // Set filename info and label if required
    if (!label.isEmpty())
        request->setRawHeader("X-Label", label.toLatin1());

    request->setRawHeader("X-Filename", file_to_upload->getFileName().toLatin1());
    request->setHeader(QNetworkRequest::ContentLengthHeader, file_to_upload->totalBytes());

    // Create extra headers
    for(const QVariant &header_value:extra_headers){
        request->setRawHeader(extra_headers.key(header_value).toUtf8(), header_value.toString().toUtf8());
    }

    // Do the post request
    if (m_currentUploads.count() < MAX_SIMULTANEOUS_UPLOADS){
        startFileUpload(file_to_upload, request);
    }else{
        m_waitingUploads[request] = file_to_upload;
    }
    emit uploadProgress(file_to_upload); // Emit a request to indicate the file is waiting

}

void BaseComManager::doUploadWithMultiPart(const QString &path, const QString &file_name, const QString &form_field_name, const QString &form_infos, const QVariantMap extra_headers, const bool &use_token)
{
    // General query initialization
    QUrl query = m_serverUrl;
    query.setPath(path);

    // Prepare request
    QNetworkRequest* request = new QNetworkRequest(query);
    setRequestCredentials(*request, use_token);
    setRequestLanguage(*request);
    setRequestVersions(*request);

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
    filePart.setHeader(QNetworkRequest::ContentLengthHeader, QVariant(file_to_upload->totalBytes()));
    if (!file){ // Invalid file
        delete multiPart;
        return;
    }

    file->open(QIODevice::ReadOnly);
    filePart.setBodyDevice(file);

    multiPart->append(formPart);
    multiPart->append(filePart);

    file_to_upload->setHttpMultiPart(multiPart);
    if (m_currentUploads.count() < MAX_SIMULTANEOUS_UPLOADS){
        startFileUpload(file_to_upload, request);
    }else{
        m_waitingUploads[request] = file_to_upload;
    }
    emit uploadProgress(file_to_upload); // Emit a request to indicate the file is waiting
}

bool BaseComManager::isDownloadingFiles()
{
    return !m_currentDownloads.isEmpty() || !m_waitingDownloads.isEmpty();
}

bool BaseComManager::isUploadingFiles()
{
    return !m_currentUploads.isEmpty() || !m_waitingUploads.isEmpty();
}

bool BaseComManager::isTransferringFiles()
{
    return !isDownloadingFiles() || !isUploadingFiles();
}

bool BaseComManager::hasDownloadsWaiting()
{
    return !m_waitingDownloads.isEmpty();
}

void BaseComManager::updateWaitingDownloadsQueryParameter(const QString &parameter, const QString &new_value)
{
    for(DownloadingFile* file: std::as_const(m_waitingDownloads)){
        QNetworkRequest* request = m_waitingDownloads.key(file);
        QUrlQuery query(request->url());
        if (query.hasQueryItem(parameter)){
            QUrl url = request->url();
            query.removeAllQueryItems(parameter);
            query.addQueryItem(parameter, new_value);
            url.setQuery(query);
            request->setUrl(url);
        }
    }
}

void BaseComManager::updateWaitingDownloadsQueryParameter(const QString &download_uuid, const QString &parameter, const QString &new_value)
{
    for(DownloadingFile* file: std::as_const(m_waitingDownloads)){
        if (file->getAssociatedUuid() == download_uuid){
            QNetworkRequest* request = m_waitingDownloads.key(file);
            QUrlQuery query(request->url());
            if (query.hasQueryItem(parameter)){
                QUrl url = request->url();
                query.removeAllQueryItems(parameter);
                query.addQueryItem(parameter, new_value);
                url.setQuery(query);
                request->setUrl(url);
            }
        }
    }
}

void BaseComManager::abortDownloads()
{
    for(DownloadingFile* file:std::as_const(m_waitingDownloads)){
        delete m_waitingDownloads.key(file);
        file->deleteLater();
    }
    m_waitingDownloads.clear();

    QList<DownloadingFile*> files = m_currentDownloads.values();
    for(DownloadingFile* file:std::as_const(files)){
        file->abortTransfer(); // Should cascade and be deleted on_Transfer_abort
    }
}

void BaseComManager::abortUploads()
{
    for(UploadingFile* file:std::as_const(m_waitingUploads)){
        delete m_waitingUploads.key(file);
        file->deleteLater();
    }
    m_waitingUploads.clear();

    QList<UploadingFile*> files = m_currentUploads.values();
    for(UploadingFile* file:std::as_const(files)){
        file->abortTransfer(); // Should cascade and be deleted on_Transfer_abort
    }
}

void BaseComManager::abortTransfers()
{
    abortDownloads();
    abortUploads();
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

        // Strip all HTML if needed
        reply_msg = QTextDocumentFragment::fromHtml(reply_msg).toPlainText();

        // Update file status if error occured for a transferring file
        if (m_currentUploads.contains(reply)){
            m_currentUploads[reply]->setStatus(TransferringFile::ERROR);
            m_currentUploads[reply]->setLastError(reply_msg);
            m_currentUploads[reply]->abortTransfer();
        }
        if (m_currentDownloads.contains(reply)){
            m_currentDownloads[reply]->setStatus(TransferringFile::ERROR);
            m_currentDownloads[reply]->setLastError(reply_msg);
            m_currentDownloads[reply]->abortTransfer();
        }

        emit networkError(reply->error(), reply_msg, reply->operation(), status_code, reply->url().path(), QUrlQuery(reply->url().query()));
    }

    reply->deleteLater();

}

#ifndef OPENTERA_WEBASSEMBLY
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
#endif


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

        // Check if we have pending downloads
        if (!m_waitingDownloads.isEmpty()){
            startFileDownload(m_waitingDownloads.first(), m_waitingDownloads.key(m_waitingDownloads.first()));
        }
    }

    if (m_currentUploads.contains(reply)){
        UploadingFile* file = m_currentUploads.take(reply);
        //qDebug() << "Upload completed.";
        emit uploadCompleted(file);
        file->deleteLater();

        // Check if we have pending uploads
        if (!m_waitingUploads.isEmpty()){
            startFileUpload(m_waitingUploads.first(), m_waitingUploads.key(m_waitingUploads.first()));
        }
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

        // Check if we have pending downloads
        if (!m_waitingDownloads.isEmpty()){
            startFileDownload(m_waitingDownloads.first(), m_waitingDownloads.key(m_waitingDownloads.first()));
        }
    }

    if (m_currentUploads.contains(reply)){
        UploadingFile* file = m_currentUploads.take(reply);
        file->deleteLater();

        // Check if we have pending uploads
        if (!m_waitingUploads.isEmpty()){
            startFileUpload(m_waitingUploads.first(), m_waitingUploads.key(m_waitingUploads.first()));
        }
    }

    reply->deleteLater();

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
        //qDebug() << "NOT Using token";
        QString concatenatedCredentials = m_username + ":" + m_password;
        QByteArray data = concatenatedCredentials.toLocal8Bit().toBase64();
        headerData = "Basic " + data;
    }else{
        //qDebug() << "Using token";
        headerData = "OpenTera " + m_token;
    }

    // Pack in credentials
    request.setRawHeader( "Authorization", headerData.toLocal8Bit());

}

void BaseComManager::setRequestVersions(QNetworkRequest &request)
{
    request.setRawHeader("X-Client-Name", QByteArray(OPENTERAPLUS_CLIENT_NAME));
    request.setRawHeader("X-Client-Version", QByteArray(OPENTERAPLUS_VERSION));
    request.setRawHeader("X-OS-Name", m_osName.toUtf8());
    request.setRawHeader("X-OS-Version", m_osVersion.toUtf8());

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

void BaseComManager::startFileUpload(UploadingFile *upload_file, QNetworkRequest *request)
{
    QNetworkReply* reply = nullptr;
    if (upload_file->getHttpMultiPart()){
        // Upload using multipart data
        reply = m_netManager->post(*request, upload_file->getHttpMultiPart());
        if (reply){
             upload_file->getHttpMultiPart()->setParent(reply); // delete the multiPart with the reply
        }else{
            upload_file->getHttpMultiPart()->deleteLater();
            upload_file->setHttpMultiPart(nullptr);
        }
    }else{
        // "Regular" upload
        upload_file->getFile()->open(QIODevice::ReadOnly); // Open the file
        reply = m_netManager->post(*request, upload_file->getFile());
    }

    if (reply){
        upload_file->setNetworkReply(reply);
        upload_file->setStatus(TransferringFile::INPROGRESS);
        m_currentUploads[reply] = upload_file;
        connect(upload_file, &UploadingFile::transferProgress, this, &BaseComManager::onTransferProgress);
        connect(upload_file, &UploadingFile::transferComplete, this, &BaseComManager::onTransferCompleted);
        connect(upload_file, &UploadingFile::transferAborted, this, &BaseComManager::onTransferAborted);
    }else{
        upload_file->setStatus(TransferringFile::ERROR);
        upload_file->setLastError(tr("Impossible de créer la requête"));
        emit transferError(upload_file);
        upload_file->deleteLater();
    }

    if (m_waitingUploads.contains(request))
        m_waitingUploads.remove(request);

    delete request;
}

void BaseComManager::startFileDownload(DownloadingFile *download_file, QNetworkRequest *request)
{
    QNetworkReply* reply = m_netManager->get(*request);

    if (reply){
        download_file->setNetworkReply(reply);
        download_file->setStatus(TransferringFile::INPROGRESS);
        m_currentDownloads[reply] = download_file;
        connect(download_file, &DownloadingFile::transferProgress, this, &BaseComManager::onTransferProgress);
        connect(download_file, &DownloadingFile::transferComplete, this, &BaseComManager::onTransferCompleted);
        connect(download_file, &DownloadingFile::transferAborted, this, &BaseComManager::onTransferAborted);
    }else{
        download_file->setStatus(TransferringFile::ERROR);
        download_file->setLastError(tr("Impossible de créer la requête"));
        emit transferError(download_file);
        download_file->deleteLater();
    }

    if (m_waitingDownloads.contains(request))
        m_waitingDownloads.remove(request);

    delete request;

}
