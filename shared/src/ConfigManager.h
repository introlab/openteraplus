#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "SharedLib.h"
#include <QObject>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonObject>
#include <QHash>
#include <QUrl>
#include <QVariant>

class SHAREDLIB_EXPORT ConfigManager : public QObject
{
    Q_OBJECT
public:
    explicit ConfigManager(QObject *parent = nullptr);
    ConfigManager(QString filename, QObject* parent = nullptr);

    void            setFilename(QString filename);
    QString         getFilename();

    virtual bool    loadConfig();
    virtual bool    saveConfig();

    QJsonParseError getLastParseError();
    bool            hasParseError();

    virtual void    initConfig();
    void            initConfig(QStringList sections);

protected:
    QString         m_filename;

    QJsonDocument   m_config;
    QJsonParseError m_lastJsonError;

signals:

public slots:

};

#endif // CONFIGMANAGER_H
