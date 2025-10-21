#include "ActimetryComManager.h"
#include "ActimetryWebAPI.h"

ActimetryComManager::ActimetryComManager(ComManager *comManager, QObject *parent)
    : BaseServiceComManager(comManager, "ActimetryService")
{
    connect(this, &BaseComManager::networkError, this, &ActimetryComManager::handleNetworkError);
}

ActimetryComManager::~ActimetryComManager()
{
}

void ActimetryComManager::postHandleData(const QList<QJsonObject> &items, const QString &reply_path, const QUrlQuery &reply_query)
{


}



void ActimetryComManager::handleNetworkError(QNetworkReply::NetworkError error, QString error_str, QNetworkAccessManager::Operation op, int status_code, QString path, QUrlQuery url_query)
{
    // Handle network errors specific to Actimetry service
    qDebug() << "ActimetryComManager::handleNetworkError - Error:" << error_str << "Status Code:" << status_code;



}
