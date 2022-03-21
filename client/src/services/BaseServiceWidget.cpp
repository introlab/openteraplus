#include "BaseServiceWidget.h"

BaseServiceWidget::BaseServiceWidget(ComManager *comMan, QWidget *parent) :
    QWidget(parent),
    m_comManager(comMan)
{
    m_session = nullptr;
}

BaseServiceWidget::~BaseServiceWidget()
{
    if (m_session)
        delete m_session;
}

void BaseServiceWidget::setSession(const TeraData *session)
{
    if (m_session)
        delete m_session;

    m_session = new TeraData(*session);
}

bool BaseServiceWidget::handleJoinSessionEvent(const JoinSessionEvent &event)
{
    Q_UNUSED(event)
    // Don't do anything for now for joinSessionEvent in base class

    return true; // Accept by default
}

QStringList BaseServiceWidget::getHandledServiceKeys()
{
    // Get all handled service keys by that software
    // TODO: Dynamic plugin for each service

    return QStringList() << "VideoRehabService" /*<< "DanceService"*/ << "RobotTeleOperationService";
}

QList<TeraSessionCategory::SessionTypeCategories> BaseServiceWidget::getHandledSessionCategories()
{
    QList<TeraSessionCategory::SessionTypeCategories> categories;

    // List handled session types
    categories.append(TeraSessionCategory::SESSION_TYPE_SERVICE);

    return categories;
}
