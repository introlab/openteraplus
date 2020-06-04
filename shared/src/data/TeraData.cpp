#include "TeraData.h"
#include <QDateTime>
#include <Logger.h>

#include <QDebug>

/*
TeraData::TeraData(QObject *parent) :
    QObject(parent)
{
    setDataType(TERADATA_NONE);
}*/

TeraData::TeraData(TeraDataTypes obj_type, QObject *parent) :
    QObject(parent)
{
    setDataType(obj_type);
}

TeraData::TeraData(const TeraData &copy, QObject *parent) : QObject (parent)
{
    *this = copy;
}

TeraData::TeraData(TeraDataTypes obj_type, const QJsonValue &json, QObject *parent):
    QObject(parent)
{
    setDataType(obj_type);
    fromJson(json);
}

int TeraData::getId() const
{
    QVariant raw_id = getFieldValue(m_idField);

    if (raw_id.isValid()){
        return raw_id.toInt();
    }
    return -1;
}

QString TeraData::getIdFieldName() const
{
    return m_idField;
}

void TeraData::setId(const int &id)
{
    setFieldValue(m_idField, id);
}

QString TeraData::getName() const
{
    QVariant raw_name = getFieldValue(m_nameField);
    if (raw_name.isValid())
        return raw_name.toString();

    return tr("Inconnu");
}

void TeraData::setName(const QString &name)
{
    setFieldValue(m_nameField, name);
}

bool TeraData::hasEnabledField()
{
    return hasFieldName(m_enabledField);
}

bool TeraData::isEnabled()
{
    return getFieldValue(m_enabledField).toBool();
}

bool TeraData::isNew()
{
    return getId()<=0;
}

TeraData &TeraData::operator =(const TeraData &other)
{

    setDataType(other.m_data_type);
    // qDebug() << "Other: " << other.dynamicPropertyNames();

    // qDebug() << "This: " << dynamicPropertyNames();
    /*for (int i=0; i<other.dynamicPropertyNames().count(); i++){
        //metaObject()->property(i).write(this, other.metaObject()->property(i).read(&other));
        QString name = QString(other.dynamicPropertyNames().at(i));
        // qDebug() << "TeraData::operator= - setting " << name;
        setFieldValue(name, other.property(name.toStdString().c_str()));
    }*/
    // qDebug() << "This final: " << dynamicPropertyNames();

    m_fieldsValue = other.m_fieldsValue;

    return *this;
}

bool TeraData::operator ==(const TeraData &other) const
{
    return getId() == other.getId() && getDataType() == other.getDataType();
}

void TeraData::updateFrom(const TeraData &other)
{
    foreach(QString key, other.m_fieldsValue.keys()){
        m_fieldsValue[key] = other.m_fieldsValue[key];
    }
}

TeraDataTypes TeraData::getDataType() const
{
    return m_data_type;
}

bool TeraData::hasFieldName(const QString &fieldName) const
{
    //return dynamicPropertyNames().contains(fieldName.toUtf8());
    return m_fieldsValue.contains(fieldName);
}

void TeraData::removeFieldName(const QString &fieldName)
{
    if (m_fieldsValue.contains(fieldName)){
        m_fieldsValue.remove(fieldName);
    }
}

QVariant TeraData::getFieldValue(const QString &fieldName) const
{
    if (hasFieldName(fieldName))
        //return property(fieldName.toStdString().c_str());
        return m_fieldsValue[fieldName];

    LOG_WARNING("Field " + fieldName + " not found in " + metaObject()->className(), "TeraData::getFieldValue");
    return QVariant();
}

void TeraData::setFieldValue(const QString &fieldName, const QVariant &fieldValue)
{
    //setProperty(fieldName.toStdString().c_str(), fieldValue);
    m_fieldsValue[fieldName] = fieldValue;
}

QList<QString> TeraData::getFieldList() const
{
    QList<QString> rval;
    //for (QByteArray fieldname:dynamicPropertyNames()){
    for (QString fieldname:m_fieldsValue.keys()){
        rval.append(fieldname);
    }

    return rval;
}

QVariantMap TeraData::getFieldValues()
{
    return m_fieldsValue;
}

QString TeraData::getDataTypeName(const TeraDataTypes &data_type)
{
    switch (data_type) {
    case TERADATA_NONE:
        return "";
    case TERADATA_USER:
        return "user";
    case TERADATA_USERGROUP:
        return "user_group";
    case TERADATA_SITE:
        return "site";
    case TERADATA_SESSIONTYPE:
        return "session_type";
    case TERADATA_TESTDEF:
        return "test_type";
    case TERADATA_PROJECT:
        return "project";
    case TERADATA_DEVICE:
        return "device";
    case TERADATA_PARTICIPANT:
        return "participant";
    case TERADATA_GROUP:
        return "participant_group";
    case TERADATA_SITEACCESS:
        return "siteaccess";
    case TERADATA_PROJECTACCESS:
        return "projectaccess";
    case TERADATA_SESSION:
        return "session";
    case TERADATA_DEVICESITE:
        return "device_site";
    case TERADATA_DEVICEPROJECT:
        return "device_project";
    case TERADATA_DEVICEPARTICIPANT:
        return "device_participant";
    case TERADATA_DEVICESUBTYPE:
        return "device_subtype";
    case TERADATA_SESSIONTYPEDEVICETYPE:
        return "session_type_device_type";
    case TERADATA_DEVICEDATA:
        return "device_data";
    case TERADATA_SESSIONTYPEPROJECT:
        return "session_type_project";
    case TERADATA_SESSIONEVENT:
        return "session_event";
    case TERADATA_SERVICE:
        return "service";
    case TERADATA_SERVICE_PROJECT:
        return "service_project";
    case TERADATA_SERVICE_PROJECT_ROLE:
        return "service_project_role";
    }

    return "";
}

