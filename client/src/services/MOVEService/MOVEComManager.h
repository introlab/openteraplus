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
    void updateParticipantProfile(const QJsonObject &profile_data);

private:
    void postHandleData(const QList<QJsonObject> &items, const QString &reply_path, const QUrlQuery &reply_query) override;
    void handleNetworkError(QNetworkReply::NetworkError error, QString error_str, QNetworkAccessManager::Operation op, int status_code, QString path, QUrlQuery url_query);

signals:

    void participantProfileReceived(QJsonObject user_profile, QUrlQuery reply_query);
    void participantProfileNotFound(const QString &path, int status_code, const QString &error_str);
};

#endif // MOVECOMMANAGER_H
