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

protected:
    bool processNetworkReply(QNetworkReply *reply) override;

private slots:
    void handleUserTokenUpdated();
};

#endif // ASSETCOMMANAGER_H
