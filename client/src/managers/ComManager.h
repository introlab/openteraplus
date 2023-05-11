#ifndef COMMANAGER_H
#define COMMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QAuthenticator>
#include <QNetworkProxy>

#ifndef OPENTERA_WEBASSEMBLY
#include <QSslPreSharedKeyAuthenticator>
#endif

#include <QNetworkCookieJar>
#include <QNetworkCookie>

#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonValue>

#include <QUrl>
#include <QUrlQuery>
#include <QTimer>

#include <QStringList>

#include "Logger.h"
#include "WebAPI.h"

#include "TeraData.h"
#include "TeraPreferences.h"
#include "TeraSessionStatus.h"
#include "TeraSessionCategory.h"
#include "data/DownloadingFile.h"
#include "WebSocketManager.h"

#include "BaseComManager.h"


class ComManager : public BaseComManager
{
    Q_OBJECT
public:
    explicit ComManager(QUrl serverUrl, bool connectWebsocket = true, QObject *parent = nullptr);
    ~ComManager();

    void connectToServer(QString username, QString password);
    void disconnectFromServer();

    bool processNetworkReply(QNetworkReply* reply);

    void doUpdateCurrentUser();

    void startSession(const TeraData& session_type, const int &id_session, const QStringList &participants_list, const QStringList &users_list, const QStringList &devices_list, const QJsonDocument &session_config);
    void joinSession(const TeraData& session_type, const int &id_session);
    void stopSession(const TeraData& session, const int &id_service = 0);
    void leaveSession(const int &id_session, bool signal_server = true);
    void sendJoinSessionReply(const QString &session_uuid, const opentera::protobuf::JoinSessionReplyEvent::ReplyType reply_type, const QString &join_msg = QString());

    TeraData &getCurrentUser();
    TeraPreferences &getCurrentPreferences();
    QString getCurrentUserSiteRole(const int &site_id);
    QString getCurrentUserProjectRole(const int &project_id);

    bool isCurrentUserProjectAdmin(const int& project_id);
    bool isCurrentUserSiteAdmin(const int& site_id);
    bool isCurrentUserSuperAdmin();
    typedef void (ComManager::* signal_ptr)(QList<TeraData>, QUrlQuery);

    WebSocketManager* getWebSocketManager();

    static signal_ptr getSignalFunctionForDataType(const TeraDataTypes& data_type);

    QJsonDocument   getCurrentSessionConfig();
    TeraData*       getCurrentSessionType();

protected:
    bool handleLoginReply(const QString& reply_data);
    bool handleLoginSequence(const QString& reply_path, const QString& reply_data, const QUrlQuery& reply_query);
    bool handleDataReply(const QString& reply_path, const QString& reply_data, const QUrlQuery& reply_query);
    bool handleSessionManagerReply(const QString& reply_data, const QUrlQuery& reply_query);
    bool handleFormReply(const QUrlQuery& reply_query, const QString& reply_data);
    bool handleVersionsReply(const QJsonDocument &version_data);
    bool handleTokenRefreshReply(const QJsonDocument &refresh_data);

    void updateCurrentUser(const TeraData& user_data);
    void updateCurrentPrefs(const TeraData& user_prefs);

    void clearCurrentUser();
    void refreshUserToken();

    WebSocketManager*       m_webSocketMan;
    QTimer                  m_tokenRefreshTimer;

    bool                    m_enableWebsocket;

    TeraData                m_currentUser;
    TeraPreferences         m_currentPreferences;
    TeraData*               m_currentSessionType;
    QJsonDocument           m_currentSessionConfig;

signals:
    void serverDisconnected();
    void socketError(QAbstractSocket::SocketError, QString);

    void loginResult(bool logged_in, QString login_msg);

    void currentUserUpdated();
    void userTokenUpdated();
    void preferencesUpdated();

    void formReceived(QString form_type, QString data);

    // Generic data signal, emitted for all data type.
    void dataReceived(TeraDataTypes data_type, QList<TeraData> generic_list, QUrlQuery reply_query);

    // Specific data signals
    void usersReceived(QList<TeraData> user_list, QUrlQuery reply_query);
    void sitesReceived(QList<TeraData> site_list, QUrlQuery reply_query);
    void sessionTypesReceived(QList<TeraData> st_list, QUrlQuery reply_query);
    void testTypesReceived(QList<TeraData> tests_list, QUrlQuery reply_query);
    void testTypesProjectsReceived(QList<TeraData> ttp_list, QUrlQuery reply_query);
    void testTypesSitesReceived(QList<TeraData> tts_list, QUrlQuery reply_query);
    void testsReceived(QList<TeraData> tests_list, QUrlQuery reply_query);
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
    void sessionTypesProjectsReceived(QList<TeraData> session_types_projects_list, QUrlQuery reply_query);
    void sessionTypesSitesReceived(QList<TeraData> session_types_sites_list, QUrlQuery reply_query);
    void sessionEventsReceived(QList<TeraData> events_list, QUrlQuery reply_query);
    void deviceTypesReceived(QList<TeraData> device_types_list, QUrlQuery reply_query);
    void deviceSubtypesReceived(QList<TeraData> device_subtypes_list, QUrlQuery reply_query);
    void userGroupsReceived(QList<TeraData> user_groups_list, QUrlQuery reply_query);
    void userUserGroupsReceived(QList<TeraData> user_users_groups_list, QUrlQuery reply_query);
    void userPreferencesReceived(QList<TeraData> user_prefs_list, QUrlQuery reply_query);
    void servicesReceived(QList<TeraData> services_list, QUrlQuery reply_query);
    void servicesProjectsReceived(QList<TeraData> projects_list, QUrlQuery reply_query);
    void servicesSitesReceived(QList<TeraData> sites_list, QUrlQuery reply_query);
    void servicesAccessReceived(QList<TeraData> access_list, QUrlQuery reply_query);
    void servicesConfigReceived(QList<TeraData> config_list, QUrlQuery reply_query);
    void servicesRolesReceived(QList<TeraData> roles_list, QUrlQuery reply_query);
    void statsReceived(TeraData stats, QUrlQuery reply_query);
    void onlineUsersReceived(QList<TeraData> users_list, QUrlQuery reply_query);
    void onlineParticipantsReceived(QList<TeraData> participants_list, QUrlQuery reply_query);
    void onlineDevicesReceived(QList<TeraData> devices_list, QUrlQuery reply_query);
    void assetsReceived(QList<TeraData> assets_list, QUrlQuery reply_query);
    void logsLoginReceived(QList<TeraData> logins_list, QUrlQuery reply_query);
    void logsLogsReceived(QList<TeraData> logs_list, QUrlQuery reply_query);

    // Generic session
    void sessionStarted(TeraData session_type, int id_session);
    void sessionStartRequested(TeraData session_type);
    void sessionStopped(int id_session);
    void sessionStopRequested(TeraData session_type);
    void sessionError(QString error);

    // Version management
    void newVersionAvailable(QString version, QString download_url);   

private slots:
    // Network
    void onNetworkAuthenticationFailed();

    void onWebSocketLoginResult(bool logged_in);

};



#endif // COMMANAGER_H
