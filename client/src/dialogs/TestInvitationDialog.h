#ifndef TESTINVITATIONDIALOG_H
#define TESTINVITATIONDIALOG_H

#include <QDialog>
#include "managers/ComManager.h"
#include "TeraData.h"

namespace Ui {
class TestInvitationDialog;
}

class TestInvitationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TestInvitationDialog(ComManager* comMan, QWidget *parent = nullptr);
    ~TestInvitationDialog();

    void setTestTypes(const QList<TeraData>& test_types);
    void setCurrentData(TeraData* data);

    void setInvitableDevices(QHash<int, TeraData>* devices);
    void setInvitableParticipants(QHash<int, TeraData>* participants);
    void setInvitableUsers(QHash<int, TeraData>* users);

private slots:
    void on_btnCancel_clicked();

    void on_stackedPages_currentChanged(int current_index);

    void on_btnPrevious_clicked();
    void on_btnNext_clicked();
    void on_btnOK_clicked();

private:
    Ui::TestInvitationDialog *ui;
    ComManager* m_comManager;
    TeraData*   m_data = nullptr;

    void initUI();
};

#endif // TESTINVITATIONDIALOG_H
