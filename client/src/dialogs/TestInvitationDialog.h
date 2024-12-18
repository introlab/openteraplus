#ifndef TESTINVITATIONDIALOG_H
#define TESTINVITATIONDIALOG_H

#include <QDialog>
#include "managers/ComManager.h"
#include "services/EmailService/EmailComManager.h"
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
    void setCurrentSession(const TeraData* session);
    void setCurrentData(TeraData* data);
    void setEnableEmail(const bool& enable_email);
    void setEnableInviteesList(const bool& enable);

    void setInvitableDevices(QHash<int, TeraData>* devices);
    void setInvitableParticipants(QHash<int, TeraData>* participants);
    void setInvitableUsers(QHash<int, TeraData>* users);

    void addDevicesToInvitation(const QStringList &device_uuids);
    void addParticipantsToInvitation(const QStringList &participant_uuids);
    void addUsersToInvitation(const QStringList &user_uuids);

private slots:
    void processTestInvitationsReply(QList<TeraData> invitations);
    void emailSentSuccess();
    void emailSentError(QNetworkReply::NetworkError error, QString error_str, QNetworkAccessManager::Operation op, int status_code, QString path, QUrlQuery url_query);

    void on_btnCancel_clicked();
    void on_stackedPages_currentChanged(int current_index);
    void on_btnPrevious_clicked();
    void on_btnNext_clicked();
    void on_btnOK_clicked();
    void on_btnDone_clicked();
    void on_chkInviteEmail_checkStateChanged(const Qt::CheckState &state);

private:
    Ui::TestInvitationDialog *ui;
    ComManager*         m_comManager;
    EmailComManager*    m_emailComManager;
    TeraData*   m_data = nullptr;
    bool        m_enableEmails = false;

    QList<TeraData> m_pendingInvitations;
    int         m_currentSessionId = 0;

    void initUI();
    void processNextEmail();
};

#endif // TESTINVITATIONDIALOG_H
