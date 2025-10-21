#ifndef ACTIMETRYCOMMANAGER_H
#define ACTIMETRYCOMMANAGER_H

#include <QObject>
#include "managers/ComManager.h"
#include "services/BaseServiceComManager.h"

class ActimetryComManager : public BaseServiceComManager
{
    Q_OBJECT
public:
    explicit ActimetryComManager(ComManager *comManager, QObject *parent = nullptr);
    ~ActimetryComManager();


private:
    void postHandleData(const QList<QJsonObject> &items, const QString &reply_path, const QUrlQuery &reply_query) override;
    void handleNetworkError(QNetworkReply::NetworkError error, QString error_str, QNetworkAccessManager::Operation op, int status_code, QString path, QUrlQuery url_query);

signals:


};

#endif // ACTIMETRYCOMMANAGER_H
