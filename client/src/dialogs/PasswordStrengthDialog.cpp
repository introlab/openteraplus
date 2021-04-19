#include "PasswordStrengthDialog.h"
#include "ui_PasswordStrengthDialog.h"

PasswordStrengthDialog::PasswordStrengthDialog(QString current_password, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasswordStrengthDialog)
{
    ui->setupUi(this);
    setCurrentPassword(current_password);

}

PasswordStrengthDialog::~PasswordStrengthDialog()
{
    delete ui;
}

void PasswordStrengthDialog::setCurrentPassword(const QString &password)
{
    ui->txtPassword->setText(password);
}

QString PasswordStrengthDialog::getCurrentPassword()
{
    return ui->txtPassword->text();
}

void PasswordStrengthDialog::setCursorPosition(int pos)
{
    ui->txtPassword->setCursorPosition(pos);
}

void PasswordStrengthDialog::setValidationItemState(int index, bool state)
{
    if (index < ui->lstValidate->count() && index >= 0){
        if (state){
            ui->lstValidate->item(index)->setCheckState(Qt::Checked);
            ui->lstValidate->item(index)->setForeground(Qt::green);
        }else{
            ui->lstValidate->item(index)->setCheckState(Qt::Unchecked);
            ui->lstValidate->item(index)->setForeground(Qt::red);
        }
    }
}

void PasswordStrengthDialog::checkPasswordsMatch()
{
    if (ui->txtPassword->text() != ui->txtPasswordConf->text()){
        ui->txtPasswordConf->setStyleSheet("background-color: #ffaaaa;");
        ui->btnOK->setEnabled(false);
    }else{
        ui->txtPasswordConf->setStyleSheet("");
        ui->btnOK->setEnabled(m_errors.isEmpty());
    }
}

void PasswordStrengthDialog::on_btnOK_clicked()
{
    accept();
}

void PasswordStrengthDialog::on_btnCancel_clicked()
{
    reject();
}

void PasswordStrengthDialog::on_txtPassword_textChanged(const QString &current_pass)
{
   m_errors = Utils::validatePassword(current_pass);

   setValidationItemState(0, !m_errors.contains(Utils::PASSWORD_LENGTH));
   setValidationItemState(1, !m_errors.contains(Utils::PASSWORD_NONOCAPS));
   setValidationItemState(2, !m_errors.contains(Utils::PASSWORD_NOCAPS));
   setValidationItemState(3, !m_errors.contains(Utils::PASSWORD_NODIGITS));
   setValidationItemState(4, !m_errors.contains(Utils::PASSWORD_NOCHAR));

   checkPasswordsMatch();

}

void PasswordStrengthDialog::on_txtPasswordConf_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)

    checkPasswordsMatch();
}
