#include "AssetComManager.h"

AssetComManager::AssetComManager(ComManager *comManager, QObject *parent)
    : BaseComManager(QUrl(), parent),
      m_comManager(comManager)
{
    // Set initial user token
    setCredentials(m_comManager->getCurrentToken());

    // Connect signals
    connectSignals();
}

void AssetComManager::doGet(const QString &path, const QUrlQuery &query_args, const bool &use_token)
{
    Q_UNUSED(use_token)
    BaseComManager::doGet(path, query_args, true); // Always use token!
}

void AssetComManager::doPost(const QString &path, const QString &post_data, const bool &use_token)
{
    Q_UNUSED(use_token)
    BaseComManager::doPost(path, post_data, true); // Always use token!
}

void AssetComManager::doPost(const QUrl &full_url, const QString &post_data, const bool &use_token)
{
    Q_UNUSED(use_token)
    BaseComManager::doPost(full_url, post_data, true); // Always use token!
}

void AssetComManager::doDelete(const QString &path, const int &id, const bool &use_token)
{
    Q_UNUSED(use_token)
    BaseComManager::doDelete(path, id, true); // Always use token!
}

void AssetComManager::connectSignals()
{
    connect(m_comManager, &ComManager::userTokenUpdated, this, &AssetComManager::handleUserTokenUpdated);
}

bool AssetComManager::processNetworkReply(QNetworkReply *reply)
{
    return false;
}

void AssetComManager::handleUserTokenUpdated()
{
    // Update token
    setCredentials(m_comManager->getCurrentToken());
}
