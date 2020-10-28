#include "BaseServiceSetupWidget.h"

BaseServiceSetupWidget::BaseServiceSetupWidget(ComManager *comManager, QWidget *parent) : QWidget(parent),
     m_comManager(comManager)
{

}

QJsonDocument BaseServiceSetupWidget::getSetupConfig()
{
    return QJsonDocument();
}
