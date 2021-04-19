#include "ConfigManagerClient.h"
#include <QDebug>

ConfigManagerClient::ConfigManagerClient(QObject *parent) : ConfigManager(parent)
{

}

ConfigManagerClient::ConfigManagerClient(QString filename, QObject *parent) : ConfigManager (filename, parent)
{

}

QStringList ConfigManagerClient::getServerNames()
{
    QStringList server_names;

    if (!m_config.isNull()){
        QMap<QString, QVariant> servers = m_config["Servers"].toObject().toVariantMap();
        foreach (QString name, servers.keys()){
            server_names.append(name);
        }
    }

    return server_names;
}

QUrl ConfigManagerClient::getServerUrl(const QString &server_name)
{
    QUrl server_url;
    if (!m_config.isNull()){
        QHash<QString, QVariant> servers = m_config["Servers"].toObject().toVariantHash();
        QVariantMap server_info = servers[server_name].toMap();
        server_url.setHost(server_info["url"].toString());
        server_url.setPort(server_info["port"].toInt());
    }
    return server_url;
}

bool ConfigManagerClient::getLogToFile()
{
    bool rval = false;
    if (!m_config.isNull()){
        QHash<QString, QVariant> settings = m_config["Settings"].toObject().toVariantHash();
        rval = settings["logToFile"].toBool();
    }
    return rval;
}

bool ConfigManagerClient::showServers()
{
    bool rval = false;
    if (!m_config.isNull()){
        QHash<QString, QVariant> settings = m_config["Settings"].toObject().toVariantHash();
        rval = settings["showServers"].toBool();
    }
    return rval;
}

bool ConfigManagerClient::isKitMode()
{
    bool rval = false;
    if (!m_config.isNull()){
        QHash<QString, QVariant> settings = m_config["Settings"].toObject().toVariantHash();
        rval = settings["kitMode"].toBool();
    }
    return rval;
}
