#ifndef MOVECOMMANAGER_H
#define MOVECOMMANAGER_H

#include <QObject>
#include "managers/ComManager.h"
#include "services/BaseServiceComManager.h"

class MOVEComManager : public BaseServiceComManager
{
    Q_OBJECT
public:
    explicit MOVEComManager(ComManager* comManager, QObject *parent = nullptr);
    ~MOVEComManager();

private:
    void postHandleData(const QList<QJsonObject>& items, const QString &reply_path, const QUrlQuery &reply_query) override;

signals:

    void videosReceived(QList<QJsonObject> videos, QUrlQuery reply_query);
    void playlistReceived(QList<QJsonObject> playlist, QUrlQuery reply_query);

};

#endif // MOVECOMMANAGER_H
