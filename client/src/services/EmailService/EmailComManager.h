#ifndef EMAILCOMMANAGER_H
#define EMAILCOMMANAGER_H

#include <QObject>
#include "services/BaseServiceComManager.h"
#include "managers/ComManager.h"

class EmailComManager : public BaseServiceComManager
{
    Q_OBJECT
public:
    explicit EmailComManager(ComManager* comManager, QObject *parent = nullptr);
    ~EmailComManager();

private:
    bool preHandleData(const QString& reply_path, const QString& data, const QUrlQuery &reply_query) override;
    void postHandleData(const QList<QJsonObject>& items, const QString &reply_path, const QUrlQuery &reply_query) override;

signals:
    void emailTemplateReceived(QJsonObject email_template);
    void emailSendSuccess();

};

#endif // EMAILCOMMANAGER_H
