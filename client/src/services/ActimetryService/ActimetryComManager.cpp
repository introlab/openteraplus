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

void ActimetryComManager::queryAvailableAlgorithms()
{
    QUrlQuery query_args;
    query_args.addQueryItem("list", "true");
    QString path = QString(ACTIMETRY_USER_ALGORITHMS_PATH);
    doGet(path, query_args, true);
}

void ActimetryComManager::queryAlgorithm(const QString &algorithmKey)
{
    QUrlQuery query_args;
    query_args.addQueryItem("key", algorithmKey);
    QString path = QString(ACTIMETRY_USER_ALGORITHMS_PATH);
    doGet(path, query_args, true);
}

void ActimetryComManager::queryProcessing(const QString &algorithmKey, const QString &participant_uuid, const QString &parameters)
{
    QString path = QString(ACTIMETRY_USER_PROCESSING_PATH);


    QJsonObject post_data;
    //TODO Fix and remove toLower
    post_data.insert("key", algorithmKey.toLower());
    post_data.insert("participant_uuid", participant_uuid);
    post_data.insert("parameters", parameters);


    QJsonObject worker_data;
    worker_data.insert("worker", post_data);

    doPost(path, QJsonDocument(worker_data).toJson(QJsonDocument::JsonFormat::Compact), true);
}

void ActimetryComManager::postHandleData(const QList<QJsonObject> &items, const QString &reply_path, const QUrlQuery &reply_query)
{

    if (reply_path.contains(ACTIMETRY_USER_ALGORITHMS_PATH) && items.count() > 0)
    {
        if (reply_query.hasQueryItem("list"))
        {
            qDebug() << "Received items (list algorithms): " << items;
            emit availableAlgorithmsReceived(items);
        }
        else if (reply_query.hasQueryItem("key"))
        {
            qDebug() << "Received items (algorithm): " << items;
            emit algorithmInfoReceived(items.first());
        }
    }
    else if (reply_path.contains(ACTIMETRY_USER_PROCESSING_PATH))
    {
        // Handle processing results if needed
        qDebug() << "Received items (processing): " << items;
    }

}



void ActimetryComManager::handleNetworkError(QNetworkReply::NetworkError error, QString error_str, QNetworkAccessManager::Operation op, int status_code, QString path, QUrlQuery url_query)
{
    // Handle network errors specific to Actimetry service
    qDebug() << "ActimetryComManager::handleNetworkError - Error:" << error_str << "Status Code:" << status_code;



}
