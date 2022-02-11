#ifndef ASSETCOMMANAGER_H
#define ASSETCOMMANAGER_H

#include "BaseComManager.h"

#include <QObject>

#include "managers/ComManager.h"

class AssetComManager : public BaseComManager
{
    Q_OBJECT
public:
    explicit AssetComManager(ComManager* comManager, QObject *parent = nullptr);

    void doGet(const QString &path, const QUrlQuery &query_args = QUrlQuery(), const bool &use_token=true) override;
    void doPost(const QString &path, const QString &post_data, const bool &use_token=true) override;
    void doPost(const QUrl &full_url, const QString &post_data, const bool &use_token=true) override;
    void doDelete(const QString &path, const int& id, const bool &use_token=true) override;

private:
    ComManager*     m_comManager;

    void connectSignals();
    bool handleDataReply(const QString& reply_path, const QString &reply_data, const QUrlQuery &reply_query);

protected:
    bool processNetworkReply(QNetworkReply *reply) override;

private slots:
    void handleUserTokenUpdated();

signals:
    void dataReceived(QList<QJsonObject> items, QUrlQuery reply_query);

    void assetsInfosReceived(QList<QJsonObject> infos, QUrlQuery reply_query, QString reply_path);
};

#endif // ASSETCOMMANAGER_H
