#ifndef DASHCOMMANAGER_H
#define DASHCOMMANAGER_H

#include <QObject>
#include "managers/ComManager.h"
#include "services/BaseServiceComManager.h"

class DashboardsComManager : public BaseServiceComManager
{
    Q_OBJECT
public:
    explicit DashboardsComManager(ComManager* comManager, QObject *parent = nullptr);
    ~DashboardsComManager();

private:
    void postHandleData(const QList<QJsonObject>& items, const QString &reply_path, const QUrlQuery &reply_query) override;

signals:

    void dashboardsReceived(QList<QJsonObject> dashboards, QUrlQuery reply_query);

};

#endif // DASHCOMMANAGER_H
