#include "DanceConfigWidget.h"
#include "ui_DanceConfigWidget.h"

DanceConfigWidget::DanceConfigWidget(ComManager *comManager, int projectId, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DanceConfigWidget),
    m_comManager(comManager),
    m_idProject(projectId)
{
    ui->setupUi(this);

    m_danceComManager = new DanceComManager(comManager);

    ui->tabMain->setCurrentIndex(0);

}

DanceConfigWidget::~DanceConfigWidget()
{
    delete ui;
    delete m_danceComManager;
}

void DanceConfigWidget::on_btnUpload_clicked()
{

}

