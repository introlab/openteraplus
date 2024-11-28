#include "EmailComManager.h"
#include "EmailServiceWebAPI.h"

EmailComManager::EmailComManager(ComManager *comManager, QObject *parent)
    : BaseServiceComManager{comManager, "EmailService", parent}
{

}

EmailComManager::~EmailComManager()
{

}

bool EmailComManager::preHandleData(const QString &reply_path, const QString &data, const QUrlQuery &reply_query)
{
    Q_UNUSED(data)
    Q_UNUSED(reply_query)
    if (reply_path.endsWith(EMAIL_SEND_PATH)){
        emit emailSendSuccess();
        return true;
    }

    return false;
}

void EmailComManager::postHandleData(const QList<QJsonObject> &items, const QString &reply_path, const QUrlQuery &reply_query)
{
    Q_UNUSED(items)
    Q_UNUSED(reply_query)
    if (reply_path.endsWith(EMAIL_TEMPLATE_PATH)){
        if (!items.empty()){
            emit emailTemplateReceived(items.first());
        }
    }
}

