#include "BaseServiceToolsWidget.h"

BaseServiceToolsWidget::BaseServiceToolsWidget(ComManager *comMan, BaseServiceWidget *baseWidget, QWidget *parent) :
    QWidget(parent),
    m_comManager(comMan),
    m_baseWidget(baseWidget)
{

}

bool BaseServiceToolsWidget::sessionCanBeEnded()
{
    // By default, all sessions can be ended
    return true;
}

void BaseServiceToolsWidget::setReadyState(bool ready_state)
{
    // Nothing here by default!
}
