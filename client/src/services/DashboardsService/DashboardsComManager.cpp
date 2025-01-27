#include "DashboardsComManager.h"
#include "DashboardsWebAPI.h"

DashboardsComManager::DashboardsComManager(ComManager *comManager, QObject *parent)
    : BaseServiceComManager(comManager, "DashboardsService")
{

}

DashboardsComManager::~DashboardsComManager()
{

}

void DashboardsComManager::postHandleData(const QList<QJsonObject> &items, const QString &reply_path, const QUrlQuery &reply_query)
{
    Q_UNUSED(reply_query)

    if (reply_path.endsWith(DASHBOARDS_USER_PATH)){
        emit dashboardsReceived(items, reply_query);
    }
}
