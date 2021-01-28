#ifndef GENERATEPASSWORDDIALOG_H
#define GENERATEPASSWORDDIALOG_H

#include <QDialog>
#include <QClipboard>

namespace Ui {
class GeneratePasswordDialog;
}

class GeneratePasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GeneratePasswordDialog(QWidget *parent = nullptr);
    ~GeneratePasswordDialog();

    QString getPassword();

private slots:
    void on_btnGenerate_clicked();

    void on_btnOK_clicked();

    void on_btnCancel_clicked();

    void on_btnCopy_clicked();

private:
    Ui::GeneratePasswordDialog *ui;
};

#endif // GENERATEPASSWORDDIALOG_H
