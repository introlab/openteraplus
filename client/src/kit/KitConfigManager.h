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

    void            initConfig() override;

    QString         getParticipantToken();
    void            setParticipantToken(QString token);

    QJsonObject     getKitConfig();
    void            setKitConfig(QVariantHash config_values);
    void            setKitConfig(QString kit_device_config);

    QString         getParticipantServiceUrl();
    void            setParticipantServiceUrl(const QString& service_url);

    QString         getServiceKey();
    void            setServiceKey(const QString& service_key);

    QString         getTechSupportClient();
    void            setTechSupportClient(const QString& client);

};

#endif // KITCONFIGMANAGER_H
