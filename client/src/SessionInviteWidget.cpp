#include "SessionInviteWidget.h"
#include "ui_SessionInviteWidget.h"

SessionInviteWidget::SessionInviteWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionInviteWidget)
{
    ui->setupUi(this);

    ui->frameSelector->hide();
}

SessionInviteWidget::~SessionInviteWidget()
{
    delete ui;
}

void SessionInviteWidget::on_btnManageInvitees_clicked()
{
    ui->frameSelector->setVisible(ui->btnManageInvitees->isChecked());

    if (ui->btnManageInvitees->isChecked()){
        ui->btnManageInvitees->setIcon(QIcon("://controls/branch_opened.png"));
    }else{
        ui->btnManageInvitees->setIcon(QIcon("://controls/branch_closed.png"));
    }
}
