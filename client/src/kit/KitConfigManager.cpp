#include "KitConfigManager.h"
#include <QDebug>

KitConfigManager::KitConfigManager(QObject *parent) : ConfigManager(parent)
{

}

KitConfigManager::KitConfigManager(QString filename, QObject *parent) : ConfigManager(filename, parent)
{

}

void KitConfigManager::initConfig()
{
    ConfigManager::initConfig(QStringList() << "Participant" << "KitConfig" << "TechSupport" << "OtherSoftware");
}

QString KitConfigManager::getParticipantToken()
{
    QString rval = "";
    if (!m_config.isNull()){
        QVariantHash settings = m_config["Participant"].toObject().toVariantHash();
        rval = settings["participant_token"].toString();
    }
    return rval;
}

void KitConfigManager::setParticipantToken(QString token)
{
    if (!m_config.isNull()){
        QJsonObject config = m_config.object();
        QJsonObject participant = config["Participant"].toObject();
        participant.insert("participant_token", token);
        config["Participant"] = participant;
        m_config.setObject(config);
    }
}

QJsonObject KitConfigManager::getKitConfig()
{
    QJsonObject rval;
    if (!m_config.isNull()){
        QVariantHash settings = m_config["KitConfig"].toObject().toVariantHash();
        rval = QJsonObject::fromVariantHash(settings["config"].toHash());

    }
    return rval;
}

void KitConfigManager::setKitConfig(QVariantHash config_values)
{
    if (!m_config.isNull()){
        QJsonObject config = m_config.object();
        QJsonObject kit_config = config["KitConfig"].toObject();
        kit_config.insert("config", QJsonObject::fromVariantHash(config_values));
        config["KitConfig"] = kit_config;
        m_config.setObject(config);
    }
}

void KitConfigManager::setKitConfig(QString kit_device_config)
{
    if (!m_config.isNull()){
        QJsonObject config = m_config.object();
        QJsonObject kit_config = config["KitConfig"].toObject();
        kit_config.insert("config", kit_device_config);
        config["KitConfig"] = kit_config;
        m_config.setObject(config);
    }
}

QString KitConfigManager::getParticipantServiceUrl()
{
    QString rval = "";
    if (!m_config.isNull()){
        QHash<QString, QVariant> settings = m_config["Participant"].toObject().toVariantHash();
        rval = settings["participant_service_url"].toString();
    }
    return rval;
}

void KitConfigManager::setParticipantServiceUrl(const QString &service_url)
{
    if (!m_config.isNull()){
        QJsonObject config = m_config.object();
        QJsonObject participant = config["Participant"].toObject();
        participant.insert("participant_service_url", service_url);
        config["Participant"] = participant;
        m_config.setObject(config);
    }
}

QString KitConfigManager::getServiceKey()
{
    QString rval = "";
    if (!m_config.isNull()){
        QHash<QString, QVariant> settings = m_config["Participant"].toObject().toVariantHash();
        rval = settings["service_key"].toString();
    }
    return rval;
}

void KitConfigManager::setServiceKey(const QString &service_key)
{
    if (!m_config.isNull()){
        QJsonObject config = m_config.object();
        QJsonObject participant = config["Participant"].toObject();
        participant.insert("service_key", service_key);
        config["Participant"] = participant;
        m_config.setObject(config);
    }
}

QString KitConfigManager::getTechSupportClient()
{
    QString rval = "";
    if (!m_config.isNull()){
        QHash<QString, QVariant> settings = m_config["TechSupport"].toObject().toVariantHash();
        rval = settings["client_path"].toString();
    }
    return rval;
}

void KitConfigManager::setTechSupportClient(const QString &client)
{
    if (!m_config.isNull()){
        QJsonObject config = m_config.object();
        QJsonObject tech_sup = config["TechSupport"].toObject();
        tech_sup.insert("client_path", client);
        config["TechSupport"] = tech_sup;
        m_config.setObject(config);
    }
}

QString KitConfigManager::getOtherSoftwarePath()
{
    QString rval = "";
    if (!m_config.isNull()){
        QHash<QString, QVariant> settings = m_config["OtherSoftware"].toObject().toVariantHash();
        rval = settings["software_path"].toString();
    }
    return rval;
}

void KitConfigManager::setOtherSoftwarePath(const QString &path)
{
    if (!m_config.isNull()){
        QJsonObject config = m_config.object();
        QJsonObject tech_sup = config["OtherSoftware"].toObject();
        tech_sup.insert("software_path", path);
        config["OtherSoftware"] = tech_sup;
        m_config.setObject(config);
    }
}


