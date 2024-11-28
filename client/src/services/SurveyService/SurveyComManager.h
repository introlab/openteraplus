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

    void connectSignals();

private slots:
    void onDataReceived(QList<QJsonObject> items, QString reply_path, QUrlQuery reply_query);

signals:
    void activeSurveyReceived(QJsonObject survey);

};

#endif // SURVEYCOMMANAGER_H
