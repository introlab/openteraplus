#ifndef SURVEYCOMMANAGER_H
#define SURVEYCOMMANAGER_H

#include <QObject>
#include "services/BaseServiceComManager.h"
#include "managers/ComManager.h"

class SurveyComManager : public BaseServiceComManager
{
    Q_OBJECT
public:
    explicit SurveyComManager(ComManager* comManager, QObject *parent = nullptr);

private:
    void postHandleData(const QList<QJsonObject>& items, const QString &reply_path, const QUrlQuery &reply_query) override;

signals:
    void activeSurveyReceived(QJsonObject survey);

};

#endif // SURVEYCOMMANAGER_H
