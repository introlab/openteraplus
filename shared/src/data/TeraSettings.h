#ifndef TERASETTINGS_H
#define TERASETTINGS_H

#include <QSettings>

#define SETTINGS_UI_ADVANCEDVIEW                "ui_advancedView"
#define SETTINGS_UI_FILTERINACTIVE              "ui_filterInactives"
#define SETTINGS_UI_ONLINEFILTERPARTICIPANTS    "ui_filterOnlineParticipants"
#define SETTINGS_UI_ONLINEFILTERUSERS           "ui_filterOnlineUsers"
#define SETTINGS_UI_ONLINEFILTERDEVICES         "ui_filterOnlineDevices"
#define SETTINGS_LASTSITEID                     "lastSiteId"
#define SETTINGS_LASTSESSIONTYPEID              "lastSessionTypeId"
#define SETTINGS_LASTLANGUAGE                   "lastLanguage"
#define SETTINGS_DATA_SAVEPATH                  "dataSavePath"

class TeraSettings
{
public:
    TeraSettings();

    static void setGlobalSetting(const QString &key, const QVariant &value);
    static QVariant getGlobalSetting(const QString& key);

    static void setUserSetting(const QString &user_uuid, const QString& key, const QVariant &value);
    static QVariant getUserSetting(const QString &user_uuid, const QString& key);

    static void setUserSettingForProject(const QString &user_uuid, const int& id_project, const QString& key, const QVariant &value);
    static QVariant getUserSettingForProject(const QString &user_uuid, const int& id_project, const QString& key);
};

#endif // TERASETTINGS_H
