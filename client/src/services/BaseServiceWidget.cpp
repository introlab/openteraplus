#include "BaseServiceWidget.h"

BaseServiceWidget::BaseServiceWidget(ComManager *comMan, QWidget *parent) :
    QWidget(parent),
    m_comManager(comMan)
{

}

bool BaseServiceWidget::handleJoinSessionEvent(const JoinSessionEvent &event)
{
    Q_UNUSED(event)
    // Don't do anything for now for joinSessionEvent in base class

    return true; // Accept by default
}
