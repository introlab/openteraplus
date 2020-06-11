#include "TeraSessionCategory.h"

TeraSessionCategory::TeraSessionCategory(QObject *parent) : QObject(parent)
{

}

QString TeraSessionCategory::getSessionTypeCategoryName(const TeraSessionCategory::SessionTypeCategories &category)
{
    switch(category){
        case SESSION_TYPE_SERVICE:
            return tr("Service");
        break;
    case SESSION_TYPE_PROTOCOL:
        return tr("Protocole");
        break;
    case SESSION_TYPE_DATACOLLECT:
        return tr("Collecte de données");
        break;
    case SESSION_TYPE_FILETRANSFER:
        return tr("Transfert de fichier");
    default:
        return tr("");
    }
}

QString TeraSessionCategory::getSessionTypeCategoryName(const int &category)
{
    return getSessionTypeCategoryName(static_cast<SessionTypeCategories>(category));
}
