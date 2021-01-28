#include "GeneratePasswordDialog.h"
#include "ui_GeneratePasswordDialog.h"

#include "Utils.h"

GeneratePasswordDialog::GeneratePasswordDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneratePasswordDialog)
{
    ui->setupUi(this);

    ui->txtPassword->setText(Utils::generatePassword(10));
}

GeneratePasswordDialog::~GeneratePasswordDialog()
{
    delete ui;
}

QString GeneratePasswordDialog::getPassword()
{
    return ui->txtPassword->text();
}

void GeneratePasswordDialog::on_btnGenerate_clicked()
{
    ui->txtPassword->setText(Utils::generatePassword(10));
}

void GeneratePasswordDialog::on_btnOK_clicked()
{
    accept();
}

void GeneratePasswordDialog::on_btnCancel_clicked()
{
    reject();
}

void GeneratePasswordDialog::on_btnCopy_clicked()
{
    QClipboard* clipboard = QApplication::clipboard();

    clipboard->setText(ui->txtPassword->text(), QClipboard::Clipboard);
}
