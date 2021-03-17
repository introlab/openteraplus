#include "ConfigManager.h"

#include <QDebug>

ConfigManager::ConfigManager(QObject *parent) : QObject(parent)
{

}

ConfigManager::ConfigManager(QString filename, QObject *parent) : QObject(parent)
{
    m_filename = filename;
}

void ConfigManager::setFilename(QString filename)
{
    m_filename = filename;
}

QString ConfigManager::getFilename()
{
    return m_filename;
}

bool ConfigManager::loadConfig()
{
    m_lastJsonError.error = QJsonParseError::NoError;

    QFile configFile(m_filename);
    // Open the config file
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    // Read all the data in the config file - size is minimal, so loading in memory is fine.
    QByteArray configData = configFile.readAll();
    configFile.close();

    // Convert data into JSON array, if possible.
    m_config = QJsonDocument::fromJson(configData, &m_lastJsonError);
    if (m_lastJsonError.error!= QJsonParseError::NoError)
        return false;

    return true;

}

bool ConfigManager::saveConfig()
{
    if (m_filename.isEmpty())
        return false;

    QFile file(m_filename);
    if (!file.open(QIODevice::WriteOnly)){
        return false;
    }

    //qDebug() << m_config.toJson(QJsonDocument::Indented);
    file.write(m_config.toJson(QJsonDocument::Indented));

    file.close();

    return true;
}

QJsonParseError ConfigManager::getLastParseError()
{
    return m_lastJsonError;
}

bool ConfigManager::hasParseError()
{
    return (m_lastJsonError.error != QJsonParseError::NoError);
}

void ConfigManager::initConfig()
{
    m_config = QJsonDocument();
    m_config.setObject(QJsonObject());
}

void ConfigManager::initConfig(QStringList sections)
{
    m_config = QJsonDocument();
    QJsonObject main_obj;

    foreach (QString section, sections){
        main_obj.insert(section, QJsonValue(QJsonObject()));

    }
    m_config.setObject(main_obj);

}



