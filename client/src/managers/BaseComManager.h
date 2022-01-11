#ifndef BASECOMMANAGER_H
#define BASECOMMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QAuthenticator>

#include "Logger.h"
#include "Utils.h"

class BaseComManager : public QObject
{
    Q_OBJECT
public:
    explicit BaseComManager(QUrl serverUrl, QObject *parent = nullptr);
    ~BaseComManager();

    virtual void doGet(const QString &path, const QUrlQuery &query_args = QUrlQuery(), const bool &use_token=false);
    virtual void doPost(const QString &path, const QString &post_data, const bool &use_token=false);
    virtual void doDelete(const QString &path, const int& id, const bool &use_token=false);

    void setCredentials(const QString &username, const QString &password);
    void setCredentials(const QString &token);
    void clearCredentials();
    QString getCurrentToken();

    QUrl getServerUrl() const;

private:
    QString                 m_username;
    QString                 m_password;
    QString                 m_token;

    bool                    m_settedCredentials;

protected:
    QUrl                    m_serverUrl;
    QNetworkAccessManager*  m_netManager;
    QNetworkCookieJar       m_cookieJar;

    bool                    m_loggingInProgress; // Indicates a login procedure is going on

    virtual bool processNetworkReply(QNetworkReply* reply) = 0;
    void setRequestLanguage(QNetworkRequest &request);
    void setRequestCredentials(QNetworkRequest &request, const bool &use_token);
    void setRequestVersions(QNetworkRequest &request);


protected slots:
    virtual void onNetworkFinished(QNetworkReply *reply);
    virtual void onNetworkSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
    void onNetworkEncrypted(QNetworkReply *reply);
    void onNetworkAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);

signals:
    void waitingForReply(bool waiting);
    void networkError(QNetworkReply::NetworkError, QString, QNetworkAccessManager::Operation op, int status_code);

    void networkAuthFailed();

    void queryResultsOK(QString path, QUrlQuery url_query);
    void postResultsOK(QString path);
    void deleteResultsOK(QString path, int id);
    void posting(QString path, QString data);
    void querying(QString path);
    void deleting(QString path);

};

#endif // BASECOMMANAGER_H
