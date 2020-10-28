#ifndef TERASETTINGS_H
#define TERASETTINGS_H

#include <QSettings>

class TeraSettings
{
public:
    TeraSettings();

    static void setGlobalSetting(const QString &key, const QVariant &value);
    static QVariant getGlobalSetting(const QString& key);

    static void setUserSetting(const QString &user_uuid, const QString& key, const QVariant &value);
    static QVariant getUsersetting(const QString &user_uuid, const QString& key);
};

#endif // TERASETTINGS_H
