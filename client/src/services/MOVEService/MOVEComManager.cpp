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
    if (reply_path.endsWith(MOVE_LIBRARY_PATH) || reply_path.endsWith(MOVE_LIBRARY_VIDEOS_PATH)){
        emit videosReceived(items, reply_query);
    }

    if (reply_path.endsWith(MOVE_PLAYLIST_PATH)){
        emit playlistReceived(items, reply_query);
    }
}
