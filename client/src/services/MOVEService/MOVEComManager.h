#ifndef MOVECOMMANAGER_H
#define MOVECOMMANAGER_H

#include <QObject>
#include "managers/ComManager.h"
#include "services/BaseServiceComManager.h"

class MOVEComManager : public BaseServiceComManager
{
    Q_OBJECT
public:
    explicit MOVEComManager(ComManager *comManager, QObject *parent = nullptr);
    ~MOVEComManager();
    void queryParticipantProfile(const QString &participant_uuid);

private:
    void postHandleData(const QList<QJsonObject> &items, const QString &reply_path, const QUrlQuery &reply_query) override;

signals:

    void participantProfileReceived(QJsonObject user_profile, QUrlQuery reply_query);
};

#endif // MOVECOMMANAGER_H