QString TeraData::getDataTypeNameText(const TeraDataTypes &data_type)
{
    switch (data_type) {
    case TERADATA_NONE:
        return "";
    case TERADATA_USER:
        return tr("Utilisateur");
    case TERADATA_USERGROUP:
        return tr("Groupe utilisateur");
    case TERADATA_SITE:
        return tr("Site");
    case TERADATA_SESSIONTYPE:
        return tr("Type de séance");
    case TERADATA_TESTDEF:
        return tr("Type d'évaluation");
    case TERADATA_PROJECT:
        return tr("Projet");
    case TERADATA_DEVICE:
        return tr("Appareil");
    case TERADATA_PARTICIPANT:
        return tr("Participant");
    case TERADATA_GROUP:
        return tr("Groupe participant");
    case TERADATA_SITEACCESS:
        return tr("Accès: site");
    case TERADATA_PROJECTACCESS:
        return tr("Accès: projet");
    case TERADATA_SESSION:
        return tr("Séance");
    case TERADATA_DEVICESITE:
        return tr("Appareil: site");
    case TERADATA_DEVICEPROJECT:
        return tr("Appareil: projet");
    case TERADATA_DEVICEPARTICIPANT:
        return tr("Appareil: participant");
    case TERADATA_DEVICESUBTYPE:
        return tr("Appareil: sous-type");
    case TERADATA_SESSIONTYPEDEVICETYPE:
        return tr("Appareil: type de séance");
    case TERADATA_DEVICEDATA:
        return tr("Appareil: donnée");
    case TERADATA_SESSIONTYPEPROJECT:
        return tr("Type de séance: projet");
    case TERADATA_SESSIONEVENT:
        return tr("Séance: événement");
    case TERADATA_SERVICE:
        return tr("Service");
    case TERADATA_SERVICE_PROJECT:
        return tr("Service: projet");
    case TERADATA_SERVICE_PROJECT_ROLE:
        return tr("Service: Projet: Role");
    }

    return "";
}

TeraDataTypes TeraData::getDataTypeFromPath(const QString &path)
{
    if (path==WEB_USERINFO_PATH)                return TERADATA_USER;
    if (path==WEB_SITEINFO_PATH)                return TERADATA_SITE;
    if (path==WEB_PROJECTINFO_PATH)             return TERADATA_PROJECT;
    if (path==WEB_SITEACCESS_PATH)              return TERADATA_SITEACCESS;
    if (path==WEB_DEVICEINFO_PATH)              return TERADATA_DEVICE;
    if (path==WEB_PARTICIPANTINFO_PATH)         return TERADATA_PARTICIPANT;
    if (path==WEB_PROJECTACCESS_PATH)           return TERADATA_PROJECTACCESS;
    if (path==WEB_GROUPINFO_PATH)               return TERADATA_GROUP;
    if (path==WEB_SESSIONINFO_PATH)             return TERADATA_SESSION;
    if (path==WEB_SESSIONTYPE_PATH)             return TERADATA_SESSIONTYPE;
    if (path==WEB_DEVICESITEINFO_PATH)          return TERADATA_DEVICESITE;
    if (path==WEB_DEVICEPROJECTINFO_PATH)       return TERADATA_DEVICEPROJECT;
    if (path==WEB_DEVICEPARTICIPANTINFO_PATH)   return TERADATA_DEVICEPARTICIPANT;
    if (path==WEB_SESSIONTYPEDEVICETYPE_PATH)   return TERADATA_SESSIONTYPEDEVICETYPE;
    if (path==WEB_DEVICEDATAINFO_PATH)          return TERADATA_DEVICEDATA;
    if (path==WEB_SESSIONTYPEPROJECT_PATH)      return TERADATA_SESSIONTYPEPROJECT;
    if (path==WEB_SESSIONEVENT_PATH)            return TERADATA_SESSIONEVENT;
    if (path==WEB_DEVICESUBTYPE_PATH)           return TERADATA_DEVICESUBTYPE;
    if (path==WEB_USERGROUPINFO_PATH)           return TERADATA_USERGROUP;
    if (path==WEB_SERVICEINFO_PATH)             return TERADATA_SERVICE;
    if (path==WEB_SERVICEPROJECTINFO_PATH)      return TERADATA_SERVICE_PROJECT;
    if (path==WEB_SERVICEPROJECTROLEINFO_PATH)  return TERADATA_SERVICE_PROJECT_ROLE;

    LOG_ERROR("Unknown data type for path: " + path, "TeraData::getDataTypeFromPath");

    return TERADATA_NONE;
}

