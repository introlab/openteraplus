#include "MOVEComManager.h"
#include "MOVEWebAPI.h"

MOVEComManager::MOVEComManager(ComManager *comManager, QObject *parent)
    : BaseServiceComManager(comManager, "MOVEService")
{
    connect(this, &BaseComManager::networkError, this, &MOVEComManager::handleNetworkError);
}

MOVEComManager::~MOVEComManager()
{
}

void MOVEComManager::postHandleData(const QList<QJsonObject> &items, const QString &reply_path, const QUrlQuery &reply_query)
{

    if (reply_path.contains(MOVE_USER_PARTICIPANT_PROFILE_PATH))
    {
        qDebug() << "Received items: " << items;
        emit participantProfileReceived(items.first(), reply_query);
    }
}

void MOVEComManager::queryParticipantProfile(const QString &participant_uuid)
{
    QUrlQuery query_args;
    QString path = QString(MOVE_USER_PARTICIPANT_PROFILE_PATH) + "/" + participant_uuid;
    doGet(path, query_args, true);
}

void MOVEComManager::updateParticipantProfile(const QJsonObject &profile_data)
{
    QUrlQuery query_args;
    QString path = MOVE_USER_PARTICIPANT_PROFILE_PATH;

    // Structure profile_data into another JSON object like : {"participant_profile": profile_data}
    QJsonObject data;
    data.insert("participant_profile", profile_data);
    //void doPost(const QString &path, const QString &post_data, const bool &use_token=true) override;
    doPost(path, QJsonDocument(data).toJson(QJsonDocument::Compact), true);
}

void MOVEComManager::handleNetworkError(QNetworkReply::NetworkError error, QString error_str, QNetworkAccessManager::Operation op, int status_code, QString path, QUrlQuery url_query)
{
    // Handle network errors specific to MOVE service
    qDebug() << "MOVEComManager::handleNetworkError - Error:" << error_str << "Status Code:" << status_code;

    if (path.contains(MOVE_USER_PARTICIPANT_PROFILE_PATH) && op == QNetworkAccessManager::GetOperation && status_code == 404)
    {
        qDebug() << "Participant profile not found for path:" << path;
        emit participantProfileNotFound(path, status_code, error_str);
    }

}
