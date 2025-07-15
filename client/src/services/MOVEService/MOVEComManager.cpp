#include "MOVEComManager.h"
#include "MOVEWebAPI.h"

MOVEComManager::MOVEComManager(ComManager *comManager, QObject *parent)
    : BaseServiceComManager(comManager, "MOVEService")
{
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