QString TeraData::getPathForDataType(const TeraDataTypes &data_type)
{
    if (data_type==TERADATA_USER)               return WEB_USERINFO_PATH;
    if (data_type==TERADATA_USERGROUP)          return WEB_USERGROUPINFO_PATH;
    if (data_type==TERADATA_SITE)               return WEB_SITEINFO_PATH;
    if (data_type==TERADATA_PROJECT)            return WEB_PROJECTINFO_PATH;
    if (data_type==TERADATA_DEVICE)             return WEB_DEVICEINFO_PATH;
    if (data_type==TERADATA_PARTICIPANT)        return WEB_PARTICIPANTINFO_PATH;
    if (data_type==TERADATA_PROJECTACCESS)      return WEB_PROJECTACCESS_PATH;
    if (data_type==TERADATA_GROUP)              return WEB_GROUPINFO_PATH;
    if (data_type==TERADATA_SESSION)            return WEB_SESSIONINFO_PATH;
    if (data_type==TERADATA_SESSIONTYPE)        return WEB_SESSIONTYPE_PATH;
    if (data_type==TERADATA_DEVICEPARTICIPANT)  return WEB_DEVICEPARTICIPANTINFO_PATH;
    if (data_type==TERADATA_DEVICESUBTYPE)      return WEB_DEVICESUBTYPE_PATH;
    if (data_type==TERADATA_SERVICE)            return WEB_SERVICEINFO_PATH;

    LOG_ERROR("Unknown path for data_type: " + getDataTypeName(data_type), "TeraData::getPathForDataType");

    return QString();
}

QString TeraData::getIconFilenameForDataType(const TeraDataTypes &data_type)
{
    switch(data_type){
    case TERADATA_USER:
        return "://icons/software_user.png";
    case TERADATA_USERGROUP:
        return "://icons/usergroup.png";
    case TERADATA_SITE:
        return "://icons/site.png";
    case TERADATA_SESSIONTYPE:
        return "://icons/session_type.png";
    case TERADATA_SESSION:
        return "://icons/session.png";
    case TERADATA_TESTDEF:
        return "://icons/test.png";
    case TERADATA_DEVICE:
        return "://icons/device.png";
    case TERADATA_PARTICIPANT:
        return "://icons/patient.png";
    case TERADATA_GROUP:
        return "://icons/group.png";
    case TERADATA_PROJECT:
    case TERADATA_SERVICE_PROJECT:
    case TERADATA_SESSIONTYPEPROJECT:
        return "://icons/project.png";
    case TERADATA_DEVICEDATA:
        return "://icons/sensors.png";
    case TERADATA_DEVICESUBTYPE:
        return "://icons/kit.png";
    case TERADATA_SERVICE:
        return "://icons/service.png";
    default:
        return "://icons/error.png";
    }

}

/*
bool TeraData::hasMetaProperty(const QString &fieldName) const
{
    for (int i=0; i<metaObject()->propertyCount(); i++){
        if (QString(metaObject()->property(i).name()) == fieldName){
            return true;
        }
    }
    return false;
}*/

void TeraData::setDataType(TeraDataTypes data_type)
{
    m_data_type = data_type;

    // Build default fields - name, ids
    m_objectName = getDataTypeName(m_data_type);
    m_idField = "id_" + m_objectName;
    m_nameField = m_objectName + "_name";
    m_enabledField = m_objectName + "_enabled";


}

bool TeraData::fromJson(const QJsonValue &value)
{
    if (value.isObject()){
        QJsonObject json_object = value.toObject();
        for (int i=0; i<json_object.keys().count(); i++){
            QString fieldName = json_object.keys().at(i);
            setFieldValue(fieldName, json_object[fieldName].toVariant());
        }
    }else{
        LOG_WARNING("Trying to load a JSON object which is not an object.", "TeraData::fromJson");
    }

    return true;
}

QJsonObject TeraData::toJson()
{
    QJsonObject object;

    for (int i=0; i<m_fieldsValue.count(); i++){
        QString fieldName = m_fieldsValue.keys().at(i);
        // Ignore "metaObject" properties
        QVariant fieldData = getFieldValue(fieldName);
        if (fieldData.canConvert(QMetaType::QString)){
            QDateTime date_tester = QDateTime::fromString(fieldData.toString(), Qt::ISODateWithMs);
            if (date_tester.isValid()){
                object[fieldName] = fieldData.toDateTime().toString(Qt::ISODateWithMs);
            }else{
                object[fieldName] = fieldData.toString();
            }
        }else if (fieldData.canConvert(QMetaType::QJsonValue)){
            object[fieldName] = fieldData.toJsonValue();
        }else{
            LOG_WARNING("Field " + fieldName + " can't be 'jsonized'", "TeraData::toJson");
        }
    }

    return object;
}
