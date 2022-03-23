#include "TeraSessionEvent.h"

TeraSessionEvent::TeraSessionEvent(QObject *parent) : QObject(parent)
{

}

QString TeraSessionEvent::getEventTypeName(const TeraSessionEvent::SessionEventType &event)
{
    switch(event){
        case(GENERAL_ERROR):
            return tr("Erreur");
        case(GENERAL_INFO):
            return tr("Information");
        case(GENERAL_WARNING):
            return tr("Avertissement");
        case(SESSION_START):
            return tr("Début");
        case(SESSION_STOP):
            return tr("Fin");
        case(DEVICE_ON_CHARGE):
            return tr("Chargement - Début");
        case(DEVICE_OFF_CHARGE):
            return tr("Chargement - Fin");
        case(DEVICE_LOW_BATT):
            return tr("Pile faible");
        case(DEVICE_STORAGE_LOW):
            return tr("Espace disque faible");
        case(DEVICE_STORAGE_FULL):
            return tr("Espace disque plein");
        case(DEVICE_EVENT):
            return tr("Événement appareil");
        case(USER_EVENT):
            return tr("Marque");
        case(SESSION_JOIN):
            return tr("Arrivée");
        case(SESSION_LEAVE):
            return tr("Départ");
        case(SESSION_JOIN_REFUSED):
            return tr("Refus");
        case(CUSTOM_EVENT1):
        case(CUSTOM_EVENT2):
        case(CUSTOM_EVENT3):
        case(CUSTOM_EVENT4):
        case(CUSTOM_EVENT5):
        case(CUSTOM_EVENT6):
        case(CUSTOM_EVENT7):
        case(CUSTOM_EVENT8):
        case(CUSTOM_EVENT9):
        case(CUSTOM_EVENT10):
            return tr("Événément");
    }

    return tr("Inconnu");
}

QString TeraSessionEvent::getEventTypeName(const int &event)
{
    return getEventTypeName(static_cast<SessionEventType>(event));
}

QString TeraSessionEvent::getIconFilenameForEventType(const TeraSessionEvent::SessionEventType &event)
{
    switch(event){
        case(GENERAL_ERROR):
        case(DEVICE_STORAGE_FULL):
            return "://icons/error.png";
        case(GENERAL_INFO):
        case(DEVICE_EVENT):
        case(USER_EVENT):
            return "://icons/info.png";
        case(SESSION_START):
            return "://icons/play.png";
        case(SESSION_STOP):
            return "://icons/stop.png";
        case(DEVICE_ON_CHARGE):
        case(DEVICE_OFF_CHARGE):
            return "://status/status_incomplete.png";
        case(GENERAL_WARNING):
        case(DEVICE_LOW_BATT):
        case(DEVICE_STORAGE_LOW):
            return "://icons/warning.png";
        case SESSION_JOIN:
            return "://icons/join.png";
        case SESSION_LEAVE:
            return "://icons/leave.png";
        case SESSION_JOIN_REFUSED:
            return "://icons/error2.png";
        case(CUSTOM_EVENT1):
        case(CUSTOM_EVENT2):
        case(CUSTOM_EVENT3):
        case(CUSTOM_EVENT4):
        case(CUSTOM_EVENT5):
        case(CUSTOM_EVENT6):
        case(CUSTOM_EVENT7):
        case(CUSTOM_EVENT8):
        case(CUSTOM_EVENT9):
        case(CUSTOM_EVENT10):
            return "://icons/info.png";

    }
    return "";
}

QString TeraSessionEvent::getIconFilenameForEventType(const int &event)
{
    return getIconFilenameForEventType(static_cast<SessionEventType>(event));
}
