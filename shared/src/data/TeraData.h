#ifndef TERADATA_H
#define TERADATA_H

#include <QObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>

#include <QMetaProperty>
#include <QUrl>

#include "WebAPI.h"
#include "ParticipantWebAPI.h"

enum TeraDataTypes {
    TERADATA_NONE,
    TERADATA_USER,
    TERADATA_USERGROUP,
    TERADATA_USERUSERGROUP,
    TERADATA_USERPREFERENCE,
    TERADATA_SITE,
    TERADATA_SESSIONTYPE,
    TERADATA_TESTTYPE,
    TERADATA_TESTTYPEPROJECT,
    TERADATA_TESTTYPESITE,
    TERADATA_PROJECT,
    TERADATA_DEVICE,
    TERADATA_GROUP,
    TERADATA_PARTICIPANT,
    TERADATA_SITEACCESS,
    TERADATA_PROJECTACCESS,
    TERADATA_SESSION,
    TERADATA_DEVICESITE,
    TERADATA_DEVICEPROJECT,
    TERADATA_DEVICEPARTICIPANT,
    TERADATA_DEVICESUBTYPE,
    TERADATA_DEVICETYPE,
    TERADATA_SESSIONTYPEPROJECT,
    TERADATA_SESSIONTYPESITE,
    TERADATA_SESSIONEVENT,
    TERADATA_SERVICE,
    TERADATA_SERVICE_PROJECT,
    TERADATA_SERVICE_SITE,
    TERADATA_SERVICE_ACCESS,
    TERADATA_SERVICE_CONFIG,
    TERADATA_STATS,
    TERADATA_ONLINE_USER,
    TERADATA_ONLINE_PARTICIPANT,
    TERADATA_ONLINE_DEVICE,
    TERADATA_ASSET,
    TERADATA_TEST
};

Q_DECLARE_METATYPE(TeraDataTypes)

class TeraData : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int id READ getId CONSTANT)
    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(TeraDataTypes data_type READ getDataType WRITE setDataType NOTIFY dataTypeChanged)

public:
    explicit TeraData(QObject *parent = nullptr);
    explicit TeraData(TeraDataTypes obj_type, QObject *parent = nullptr);
    TeraData(const TeraData& copy, QObject *parent=nullptr);
    explicit TeraData(TeraDataTypes obj_type, const QJsonValue& json, QObject *parent = nullptr);

    bool        fromJson(const QJsonValue& value);
    virtual QJsonObject toJson(const QString specific_fieldName = QString());

    virtual bool        fromMap(const QVariantMap& map);

    int getId() const;
    QString getIdFieldName() const;
    void setId(const int& id);

    QString getUuid() const;
    QString getUuidFieldName() const;
    void setUuid(const QString& uuid);
    bool hasUuidField();

    virtual QString getName() const;
    void setName(const QString& name);
    bool hasNameField();

    bool hasEnabledField() const;
    bool isEnabled() const;

    bool hasOnlineStateField() const;
    bool hasBusyStateField() const;
    bool isOnline() const;
    bool isBusy() const;
    void setBusy(const bool &busy);
    void setOnline(const bool &online);

    bool isNew() const;

    virtual TeraData &operator = (const TeraData& other);
    virtual bool    operator == (const TeraData& other) const;
    void            updateFrom(const TeraData& other);
    void            updateFrom(const QJsonObject& object);

    TeraDataTypes   getDataType() const;
    bool            hasFieldName(const QString& fieldName) const;
    void            removeFieldName(const QString& fieldName);
    QVariant        getFieldValue(const QString &fieldName) const;
    void            setFieldValue(const QString& fieldName, const QVariant& fieldValue);
    QList<QString>  getFieldList() const;
    QVariantMap     getFieldValues();

    static QString getDataTypeName(const TeraDataTypes& data_type);
    static QString getDataTypeNameText(const TeraDataTypes& data_type);
    static TeraDataTypes getDataTypeFromPath(const QString& path);
    static QString getPathForDataType(const TeraDataTypes& data_type);

    static QString getIconFilenameForDataType(const TeraDataTypes& data_type);
    QString getIconStateFilename() const;

    static QString getServiceParticipantUrl(const TeraData &service, const QUrl &server_url, const QString &participant_token);

protected:

    TeraDataTypes   m_data_type;

private:
    QString     m_objectName;
    QString     m_idField;
    QString     m_nameField;
    QString     m_enabledField;
    QString     m_onlineField;
    QString     m_busyField;
    QString     m_uuidField;

    QVariantMap m_fieldsValue;

    //bool hasMetaProperty(const QString& fieldName) const;

signals:
    void dataTypeChanged(const TeraDataTypes& new_datatype);

public slots:
    void setDataType(TeraDataTypes data_type);

};

#endif // TERADATA_H
