#include "MOVEConfigWidget.h"
#include "ui_MOVEConfigWidget.h"

#include "MOVEWebAPI.h"
#include "WebAPI.h"

MOVEConfigWidget::MOVEConfigWidget(ComManager *comManager, int projectId, QString participantUuid, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MOVEConfigWidget),
    m_comManager(comManager),
    m_idProject(projectId),
    m_uuidParticipant(participantUuid)
{
    ui->setupUi(this);
    ui->cmbSessionTypes->setItemDelegate(new QStyledItemDelegate(ui->cmbSessionTypes));

    m_MOVEComManager = new MOVEComManager(comManager);


    ui->tabMain->setCurrentIndex(0);
    ui->lblWarning->hide();
    ui->wdgMessages->hide();

    connectSignals();
}

MOVEConfigWidget::~MOVEConfigWidget()
{
    delete ui;
    delete m_MOVEComManager;
}


void MOVEConfigWidget::connectSignals()
{

}
