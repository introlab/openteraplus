#include "TeraPreferences.h"

TeraPreferences::TeraPreferences(QObject *parent) : QObject(parent)
{
    clear();
}

TeraPreferences::TeraPreferences(const TeraPreferences &copy, QObject *parent)
{
    *this = copy;
}

TeraPreferences &TeraPreferences::operator =(const TeraPreferences &other)
{

    m_language = other.m_language;
    m_notifySounds = other.m_notifySounds;
    m_isSet = other.m_isSet;

    return *this;
}

void TeraPreferences::load(const TeraData &pref_obj)
{
    if (!pref_obj.hasFieldName("user_preference_preference")){
        qWarning("TeraPreferences::load - missing 'user_preference_preference' in TeraData object.");
        return;
    }
    QString prefs = pref_obj.getFieldValue("user_preference_preference").toString();

    load(prefs);
}

void TeraPreferences::clear()
{
    // Reset default values
    m_language = "";
    m_notifySounds = true;
    m_isSet = false;

}

void TeraPreferences::load(const QString &pref_str)
{
    QJsonDocument doc = QJsonDocument::fromJson(pref_str.toUtf8());

    QVariantMap values = doc.object().toVariantMap();

    if (values.contains("language")){
        m_language = values["language"].toString();
    }
    if (values.contains("notify_sounds")){
        m_notifySounds = values["notify_sounds"].toBool();
    }

    m_isSet = true;
}


void TeraPreferences::setSet(bool isSet)
{
    m_isSet = isSet;
}

bool TeraPreferences::isSet() const
{
    return m_isSet;
}

QString TeraPreferences::getLanguage() const
{
    return m_language;
}

bool TeraPreferences::isNotifySounds() const
{
    return m_notifySounds;
}

void TeraPreferences::setLanguage(QString language)
{
    m_language = language;
}

void TeraPreferences::setNotifySounds(bool notifySounds)
{
    m_notifySounds = notifySounds;
}
