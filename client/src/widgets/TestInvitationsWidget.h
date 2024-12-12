#ifndef TESTINVITATIONSWIDGET_H
#define TESTINVITATIONSWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>

#include "managers/ComManager.h"

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
    void loadForProject(const int& id_project);

private slots:
    void processTestInvitationsReply(QList<TeraData> invitations);

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

    Ui::TestInvitationsWidget *ui;
    ComManager*                 m_comManager = nullptr;
    ViewMode                    m_currentView = VIEWMODE_UNKNOWN;

    QHash<int, QTableWidgetItem*>   m_listInvitations_items; // ID Invitation to QTableWidgetItem* mapping

    void updateInvitation(const TeraData* invitation);
    void setViewMode(const ViewMode &mode);
};

#endif // TESTINVITATIONSWIDGET_H
