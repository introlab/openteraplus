#include "DanceComManager.h"
#include "DanceWebAPI.h"

DanceComManager::DanceComManager(ComManager *comManager, QObject *parent)
    : BaseServiceComManager(comManager, "DanceService")
{

}

DanceComManager::~DanceComManager()
{

}

void DanceComManager::postHandleData(const QList<QJsonObject> &items, const QString &reply_path, const QUrlQuery &reply_query)
{
    if (reply_path.endsWith(DANCE_LIBRARY_PATH) || reply_path.endsWith(DANCE_LIBRARY_VIDEOS_PATH)){
        emit videosReceived(items, reply_query);
    }

    if (reply_path.endsWith(DANCE_PLAYLIST_PATH)){
        emit playlistReceived(items, reply_query);
    }
}
