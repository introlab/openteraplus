#ifndef TESTINVITATIONDIALOG_H
#define TESTINVITATIONDIALOG_H

#include <QDialog>

namespace Ui {
class TestInvitationDialog;
}

class TestInvitationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TestInvitationDialog(QWidget *parent = nullptr);
    ~TestInvitationDialog();

private slots:
    void on_btnCancel_clicked();

    void on_stackedPages_currentChanged(int current_index);

    void on_btnPrevious_clicked();

    void on_btnNext_clicked();

private:
    Ui::TestInvitationDialog *ui;

    void initUI();
};

#endif // TESTINVITATIONDIALOG_H
