#ifndef TERADATA_H
#define TERADATA_H

#include <QObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>

#include <QMetaProperty>

#include "WebAPI.h"

enum TeraDataTypes {
    TERADATA_NONE,
    TERADATA_USER,
    TERADATA_USERGROUP,
    TERADATA_USERUSERGROUP,
    TERADATA_SITE,
    TERADATA_SESSIONTYPE,
    TERADATA_TESTDEF,
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
    TERADATA_DEVICEDATA,
    TERADATA_SESSIONTYPEPROJECT,
    TERADATA_SESSIONEVENT,
    TERADATA_SERVICE,
    TERADATA_SERVICE_PROJECT,
    TERADATA_SERVICE_ACCESS,
    TERADATA_SERVICE_CONFIG,
    TERADATA_STATS
};

Q_DECLARE_METATYPE(TeraDataTypes)

class TeraData : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int id READ getId)
    Q_PROPERTY(QString name READ getName)
    Q_PROPERTY(TeraDataTypes data_type READ getDataType WRITE setDataType)

public:
    explicit TeraData(QObject *parent = nullptr);
    explicit TeraData(TeraDataTypes obj_type, QObject *parent = nullptr);
    TeraData(const TeraData& copy, QObject *parent=nullptr);
    explicit TeraData(TeraDataTypes obj_type, const QJsonValue& json, QObject *parent = nullptr);

    virtual bool        fromJson(const QJsonValue& value);
    virtual QJsonObject toJson(const QString specific_fieldName = QString());

    int getId() const;
    QString getIdFieldName() const;
    void setId(const int& id);

    virtual QString getName() const;
    void setName(const QString& name);
    bool hasNameField();

    bool hasEnabledField();
    bool isEnabled();

    bool isNew();

    virtual TeraData &operator = (const TeraData& other);
    virtual bool operator == (const TeraData& other) const;
    void updateFrom(const TeraData& other);

    TeraDataTypes getDataType() const;
    bool hasFieldName(const QString& fieldName) const;
    void removeFieldName(const QString& fieldName);
    QVariant getFieldValue(const QString &fieldName) const;
    void setFieldValue(const QString& fieldName, const QVariant& fieldValue);
    QList<QString> getFieldList() const;
    QVariantMap getFieldValues();

    static QString getDataTypeName(const TeraDataTypes& data_type);
    static QString getDataTypeNameText(const TeraDataTypes& data_type);
    static TeraDataTypes getDataTypeFromPath(const QString& path);
    static QString getPathForDataType(const TeraDataTypes& data_type);

    static QString getIconFilenameForDataType(const TeraDataTypes& data_type);

protected:

    TeraDataTypes   m_data_type;

private:
    QString     m_objectName;
    QString     m_idField;
    QString     m_nameField;
    QString     m_enabledField;

    QVariantMap m_fieldsValue;

    //bool hasMetaProperty(const QString& fieldName) const;

signals:


public slots:
    void setDataType(TeraDataTypes data_type);

};

#endif // TERADATA_H
