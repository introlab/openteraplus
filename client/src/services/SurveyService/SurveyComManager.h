#ifndef SURVEYCOMMANAGER_H
#define SURVEYCOMMANAGER_H

#include <QObject>
#include "managers/BaseComManager.h"
#include "managers/ComManager.h"

class SurveyComManager : public BaseComManager
{
    Q_OBJECT
public:
    explicit SurveyComManager(ComManager* comManager, QObject *parent = nullptr);

private:
    ComManager*             m_comManager;

    void connectSignals();
    bool processNetworkReply(QNetworkReply* reply) override;
    bool handleDataReply(const QString& reply_path, const QString &reply_data, const QUrlQuery &reply_query);

private slots:
    void handleUserTokenUpdated();

signals:
    void dataReceived(QList<QJsonObject> items, QUrlQuery reply_query);
    void activeSurveyReceived(QJsonObject survey);

};

#endif // SURVEYCOMMANAGER_H
