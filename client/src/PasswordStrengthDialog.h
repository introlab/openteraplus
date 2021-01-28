#ifndef PASSWORDSTRENGTHDIALOG_H
#define PASSWORDSTRENGTHDIALOG_H

#include <QDialog>
#include "Utils.h"

namespace Ui {
class PasswordStrengthDialog;
}

class PasswordStrengthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordStrengthDialog(QString current_password = QString(), QWidget *parent = nullptr);
    ~PasswordStrengthDialog();

    void setCurrentPassword(const QString& password);
    QString getCurrentPassword();

private:
    void setValidationItemState(int index, bool state);
    void checkPasswordsMatch();

private slots:
    void on_btnOK_clicked();

    void on_btnCancel_clicked();

    void on_txtPassword_textChanged(const QString &current_pass);

    void on_txtPasswordConf_textChanged(const QString &arg1);

private:
    Ui::PasswordStrengthDialog *ui;
    QList<Utils::PasswordValidationErrors> m_errors;
};

#endif // PASSWORDSTRENGTHDIALOG_H
