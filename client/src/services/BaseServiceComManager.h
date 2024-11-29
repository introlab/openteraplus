#ifndef BASESERVICECOMMANAGER_H
#define BASESERVICECOMMANAGER_H

#include <QObject>
#include "managers/BaseComManager.h"
#include "managers/ComManager.h"

class BaseServiceComManager : public BaseComManager
{
    Q_OBJECT
public:
    explicit BaseServiceComManager(ComManager* comManager, QString service_key, QObject *parent = nullptr);

    void doGet(const QString &path, const QUrlQuery &query_args = QUrlQuery(), const bool &use_token=true) override;
    void doPost(const QString &path, const QString &post_data, const bool &use_token=true) override;
    void doPost(const QUrl &full_url, const QString &post_data, const bool &use_token=true) override;
    void doPostWithParams(const QString &path, const QString &post_data, const QUrlQuery &query_args, const bool &use_token=true) override;
    void doDelete(const QString &path, const int& id, const bool &use_token=true) override;

    bool isReady();

    QString getServiceEndpoint(const QString &path);

private:
    void connectSignals();

protected:
    ComManager* m_comManager;
    TeraData    m_service;
    QString     m_serviceKey;

    bool processNetworkReply(QNetworkReply* reply) override;
    virtual bool handleDataReply(const QString& reply_path, const QString &reply_data, const QUrlQuery &reply_query);
    virtual bool preHandleData(const QString& reply_path, const QString& data, const QUrlQuery &reply_query);
    virtual void postHandleData(const QList<QJsonObject>& items, const QString &reply_path, const QUrlQuery &reply_query) = 0;

private slots:
    void handleUserTokenUpdated();
    void processServicesReply(QList<TeraData> services, QUrlQuery reply_query);

signals:
    void dataReceived(QList<QJsonObject> items, QString endpoint, QUrlQuery reply_query);
    void readyChanged(bool ready);
    void userTokenUpdated();

};

#endif // BASESERVICECOMMANAGER_H
