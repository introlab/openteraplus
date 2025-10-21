#include "ActimetryConfigWidget.h"
#include "ui_ActimetryConfigWidget.h"

#include "ActimetryWebAPI.h"
#include "WebAPI.h"

ActimetryConfigWidget::ActimetryConfigWidget(ComManager *comManager, int projectId, QString participantUuid, QString participantName, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::ActimetryConfigWidget),
    m_comManager(comManager),
    m_idProject(projectId)
{
    m_ui->setupUi(this);

    // NOTE At this state, comManager is not ready, will need to get service information before and then change state to ready.
    m_ActimetryComManager = new ActimetryComManager(comManager);

    m_ui->tabMain->setCurrentIndex(0);
    m_ui->lblWarning->hide();
    m_ui->wdgMessages->hide();


    connectSignals();
}

ActimetryConfigWidget::~ActimetryConfigWidget()
{
    delete m_ui;
    delete m_ActimetryComManager;
}

void ActimetryConfigWidget::connectSignals()
{

    connect(m_ActimetryComManager, &ActimetryComManager::readyChanged, this, &ActimetryConfigWidget::serviceReadyChanged);
    connect(m_ui->wdgMessages, &ResultMessageWidget::nextMessageShown, this, &ActimetryConfigWidget::nextMessageWasShown);
}


void ActimetryConfigWidget::serviceReadyChanged(bool ready)
{
    if(ready)
    {


    }
}

void ActimetryConfigWidget::nextMessageWasShown(Message current_message)
{
    if (current_message.getMessageType()==Message::MESSAGE_NONE)
    {
        m_ui->wdgMessages->hide();
    }
    else
    {
        m_ui->wdgMessages->show();
    }
}
