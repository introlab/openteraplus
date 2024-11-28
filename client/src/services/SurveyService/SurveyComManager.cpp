#include "SurveyComManager.h"
#include "SurveyServiceWebAPI.h"

SurveyComManager::SurveyComManager(ComManager *comManager, QObject *parent)
    : BaseServiceComManager{comManager, "SurveyJSService", parent}
{
}


void SurveyComManager::postHandleData(const QList<QJsonObject>& items, const QString &reply_path, const QUrlQuery &reply_query)
{
    Q_UNUSED(reply_query)
    // Check to emit correct signals for specific data types
    if (reply_path.endsWith(SURVEY_ACTIVE_PATH)){
        if (!items.empty()){
            emit activeSurveyReceived(items.first());
        }
    }

}
