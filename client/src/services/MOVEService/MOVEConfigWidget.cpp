#include "MOVEConfigWidget.h"
#include "ui_MOVEConfigWidget.h"

#include "MOVEWebAPI.h"
#include "WebAPI.h"

MOVEConfigWidget::MOVEConfigWidget(ComManager *comManager, int projectId, QString participantUuid, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::MOVEConfigWidget),
    m_comManager(comManager),
    m_idProject(projectId),
    m_uuidParticipant(participantUuid)
{
    m_ui->setupUi(this);
    m_ui->cmbSessionTypes->setItemDelegate(new QStyledItemDelegate(m_ui->cmbSessionTypes));

    //NOTE At this state, comManager is not ready, will need to get service information before and then change state to ready.
    m_MOVEComManager = new MOVEComManager(comManager);


    m_ui->tabMain->setCurrentIndex(0);
    m_ui->lblWarning->hide();
    m_ui->wdgMessages->hide();

    connectSignals();
}

MOVEConfigWidget::~MOVEConfigWidget()
{
    delete m_ui;
    delete m_MOVEComManager;
}


void MOVEConfigWidget::connectSignals()
{
    connect(m_MOVEComManager, &MOVEComManager::participantProfileReceived, this, &MOVEConfigWidget::participantProfileReceived);
    connect(m_MOVEComManager, &MOVEComManager::readyChanged, this, &MOVEConfigWidget::serviceReadyChanged);
}

void MOVEConfigWidget::participantProfileReceived(QJsonObject user_profile, QUrlQuery reply_query)
{
    qDebug() << "MOVEConfigWidget::participantProfileReceived " << user_profile << reply_query.toString();

    /*
     {"id_participant_activities_schedule":1,
     "id_participant_profile":1,
     "participant_context":"PROFILE TEST",
     "participant_location":"Sherbrooke, QC",
     "participant_name":"P1-Test",
     "participant_profile_last_updated":"2025-07-14T11:13:49.510023-04:00",
    "participant_uuid":"9e11a737-266f-468e-bedb-ea7e4da4c496"}
    */

    if (user_profile.contains("participant_context"))
    {
        //Get Context
        QString context = user_profile.value("participant_context").toString();

        //Update textedit
        m_ui->textEdit->setText(context);
    }

}

void MOVEConfigWidget::serviceReadyChanged(bool ready)
{
    if(ready)
    {
        //Try to get the profile
        m_MOVEComManager->queryParticipantProfile(m_uuidParticipant);
    }
}
