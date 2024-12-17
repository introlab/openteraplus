#ifndef TESTINVITATIONSWIDGET_H
#define TESTINVITATIONSWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>

#include "managers/ComManager.h"
#include "dialogs/TestInvitationDialog.h"

namespace Ui {
class TestInvitationsWidget;
}

class TestInvitationsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TestInvitationsWidget(QWidget *parent = nullptr);
    explicit TestInvitationsWidget(ComManager* comMan, QWidget* parent = nullptr);
    ~TestInvitationsWidget();

    void setComManager(ComManager* comMan);
    void setCurrentTestTypes(const QList<TeraData>& test_types);
    void setInvitableDevices(QHash<int, TeraData>* devices);
    void setInvitableParticipants(QHash<int, TeraData>* participants);
    void setInvitableUsers(QHash<int, TeraData>* users);

    void loadForProject(const int& id_project);

private slots:
    void processTestInvitationsReply(QList<TeraData> invitations);
    void deleteDataReply(QString path, int id);

    void onTestInvitationDialogFinished(int result);

    void on_btnInvite_clicked();

    void on_deleteInvitation();
    void on_editInvitation();
    void on_copyInvitation();
    void on_viewInvitation();

    void on_tableInvitations_itemSelectionChanged();

private:
    typedef enum {
        COLUMN_USER = 0,
        COLUMN_PARTICIPANT = 1,
        COLUMN_DEVICE = 2,
        COLUMN_TESTTYPE = 3,
        COLUMN_LIMIT = 4,
        COLUMN_COUNT = 5,
        COLUMN_KEY = 6,
        COLUMN_CREATION = 7,
        COLUMN_EXPIRATION = 8,
        COLUMN_ACTIONS = 9
    }
    InviteColumns;

    typedef enum {
        VIEWMODE_UNKNOWN,
        VIEWMODE_DEVICE,
        VIEWMODE_PARTICIPANT,
        VIEWMODE_PROJECT,
        VIEWMODE_USER
    }
    ViewMode;

    Ui::TestInvitationsWidget       *ui;
    ComManager*                     m_comManager = nullptr;
    ViewMode                        m_currentView = VIEWMODE_UNKNOWN;

    QList<TeraData>                 m_testTypes;

    TestInvitationDialog*           m_invitationDialog = nullptr;

    QHash<int, QTableWidgetItem*>   m_listInvitations_items; // ID Invitation to QTableWidgetItem* mapping
    QHash<int, TeraData>            m_invitations;

    QHash<int, TeraData>*           m_invitableDevices = nullptr;
    QHash<int, TeraData>*           m_invitableParticipants = nullptr;
    QHash<int, TeraData>*           m_invitableUsers = nullptr;

    // Icons are defined here (speed up load)
    QIcon m_deleteIcon;
    QIcon m_viewIcon;
    QIcon m_editIcon;
    QIcon m_copyIcon;

    void initUI();
    void connectSignals();

    void updateInvitation(const TeraData* invitation);
    void setViewMode(const ViewMode &mode);
};

#endif // TESTINVITATIONSWIDGET_H
