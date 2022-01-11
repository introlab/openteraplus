#ifndef DANCECOMMANAGER_H
#define DANCECOMMANAGER_H

#include <QObject>
#include "managers/ComManager.h"
#include "managers/BaseComManager.h"
#include "Utils.h"

class DanceComManager : public BaseComManager
{
    Q_OBJECT
public:
    explicit DanceComManager(ComManager* comManager, QObject *parent = nullptr);
    ~DanceComManager();

    bool processNetworkReply(QNetworkReply* reply);


private:
    ComManager*             m_comManager;



};

#endif // DANCECOMMANAGER_H
