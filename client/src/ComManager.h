#ifndef COMMANAGER_H
#define COMMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QAuthenticator>
#include <QNetworkProxy>
#include <QSslPreSharedKeyAuthenticator>
#include <QNetworkCookieJar>
#include <QNetworkCookie>

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonValue>

#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <QUuid>

#include "Logger.h"
#include "WebAPI.h"

#include "TeraData.h"
#include "DownloadedFile.h"
#include "WebSocketManager.h"


class ComManager : public QObject
{
    Q_OBJECT
public:
    explicit ComManager(QUrl serverUrl, QObject *parent = nullptr);
    ~ComManager();

    void connectToServer(QString username, QString password);
    void disconnectFromServer();

    bool processNetworkReply(QNetworkReply* reply);
    void doQuery(const QString &path, const QUrlQuery &query_args = QUrlQuery());
    void doPost(const QString &path, const QString &post_data);
    void doDelete(const QString &path, const int& id);
    void doUpdateCurrentUser();
    void doDownload(const QString& save_path, const QString &path, const QUrlQuery &query_args = QUrlQuery());

    TeraData &getCurrentUser();
    QString getCurrentUserSiteRole(int site_id);
    QString getCurrentUserProjectRole(int project_id);
    bool isCurrentUserSuperAdmin();
    typedef void (ComManager::* signal_ptr)(QList<TeraData>, QUrlQuery);

    bool hasPendingDownloads();

    static signal_ptr getSignalFunctionForDataType(const TeraDataTypes& data_type);

protected:
    bool handleLoginReply(const QString& reply_data);
    bool handleDataReply(const QString& reply_path, const QString& reply_data, const QUrlQuery& reply_query);
    bool handleFormReply(const QUrlQuery& reply_query, const QString& reply_data);


    QUrl                    m_serverUrl;
    QNetworkAccessManager*  m_netManager;
    QNetworkCookieJar       m_cookieJar;
    WebSocketManager*       m_webSocketMan;

    QMap<QNetworkReply*, DownloadedFile*>   m_currentDownloads;

    bool                    m_loggingInProgress;

    QString                 m_username;
    QString                 m_password;

    TeraData                m_currentUser;

signals:
    void serverDisconnected();
    void networkError(QNetworkReply::NetworkError, QString);
    void socketError(QAbstractSocket::SocketError, QString);
    void waitingForReply(bool waiting);

    void loginResult(bool logged_in);

    void currentUserUpdated();

    void formReceived(QString form_type, QString data);

    // Generic data signal, emitted for all data type.
    void dataReceived(TeraDataTypes data_type, QList<TeraData> generic_list, QUrlQuery reply_query);

    // Specific data signals
    void usersReceived(QList<TeraData> user_list, QUrlQuery reply_query);
    void sitesReceived(QList<TeraData> site_list, QUrlQuery reply_query);
    void sessionTypesReceived(QList<TeraData> st_list, QUrlQuery reply_query);
    void testDefsReceived(QList<TeraData> tests_list, QUrlQuery reply_query);
    void projectsReceived(QList<TeraData> projects_list, QUrlQuery reply_query);
    void devicesReceived(QList<TeraData> devices_list, QUrlQuery reply_query);
    void participantsReceived(QList<TeraData> participants_list, QUrlQuery reply_query);
    void groupsReceived(QList<TeraData> groups_list, QUrlQuery reply_query);
    void siteAccessReceived(QList<TeraData> access_list, QUrlQuery reply_query);
    void projectAccessReceived(QList<TeraData> access_list, QUrlQuery reply_query);
    void sessionsReceived(QList<TeraData> sessions_list, QUrlQuery reply_query);
    void deviceSitesReceived(QList<TeraData> device_sites_list, QUrlQuery reply_query);
    void deviceProjectsReceived(QList<TeraData> device_projects_list, QUrlQuery reply_query);
    void deviceParticipantsReceived(QList<TeraData> device_participants_list, QUrlQuery reply_query);
    void sessionTypesDeviceTypesReceived(QList<TeraData> session_types_device_types_list, QUrlQuery reply_query);
    void sessionTypesProjectsReceived(QList<TeraData> session_types_projects_list, QUrlQuery reply_query);
    void deviceDatasReceived(QList<TeraData> device_data_list, QUrlQuery reply_query);
    void sessionEventsReceived(QList<TeraData> events_list, QUrlQuery reply_query);
    void deviceSubtypesReceived(QList<TeraData> device_subtypes_list, QUrlQuery reply_query);
    void userGroupsReceived(QList<TeraData> user_groups_list, QUrlQuery reply_query);

    //void queryResultsReceived(QString object, QUrlQuery url_query, QString data);
    //void postResultsReceived(QString path, QString data);
    void queryResultsOK(QString path, QUrlQuery url_query);
    void postResultsOK(QString path);
    void deleteResultsOK(QString path, int id);
    void posting(QString path, QString data);
    void querying(QString path);
    void deleting(QString path);

    void downloadCompleted(DownloadedFile* file);
    void downloadProgress(DownloadedFile* file);

public slots:

private slots:
    // Network
    void onNetworkAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
    void onNetworkEncrypted(QNetworkReply *reply);
    void onNetworkFinished(QNetworkReply *reply);
    void onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);
    void onNetworkSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

};

#endif // COMMANAGER_H
