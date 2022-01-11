#ifndef DANCECOMMANAGER_H
#define DANCECOMMANAGER_H

#include <QObject>
#include "managers/ComManager.h"
#include "managers/BaseComManager.h"
#include "Utils.h"
#include "DanceWebAPI.h"

class DanceComManager : public BaseComManager
{
    Q_OBJECT
public:
    explicit DanceComManager(ComManager* comManager, QObject *parent = nullptr);
    ~DanceComManager();

    bool processNetworkReply(QNetworkReply* reply);

    void doGet(const QString &path, const QUrlQuery &query_args = QUrlQuery(), const bool &use_token=false);
    void doPost(const QString &path, const QString &post_data, const bool &use_token=false);
    void doDelete(const QString &path, const int& id, const bool &use_token=false);


private:
    ComManager*             m_comManager;

    void connectSignals();

    bool handleDataReply(const QString& reply_path, const QString &reply_data, const QUrlQuery &reply_query);

private slots:
    void handleUserTokenUpdated();

signals:
    void dataReceived(QList<QJsonObject> items, QUrlQuery reply_query);

    void videosReceived(QList<QJsonObject> videos, QUrlQuery reply_query);

};

#endif // DANCECOMMANAGER_H
