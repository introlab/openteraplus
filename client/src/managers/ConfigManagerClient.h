#ifndef CONFIGMANAGERCLIENT_H
#define CONFIGMANAGERCLIENT_H

#include <QObject>

#include "ConfigManager.h"

class ConfigManagerClient : public ConfigManager
{
public:
    explicit ConfigManagerClient(QObject *parent = nullptr);
    ConfigManagerClient(QString filename, QObject* parent = nullptr);

    QStringList     getServerNames();
    QUrl            getServerUrl(const QString &server_name);
    QUrl            getServerLoginUrl(const QString &server_name);

    bool            getLogToFile();

    bool            showServers();

    bool            isKitMode();
protected:
    //QHash<QString, QVariant>    m_servers;
};

#endif // CONFIGMANAGERCLIENT_H
