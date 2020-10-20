#include "BaseServiceToolsWidget.h"

BaseServiceToolsWidget::BaseServiceToolsWidget(ComManager *comMan, BaseServiceWidget *baseWidget, QWidget *parent) :
    QWidget(parent),
    m_comManager(comMan),
    m_baseWidget(baseWidget)
{

}
