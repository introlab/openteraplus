#include "TeraSettings.h"

TeraSettings::TeraSettings()
{

}

void TeraSettings::setGlobalSetting(const QString &key, const QVariant &value)
{
    QSettings settings;
    settings.setValue("global/" + key, value);
}

QVariant TeraSettings::getGlobalSetting(const QString &key)
{
    QSettings settings;
    return settings.value("global/" + key);
}

void TeraSettings::setUserSetting(const QString &user_uuid, const QString &key, const QVariant &value)
{
    QSettings settings;
    settings.setValue("user/" + user_uuid + "/" + key, value);
}

QVariant TeraSettings::getUsersetting(const QString &user_uuid, const QString &key)
{
    QSettings settings;
    return settings.value("user/" + user_uuid + "/" + key);
}
