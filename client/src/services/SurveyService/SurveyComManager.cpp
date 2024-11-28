#include "SurveyComManager.h"
#include "SurveyServiceWebAPI.h"

SurveyComManager::SurveyComManager(ComManager *comManager, QObject *parent)
    : BaseServiceComManager{comManager, "SurveyJSService", parent}
{
    // Connect signals
    connectSignals();
}

void SurveyComManager::connectSignals()
{
    connect(this, &BaseServiceComManager::dataReceived, this, &SurveyComManager::onDataReceived);
}

void SurveyComManager::onDataReceived(QList<QJsonObject> items, QString reply_path, QUrlQuery reply_query)
{
    Q_UNUSED(reply_query)
    // Check to emit correct signals for specific data types
    if (reply_path.endsWith(SURVEY_ACTIVE_PATH)){
        if (!items.empty()){
            emit activeSurveyReceived(items.first());
        }
    }

}
