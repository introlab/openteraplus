#include "TeraSessionType.h"

TeraSessionType::TeraSessionType(QObject *parent)
    : TeraData{parent}
{

}

TeraSessionType::TeraSessionType(const TeraData &copy, QObject *parent)
: TeraData{copy, parent}
{

}

QString TeraSessionType::getAssetsRelatedServiceUuid() const
{

    if (hasFieldName("session_type_service_has_assets")){
        if (getFieldValue("session_type_service_has_assets").toBool()){
            // Got a service with assets
            return getFieldValue("session_type_service_uuid").toString();
        }
    }
    // Backward compatibility with previous OpenTera instances
    if (hasFieldName("session_type_service_key")){
        QString key =getFieldValue("session_type_service_key").toString();
        if (key == "FileTransferService" || key == "ActimetryService"){
            return getFieldValue("session_type_service_uuid").toString();
        }
    }

    // Check related services
    if (hasFieldName("session_type_secondary_services")){
        QVariantList services = getFieldValue("session_type_secondary_services").toList();
        for(const QVariant &service:std::as_const(services)){
            QVariantMap mapping = service.toMap();
            if (mapping.contains("service_has_assets")){
                if (mapping["service_has_assets"].toBool()){
                    return mapping["service_uuid"].toString();
                }
            }
            // Backward compatibility with previous OpenTera instances
            QString key = mapping["service_uuid"].toString();
            if (key == "FileTransferService" || key == "ActimetryService"){
                return mapping["service_uuid"].toString();
            }
        }
    }
    return QString();
}
