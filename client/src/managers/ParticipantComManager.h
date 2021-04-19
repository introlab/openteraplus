#ifndef PARTICIPANT_COMMANAGER_H
#define PARTICIPANT_COMMANAGER_H

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

#include <QStringList>

#include "Logger.h"
#include "ParticipantWebAPI.h"

#include "TeraData.h"
#include "TeraPreferences.h"
#include "TeraSessionStatus.h"
#include "TeraSessionCategory.h"
#include "DownloadedFile.h"
#include "WebSocketManager.h"


class ParticipantComManager : public QObject
{
    Q_OBJECT
public:
    explicit ParticipantComManager(QUrl serverUrl, bool connectWebsocket = true, QObject *parent = nullptr);
    ~ParticipantComManager();

    void connectToServer(QString token);
    void disconnectFromServer();

    bool processNetworkReply(QNetworkReply* reply);
    void doQuery(const QString &path, const QUrlQuery &query_args = QUrlQuery());
    void doPost(const QString &path, const QString &post_data);
    void doDelete(const QString &path, const int& id);
    void doUpdateCurrentParticipant();

    TeraData &getCurrentParticipant();
    typedef void (ParticipantComManager::* signal_ptr)(QList<TeraData>, QUrlQuery);



    QUrl getServerUrl() const;

    WebSocketManager* getWebSocketManager();

    static signal_ptr getSignalFunctionForDataType(const TeraDataTypes& data_type);

protected:
    bool handleLoginReply(const QString& reply_data);
    bool handleDataReply(const QString& reply_path, const QString& reply_data, const QUrlQuery& reply_query);

    void updateCurrentParticipant(const TeraData& participant_data);

    void clearCurrentParticipant();

    void setCredentials(const QString &token);

    QString filterReplyString(const QString& data_str);

    QUrl                    m_serverUrl;
    QNetworkAccessManager*  m_netManager;
    QNetworkCookieJar       m_cookieJar;
    WebSocketManager*       m_webSocketMan;

    bool                    m_enableWebsocket;

    QString                 m_token;

    TeraData                m_currentParticipant;

signals:
    void serverDisconnected();
    void networkError(QNetworkReply::NetworkError, QString, QNetworkAccessManager::Operation op, int status_code);
    void socketError(QAbstractSocket::SocketError, QString);
    void waitingForReply(bool waiting);

    void loginResult(bool logged_in);

    void currentParticipantUpdated();

    // Generic data signal, emitted for all data type.
    void dataReceived(TeraDataTypes data_type, QList<TeraData> generic_list, QUrlQuery reply_query);

    // Specific data signals
    void participantsReceived(QList<TeraData> participants_list, QUrlQuery reply_query);

    void queryResultsOK(QString path, QUrlQuery url_query);
    void postResultsOK(QString path);
    void deleteResultsOK(QString path, int id);
    void posting(QString path, QString data);
    void querying(QString path);
    void deleting(QString path);

public slots:

protected:
    void setRequestLanguage(QNetworkRequest &request);
    void setRequestCredentials(QNetworkRequest &request);
    void setRequestVersions(QNetworkRequest &request);

private slots:
    // Network
    void onNetworkEncrypted(QNetworkReply *reply);
    void onNetworkFinished(QNetworkReply *reply);
    void onNetworkSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

    void onWebSocketLoginResult(bool logged_in);

};



#endif // PARTICIPANT_COMMANAGER_H
