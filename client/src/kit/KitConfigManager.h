#ifndef KITCONFIGMANAGER_H
#define KITCONFIGMANAGER_H

#include <ConfigManager.h>
#include <QObject>


class KitConfigManager : public ConfigManager
{
    Q_OBJECT
public:
    explicit KitConfigManager(QObject *parent = nullptr);
    KitConfigManager(QString filename, QObject* parent = nullptr);

    void                initConfig() override;

    QString             getParticipantToken();
    void                setParticipantToken(QString token);

    QVariantHash        getKitConfig();
    void                setKitConfig(QVariantHash config_values);

};

#endif // KITCONFIGMANAGER_H
