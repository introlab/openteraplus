#include "TeraData.h"
#include <QDateTime>
#include <Logger.h>

#include <QDebug>


TeraData::TeraData(QObject *parent) :
    QObject(parent)
{
    setDataType(TERADATA_NONE);
}

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

QString TeraData::getUuid() const
{
    QVariant raw_uuid = getFieldValue(m_uuidField);

    if (raw_uuid.isValid() && raw_uuid.canConvert(QMetaType::QString)){
        return raw_uuid.toString();
    }
    return "";
}

QString TeraData::getUuidFieldName() const
{
    return m_uuidField;
}

void TeraData::setUuid(const QString &uuid)
{
    setFieldValue(m_uuidField, uuid);
}

bool TeraData::hasUuidField()
{
    return hasFieldName(m_uuidField);
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

bool TeraData::hasNameField()
{
    return hasFieldName(m_nameField);
}

bool TeraData::hasEnabledField() const
{
    return hasFieldName(m_enabledField);
}

bool TeraData::isEnabled() const
{
    if (hasEnabledField())
        return getFieldValue(m_enabledField).toBool();
    return false;
}

bool TeraData::hasOnlineStateField() const
{
    return hasFieldName(m_onlineField);
}

bool TeraData::hasBusyStateField() const
{
    return hasFieldName(m_busyField);
}


bool TeraData::isOnline() const
{
    if (hasOnlineStateField())
        return getFieldValue(m_onlineField).toBool();
    return false;
}

bool TeraData::isBusy() const
{
    if (hasBusyStateField())
        return getFieldValue(m_busyField).toBool();
    return false;
}

void TeraData::setBusy(const bool &busy)
{
    m_fieldsValue[m_busyField] = busy;
}

void TeraData::setOnline(const bool &online)
{
    m_fieldsValue[m_onlineField] = online;
}

bool TeraData::isNew() const
{
    return getId()<=0;
}

TeraData &TeraData::operator =(const TeraData &other)
{

    setDataType(other.m_data_type);
    m_fieldsValue = other.m_fieldsValue;

    return *this;
}

bool TeraData::operator ==(const TeraData &other) const
{
    return getId() == other.getId() && getDataType() == other.getDataType();
}

void TeraData::updateFrom(const TeraData &other)
{
    const QStringList keys = other.m_fieldsValue.keys();
    foreach(QString key, keys){
        m_fieldsValue[key] = other.m_fieldsValue[key];
    }
}

void TeraData::updateFrom(const QJsonObject &object)
{
    QVariantHash fields = object.toVariantHash();
    for(const QVariant &field_value:qAsConst(fields)){
        m_fieldsValue[fields.key(field_value)] = field_value;
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

    if (fieldName != getIdFieldName())
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
    const QStringList keys = m_fieldsValue.keys();
    //for (const QString &fieldname:m_fieldsValue.keys()){
    foreach(QString fieldname, keys){
    //foreach(QVariant value, m_fieldsValue){
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
    case TERADATA_ONLINE_USER:
        return "user";
    case TERADATA_USERGROUP:
        return "user_group";
    case TERADATA_USERUSERGROUP:
        return "user_user_group";
    case TERADATA_SITE:
        return "site";
    case TERADATA_SESSIONTYPE:
        return "session_type";
    case TERADATA_TESTDEF:
        return "test_type";
    case TERADATA_PROJECT:
        return "project";
    case TERADATA_DEVICE:
    case TERADATA_ONLINE_DEVICE:
        return "device";
    case TERADATA_PARTICIPANT:
    case TERADATA_ONLINE_PARTICIPANT:
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
    case TERADATA_DEVICETYPE:
        return "device_type";
    case TERADATA_SESSIONTYPEPROJECT:
        return "session_type_project";
    case TERADATA_SESSIONEVENT:
        return "session_event";
    case TERADATA_SERVICE:
        return "service";
    case TERADATA_SERVICE_PROJECT:
        return "service_project";
    case TERADATA_SERVICE_SITE:
        return "service_site";
    case TERADATA_SERVICE_ACCESS:
        return "service_access";
    case TERADATA_SERVICE_CONFIG:
        return "service_config";
    case TERADATA_STATS:
        return "stats";
    case TERADATA_USERPREFERENCE:
        return "user_preference";
    case TERADATA_ASSET:
        return "asset";
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
    case TERADATA_USERUSERGROUP:
        return tr("Utilisateurs: Groupe");
    case TERADATA_USERPREFERENCE:
        return tr("Utilisateurs: Préférences");
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
    case TERADATA_DEVICETYPE:
        return tr("Appareil: type");
    case TERADATA_SESSIONTYPEPROJECT:
        return tr("Type de séance: projet");
    case TERADATA_SESSIONEVENT:
        return tr("Séance: événement");
    case TERADATA_SERVICE:
        return tr("Service");
    case TERADATA_SERVICE_PROJECT:
        return tr("Service: projet");
    case TERADATA_SERVICE_SITE:
        return tr("Service: site");
    case TERADATA_SERVICE_ACCESS:
        return tr("Service: Accès");
    case TERADATA_SERVICE_CONFIG:
        return tr("Service: Configuration");
    case TERADATA_STATS:
        return tr("Statistiques");
    case TERADATA_ONLINE_DEVICE:
        return tr("Appareil: état");
    case TERADATA_ONLINE_PARTICIPANT:
        return tr("Participant: état");
    case TERADATA_ONLINE_USER:
        return tr("Utilisateur: état");
    case TERADATA_ASSET:
        return tr("Donnée");
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
    if (path==WEB_PARTICIPANTINFO_PATH || path==WEB_PARTICIPANT_PARTICIPANTINFO_PATH)         return TERADATA_PARTICIPANT;
    if (path==WEB_PROJECTACCESS_PATH)           return TERADATA_PROJECTACCESS;
    if (path==WEB_GROUPINFO_PATH)               return TERADATA_GROUP;
    if (path==WEB_SESSIONINFO_PATH)             return TERADATA_SESSION;
    if (path==WEB_SESSIONTYPE_PATH)             return TERADATA_SESSIONTYPE;
    if (path==WEB_DEVICESITEINFO_PATH)          return TERADATA_DEVICESITE;
    if (path==WEB_DEVICEPROJECTINFO_PATH)       return TERADATA_DEVICEPROJECT;
    if (path==WEB_DEVICEPARTICIPANTINFO_PATH)   return TERADATA_DEVICEPARTICIPANT;
    if (path==WEB_SESSIONTYPEPROJECT_PATH)      return TERADATA_SESSIONTYPEPROJECT;
    if (path==WEB_SESSIONEVENT_PATH)            return TERADATA_SESSIONEVENT;
    if (path==WEB_DEVICESUBTYPE_PATH)           return TERADATA_DEVICESUBTYPE;
    if (path==WEB_DEVICETYPE_PATH)              return TERADATA_DEVICETYPE;
    if (path==WEB_USERGROUPINFO_PATH)           return TERADATA_USERGROUP;
    if (path==WEB_SERVICEINFO_PATH)             return TERADATA_SERVICE;
    if (path==WEB_SERVICEPROJECTINFO_PATH)      return TERADATA_SERVICE_PROJECT;
    if (path==WEB_SERVICESITEINFO_PATH)         return TERADATA_SERVICE_SITE;
    if (path==WEB_SERVICEACCESSINFO_PATH)       return TERADATA_SERVICE_ACCESS;
    if (path==WEB_SERVICECONFIGINFO_PATH)       return TERADATA_SERVICE_CONFIG;
    if (path==WEB_USERUSERGROUPINFO_PATH)       return TERADATA_USERUSERGROUP;
    if (path==WEB_USERPREFSINFO_PATH)           return TERADATA_USERPREFERENCE;
    if (path==WEB_STATS_PATH)                   return TERADATA_STATS;
    if (path==WEB_ONLINEDEVICEINFO_PATH)        return TERADATA_ONLINE_DEVICE;
    if (path==WEB_ONLINEPARTICIPANTINFO_PATH)   return TERADATA_ONLINE_PARTICIPANT;
    if (path==WEB_ONLINEUSERINFO_PATH)          return TERADATA_ONLINE_USER;
    if (path==WEB_ASSETINFO_PATH)               return TERADATA_ASSET;

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
    if (data_type==TERADATA_DEVICETYPE)         return WEB_DEVICETYPE_PATH;
    if (data_type==TERADATA_SERVICE)            return WEB_SERVICEINFO_PATH;
    if (data_type==TERADATA_SITEACCESS)         return WEB_SITEACCESS_PATH;
    if (data_type==TERADATA_SERVICE_CONFIG)     return WEB_SERVICECONFIGINFO_PATH;
    if (data_type==TERADATA_ASSET)              return WEB_ASSETINFO_PATH;

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
    case TERADATA_SERVICE_SITE:
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
    case TERADATA_DEVICESUBTYPE:
    case TERADATA_DEVICETYPE:
        return "://icons/kit.png";
    case TERADATA_SERVICE:
    case TERADATA_SERVICE_CONFIG:
        return "://icons/service.png";
    case TERADATA_ASSET:
        return "://icons/data.png";
    default:
        return "://icons/error.png";
    }

}

QString TeraData::getIconStateFilename() const
{
    switch(m_data_type){
    case TERADATA_USER:
    case TERADATA_ONLINE_USER:
        if (isBusy() && isOnline())
            return "://icons/software_user_online_busy.png";
        if (isBusy())
            return "://icons/software_user_offline_busy.png";
        if (isOnline())
            return "://icons/software_user_online.png";
        return "://icons/software_user.png";

    case TERADATA_DEVICE:
    case TERADATA_ONLINE_DEVICE:
        if (isBusy() && isOnline())
            return "://icons/device_online_busy.png";
        if (isBusy())
            return "://icons/device_installed_busy.png";
        if (isOnline())
            return "://icons/device_online.png";
        if (isEnabled())
            return "://icons/device_installed.png";
        return "://icons/device.png";

    case TERADATA_PARTICIPANT:
    case TERADATA_ONLINE_PARTICIPANT:
        if (isBusy() && isOnline())
            return "://icons/patient_online_busy.png";
        if (isBusy())
            return "://icons/patient_installed_busy.png";
        if (isOnline())
            return "://icons/patient_online.png";
        if (isEnabled())
            return "://icons/patient_installed.png";
        return "://icons/patient.png";
    default:
        return "://icons/error.png";
    }
}

QString TeraData::getServiceParticipantUrl(const TeraData &service, const QUrl& server_url, const QString &participant_token)
{
    if (service.getDataType() != TERADATA_SERVICE){
        LOG_ERROR("Tried to generate a participant service url with a non-service object", "TeraData::getServiceParticipantUrl");
        return QString();
    }
    QString participant_endpoint;
    if (service.hasFieldName("service_endpoint_participant"))
        participant_endpoint = service.getFieldValue("service_endpoint_participant").toString();
    QString service_url = "https://" + server_url.host() + ":" + QString::number(server_url.port()) +
            service.getFieldValue("service_clientendpoint").toString() +
            participant_endpoint + "?token=" +
            participant_token;

    return service_url;
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
    m_onlineField = m_objectName + "_online";
    m_busyField = m_objectName + "_busy";
    m_uuidField = m_objectName + "_uuid";
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

QJsonObject TeraData::toJson(const QString specific_fieldName)
{
    QJsonObject object;

    if (!specific_fieldName.isEmpty()){
        if (!hasFieldName(specific_fieldName)){
            LOG_WARNING("Field " + specific_fieldName + " not in fields list.", "TeraData::toJson");
            return object;
        }
    }

    QStringList keys = m_fieldsValue.keys();
    //for (int i=0; i<m_fieldsValue.count(); i++){
    foreach(QString fieldName, keys){
        //QString fieldName = m_fieldsValue.keys().at(i);

        if (!specific_fieldName.isEmpty()){
            if (fieldName != specific_fieldName)
                continue;
        }
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

bool TeraData::fromMap(const QVariantMap &map)
{
    foreach(QVariant value, map){
        QString key = map.key(value);
        setFieldValue(key, value);
    }

    return true;
}
