#ifndef DASHCOMMANAGER_H
#define DASHCOMMANAGER_H

#include <QObject>
#include "managers/ComManager.h"
#include "managers/BaseComManager.h"
#include "Utils.h"
#include "DashboardsWebAPI.h"

class DashboardsComManager : public BaseComManager
{
    Q_OBJECT
public:
    explicit DashboardsComManager(ComManager* comManager, QObject *parent = nullptr);
    ~DashboardsComManager();

    bool processNetworkReply(QNetworkReply* reply);

    void doGet(const QString &path, const QUrlQuery &query_args = QUrlQuery());
    void doPost(const QString &path, const QString &post_data);
    void doDelete(const QString &path, const int& id);


private:
    ComManager*             m_comManager;

    void connectSignals();

    bool handleDataReply(const QString& reply_path, const QString &reply_data, const QUrlQuery &reply_query);

private slots:
    void handleUserTokenUpdated();

signals:
    void dataReceived(QList<QJsonObject> items, QUrlQuery reply_query);
    void dashboardsReceived(QList<QJsonObject> videos, QUrlQuery reply_query);

};

#endif // DASHCOMMANAGER_H
