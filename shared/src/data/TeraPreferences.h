#ifndef TERAPREFERENCES_H
#define TERAPREFERENCES_H

#include <QObject>
#include "TeraData.h"
#include <QJsonDocument>
#include "SharedLib.h"

class SHAREDLIB_EXPORT TeraPreferences : public QObject
{
    Q_OBJECT

public:
    explicit TeraPreferences(QObject *parent = nullptr);
    TeraPreferences(const TeraPreferences& copy, QObject *parent=nullptr);

    TeraPreferences &operator = (const TeraPreferences& other);

    Q_PROPERTY(bool isSet READ isSet WRITE setSet)
    Q_PROPERTY(QString language READ getLanguage WRITE setLanguage)
    Q_PROPERTY(bool notifySounds READ isNotifySounds WRITE setNotifySounds)

    bool isSet() const;
    QString getLanguage() const;
    bool isNotifySounds() const;

    void load(const QString& pref_str);
    void load(const TeraData& pref_obj);

    void clear();

public slots:
    void setLanguage(QString language);
    void setNotifySounds(bool notifySounds);

    void setSet(bool isSet);

private:
    bool m_isSet;
    QString m_language;
    bool m_notifySounds;


signals:

};

#endif // TERAPREFERENCES_H
