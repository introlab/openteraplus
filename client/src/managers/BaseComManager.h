#ifndef BASECOMMANAGER_H
#define BASECOMMANAGER_H

#include <QObject>
#include <QTextDocumentFragment>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QAuthenticator>
#include <QHttpMultiPart>
#include <QHttpPart>

#include "data/DownloadingFile.h"
#include "data/UploadingFile.h"

class BaseComManager : public QObject
{
    Q_OBJECT
public:
    explicit BaseComManager(QUrl serverUrl = QUrl(), QObject *parent = nullptr);
    ~BaseComManager();

    virtual void doGet(const QString &path, const QUrlQuery &query_args = QUrlQuery(), const bool &use_token=true);
    virtual void doPost(const QString &path, const QString &post_data, const bool &use_token=true);
    virtual void doPost(const QUrl &full_url, const QString &post_data, const bool &use_token=true);
    virtual void doPostWithParams(const QString &path, const QString &post_data, const QUrlQuery &query_args, const bool &use_token=true);

    virtual void doDelete(const QString &path, const int& id, const bool &use_token=true);

    virtual void doDownload(const QString &path, const QString& save_path, const QString& download_uuid, const QString& save_filename = QString(), const QUrlQuery &query_args = QUrlQuery(), const bool &use_token=true);
    virtual void doDownload(const QUrl &full_url, const QString& save_path, const QString& download_uuid, const QString& save_filename = QString(), const QUrlQuery &query_args = QUrlQuery(), const bool &use_token=true);
    virtual void doUpload(const QString &path, const QString& file_name, const QVariantMap extra_headers = QVariantMap(),
                          const QString& label = QString(), const bool &use_token=true);
    virtual void doUploadWithMultiPart(const QString &path, const QString& file_name, const QString &form_field_name, const QString& form_infos, const QVariantMap extra_headers = QVariantMap(), const bool &use_token=true);

    bool isDownloadingFiles();
    bool isUploadingFiles();
    bool isTransferringFiles();
    bool hasDownloadsWaiting();
    void updateWaitingDownloadsQueryParameter(const QString& parameter, const QString& new_value);
    void updateWaitingDownloadsQueryParameter(const QString& download_uuid, const QString& parameter, const QString& new_value);
    void abortDownloads();
    void abortUploads();
    void abortTransfers();

    void setCredentials(const QString &username, const QString &password);
    void setCredentials(const QString &token);
    void clearCredentials();
    QString getCurrentToken();

    QUrl getServerUrl() const;

private:
    QString                 m_username;
    QString                 m_password;
    QString                 m_token;

    QString                 m_osName;
    QString                 m_osVersion;

    bool                    m_settedCredentials;


protected:
    QUrl                    m_serverUrl;
    QNetworkAccessManager*  m_netManager;
    QNetworkCookieJar       m_cookieJar;

    bool                    m_loggingInProgress; // Indicates a login procedure is going on

    QMap<QNetworkReply*, DownloadingFile*>      m_currentDownloads;
    QMap<QNetworkRequest*, DownloadingFile*>    m_waitingDownloads;

    QMap<QNetworkReply*, UploadingFile*>        m_currentUploads;
    QMap<QNetworkRequest*, UploadingFile*>      m_waitingUploads;


    virtual bool processNetworkReply(QNetworkReply* reply) = 0;
    void setRequestLanguage(QNetworkRequest &request);
    void setRequestCredentials(QNetworkRequest &request, const bool &use_token);
    void setRequestVersions(QNetworkRequest &request);

    QString filterReplyString(const QString &data_str);

    void startFileUpload(UploadingFile* upload_file, QNetworkRequest* request);
    void startFileDownload(DownloadingFile* download_file, QNetworkRequest* request);


protected slots:
    virtual void onNetworkFinished(QNetworkReply *reply);

#ifndef OPENTERA_WEBASSEMBLY
    virtual void onNetworkSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
    void onNetworkEncrypted(QNetworkReply *reply);
#endif

    void onNetworkAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);

    void onTransferProgress(TransferringFile* file);
    void onTransferCompleted(TransferringFile* file);
    void onTransferAborted(TransferringFile* file);

signals:
    void waitingForReply(bool waiting);
    void networkError(QNetworkReply::NetworkError error, QString error_str, QNetworkAccessManager::Operation op, int status_code, QString path, QUrlQuery url_query);

    void networkAuthFailed();

    void queryResultsOK(QString path, QUrlQuery url_query);
    void postResultsOK(QString reply_path, QString reply_data);
    void deleteResultsOK(QString path, int id);
    void posting(QString path, QString data);
    void querying(QString path);
    void deleting(QString path);

    // File transfer
    void downloadCompleted(DownloadingFile* file);
    void downloadProgress(DownloadingFile* file);

    void uploadCompleted(UploadingFile* file);
    void uploadProgress(UploadingFile* file);

    void transferProgress(TransferringFile* file);
    void transferCompleted(TransferringFile* file);
    void transferAborted(TransferringFile* file);
    void transferError(TransferringFile* file);

};

#endif // BASECOMMANAGER_H
