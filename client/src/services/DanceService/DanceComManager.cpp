#include "DanceComManager.h"

DanceComManager::DanceComManager(ComManager *comManager, QObject *parent)
    : BaseComManager(comManager->getServerUrl()),
      m_comManager(comManager)
{

    // Set URL for service
    m_serverUrl.setHost(m_serverUrl.host() + "/dance");


}

DanceComManager::~DanceComManager()
{
}

bool DanceComManager::processNetworkReply(QNetworkReply *reply)
{

    return false;
}

