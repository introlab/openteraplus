#ifndef DANCECOMMANAGER_H
#define DANCECOMMANAGER_H

#include <QObject>
#include "managers/ComManager.h"
#include "services/BaseServiceComManager.h"

class DanceComManager : public BaseServiceComManager
{
    Q_OBJECT
public:
    explicit DanceComManager(ComManager* comManager, QObject *parent = nullptr);
    ~DanceComManager();

private:
    void postHandleData(const QList<QJsonObject>& items, const QString &reply_path, const QUrlQuery &reply_query) override;

signals:

    void videosReceived(QList<QJsonObject> videos, QUrlQuery reply_query);
    void playlistReceived(QList<QJsonObject> playlist, QUrlQuery reply_query);

};

#endif // DANCECOMMANAGER_H
