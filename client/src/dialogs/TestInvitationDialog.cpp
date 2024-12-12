#include "TestInvitationDialog.h"
#include "ui_TestInvitationDialog.h"

TestInvitationDialog::TestInvitationDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TestInvitationDialog)
{
    ui->setupUi(this);
    setModal(true);

    initUI();
}

TestInvitationDialog::~TestInvitationDialog()
{
    delete ui;
}

void TestInvitationDialog::initUI()
{
    ui->stackedPages->setCurrentIndex(0);
    ui->btnPrevious->setEnabled(false);
    ui->btnOK->setEnabled(false);
}

void TestInvitationDialog::on_btnCancel_clicked()
{
    reject();
}


void TestInvitationDialog::on_stackedPages_currentChanged(int current_index)
{
    ui->btnPrevious->setEnabled(current_index>0);
    ui->btnNext->setEnabled(current_index < ui->stackedPages->count() - 1);
    ui->btnOK->setEnabled(current_index == ui->stackedPages->count() -1);
}


void TestInvitationDialog::on_btnPrevious_clicked()
{
    if (ui->stackedPages->currentIndex() > 0)
        ui->stackedPages->setCurrentIndex(ui->stackedPages->currentIndex()-1);
}


void TestInvitationDialog::on_btnNext_clicked()
{
    if (ui->stackedPages->currentIndex() < ui->stackedPages->count() -1)
        ui->stackedPages->setCurrentIndex(ui->stackedPages->currentIndex()+1);
}


void TestInvitationDialog::on_btnOK_clicked()
{
    accept();
}

