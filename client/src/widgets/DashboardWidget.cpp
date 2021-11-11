#include "DashboardWidget.h"
#include "ui_DashboardWidget.h"

DashboardWidget::DashboardWidget(ComManager *comMan, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DashboardWidget),
    m_comManager(comMan)
{
    ui->setupUi(this);
}

DashboardWidget::~DashboardWidget()
{
    delete ui;
}

void DashboardWidget::on_btnUpcomingSessions_clicked()
{
    ui->frameUpcomingSessions->setVisible(ui->btnUpcomingSessions->isChecked());
    if (!ui->btnUpcomingSessions->isChecked())
        ui->icoUpcomingSessions->setPixmap(QPixmap(":/controls/branch_closed.png"));
    else
        ui->icoUpcomingSessions->setPixmap(QPixmap(":/controls/branch_opened.png"));
}


void DashboardWidget::on_btnRecentParticipants_clicked()
{
    ui->frameRecent->setVisible(ui->btnRecentParticipants->isChecked());
    if (!ui->btnRecentParticipants->isChecked())
        ui->icoRecentParticipants->setPixmap(QPixmap(":/controls/branch_closed.png"));
    else
        ui->icoRecentParticipants->setPixmap(QPixmap(":/controls/branch_opened.png"));
}


void DashboardWidget::on_btnAttention_clicked()
{
    ui->frameAttention->setVisible(ui->btnAttention->isChecked());
    if (!ui->btnAttention->isChecked())
        ui->icoAttention->setPixmap(QPixmap(":/controls/branch_closed.png"));
    else
        ui->icoAttention->setPixmap(QPixmap(":/controls/branch_opened.png"));
}

void DashboardWidget::on_icoUpcomingSessions_clicked()
{
    ui->btnUpcomingSessions->setChecked(!ui->btnUpcomingSessions->isChecked());
    on_btnUpcomingSessions_clicked();
}

void DashboardWidget::on_icoRecentParticipants_clicked()
{
    ui->btnRecentParticipants->setChecked(!ui->btnRecentParticipants->isChecked());
    on_btnRecentParticipants_clicked();
}

void DashboardWidget::on_icoAttention_clicked()
{
    ui->btnAttention->setChecked(!ui->btnAttention->isChecked());
    on_btnAttention_clicked();
}

