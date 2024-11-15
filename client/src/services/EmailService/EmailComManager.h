#ifndef EMAILCOMMANAGER_H
#define EMAILCOMMANAGER_H

#include <QObject>
#include "managers/BaseComManager.h"
#include "managers/ComManager.h"

class EmailComManager : public BaseComManager
{
    Q_OBJECT
public:
    explicit EmailComManager(ComManager* comManager, QObject *parent = nullptr);
    ~EmailComManager();

private:
    ComManager*             m_comManager;

    void connectSignals();
    bool processNetworkReply(QNetworkReply* reply) override;
    bool handleDataReply(const QString& reply_path, const QString &reply_data, const QUrlQuery &reply_query);

private slots:
    void handleUserTokenUpdated();

signals:
    void dataReceived(QList<QJsonObject> items, QUrlQuery reply_query);
    void emailTemplateReceived(QJsonObject email_template);

};

#endif // EMAILCOMMANAGER_H
