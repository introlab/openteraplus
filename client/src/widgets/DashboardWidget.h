#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QHash>
#include <QTableWidgetItem>
#include <QTreeWidgetItem>
#include <QToolButton>

#include "managers/ComManager.h"

#include "widgets/TableDateWidgetItem.h"

#ifndef OPENTERA_WEBASSEMBLY
#include "dialogs/SessionLobbyDialog.h"
#endif

#include "dialogs/CleanUpDialog.h"

#include "GlobalMessageBox.h"
#include "TeraForm.h"

namespace Ui {
class DashboardWidget;
}

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(ComManager* comMan, int id_site, QWidget *parent = nullptr);
    ~DashboardWidget();

    void setCurrentSiteId(const int &id_site, const bool &refresh_data=true);

private slots:
    void on_btnUpcomingSessions_clicked();
    void on_btnRecentParticipants_clicked();
    void on_btnAttention_clicked();
    void on_icoUpcomingSessions_clicked();
    void on_icoRecentParticipants_clicked();
    void on_icoAttention_clicked();

    void processSessionsReply(const QList<TeraData> sessions);
    void processSessionTypesReply(const QList<TeraData> session_types);
    void processParticipantsReply(const QList<TeraData> participants, const QUrlQuery reply_query);
    void processStatsReply(const TeraData stats, const QUrlQuery reply_query);
#ifndef OPENTERA_WEBASSEMBLY
    void sessionLobbyStartSessionCancelled();
    void sessionLobbyStartSessionRequested();
#endif
    void cleanupDialogCompleted();

    void on_tableUpcomingSessions_itemDoubleClicked(QTableWidgetItem *item);
    void on_tableRecentParticipants_itemDoubleClicked(QTableWidgetItem *item);

    void btnManageWarning_clicked();

signals:
    void dataDisplayRequest(TeraDataTypes data_type, int data_id);


private:
    Ui::DashboardWidget *ui;

    ComManager*                     m_comManager;
    int                             m_siteId;
#ifndef OPENTERA_WEBASSEMBLY
    SessionLobbyDialog*             m_sessionLobby;
#endif
    CleanUpDialog*                  m_cleanupDiag;

    QHash<QTableWidgetItem*, int>   m_listSessions_items;
    QHash<int, TeraData*>           m_sessions;
    QHash<int, TeraData*>           m_session_types;

    QHash<QTableWidgetItem*, int>   m_listParticipants_items;

    void connectSignals();

    void refreshData();

    void updateUiSpacing();
#ifndef OPENTERA_WEBASSEMBLY
    void showSessionLobby(const int &id_session_type, const int &id_session);
#endif
    QToolButton *createManageWarningButton();

};

#endif // DASHBOARDWIDGET_H
