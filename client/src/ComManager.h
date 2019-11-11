#ifndef COMMANAGER_H
#define COMMANAGER_H

#include <QObject>
#include <QWebSocket>
#include <QAbstractSocket>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QAuthenticator>
#include <QNetworkProxy>
#include <QSslPreSharedKeyAuthenticator>
#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QWebSocket>

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
    typedef void (ComManager::* signal_ptr)(QList<TeraData>);

    bool hasPendingDownloads();

    static signal_ptr getSignalFunctionForDataType(const TeraDataTypes& data_type);

protected:
    bool handleLoginReply(const QString& reply_data);
    bool handleDataReply(const QString& reply_path, const QString& reply_data, const QUrlQuery& reply_query);
    bool handleFormReply(const QUrlQuery& reply_query, const QString& reply_data);


    QUrl                    m_serverUrl;
    QNetworkAccessManager*  m_netManager;
    QNetworkCookieJar       m_cookieJar;
    QWebSocket*             m_webSocket;

    QMap<QNetworkReply*, DownloadedFile*>   m_currentDownloads;

    bool                    m_loggingInProgress;

    QString                 m_username;
    QString                 m_password;

    QTimer                  m_connectTimer;

    TeraData                m_currentUser;

signals:
    void serverDisconnected();
    void serverError(QAbstractSocket::SocketError, QString);
    void networkError(QNetworkReply::NetworkError, QString);
    void waitingForReply(bool waiting);

    void loginResult(bool logged_in);

    void currentUserUpdated();

    void formReceived(QString form_type, QString data);

    // Generic data signal, emitted for all data type.
    void dataReceived(QList<TeraData> generic_list);

    // Specific data signals
    void usersReceived(QList<TeraData> user_list);
    void sitesReceived(QList<TeraData> site_list);
    void sessionTypesReceived(QList<TeraData> st_list);
    void testDefsReceived(QList<TeraData> tests_list);
    void projectsReceived(QList<TeraData> projects_list);
    void devicesReceived(QList<TeraData> devices_list);
    void participantsReceived(QList<TeraData> participants_list);
    void groupsReceived(QList<TeraData> groups_list);
    void siteAccessReceived(QList<TeraData> access_list);
    void projectAccessReceived(QList<TeraData> access_list);
    void sessionsReceived(QList<TeraData> sessions_list);
    void deviceSitesReceived(QList<TeraData> device_sites_list);
    void deviceParticipantsReceived(QList<TeraData> device_participants_list);
    void sessionTypesDeviceTypesReceived(QList<TeraData> session_types_device_types_list);
    void sessionTypesProjectsReceived(QList<TeraData> session_types_projects_list);
    void deviceDatasReceived(QList<TeraData> device_data_list);
    void sessionEventsReceived(QList<TeraData> events_list);

    //void queryResultsReceived(QString object, QUrlQuery url_query, QString data);
    //void postResultsReceived(QString path, QString data);
    void queryResultsOK(QString path, QUrlQuery url_query);
    void postResultsOK(QString path);
    void deleteResultsOK(QString path, int id);

    void downloadCompleted(DownloadedFile* file);
    void downloadProgress(DownloadedFile* file);

public slots:

private slots:
    // Websockets
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);
    void onSocketSslErrors(const QList<QSslError> &errors);
    void onSocketTextMessageReceived(const QString &message);
    void onSocketBinaryMessageReceived(const QByteArray &message);

    // Network
    void onNetworkAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);
    void onNetworkEncrypted(QNetworkReply *reply);
    void onNetworkFinished(QNetworkReply *reply);
    void onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);
    void onNetworkSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

    void onTimerConnectTimeout();

};

#endif // COMMANAGER_H
