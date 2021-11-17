#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QHash>
#include <QTableWidgetItem>

#include "managers/ComManager.h"

#include "widgets/TableDateWidgetItem.h"

#include "dialogs/SessionLobbyDialog.h"
#include "GlobalMessageBox.h"

namespace Ui {
class DashboardWidget;
}

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(ComManager* comMan, int id_site, QWidget *parent = nullptr);
    ~DashboardWidget();

    void setCurrentSiteId(const int &id_site);

private slots:
    void on_btnUpcomingSessions_clicked();
    void on_btnRecentParticipants_clicked();
    void on_btnAttention_clicked();
    void on_icoUpcomingSessions_clicked();
    void on_icoRecentParticipants_clicked();
    void on_icoAttention_clicked();

    void processSessionsReply(QList<TeraData> sessions);
    void processSessionTypesReply(const QList<TeraData> session_types);

    void sessionLobbyStartSessionCancelled();
    void sessionLobbyStartSessionRequested();

    void on_tableUpcomingSessions_itemDoubleClicked(QTableWidgetItem *item);

signals:
    void dataDisplayRequest(TeraDataTypes data_type, int data_id);


private:
    Ui::DashboardWidget *ui;

    ComManager*                     m_comManager;
    int                             m_siteId;
    SessionLobbyDialog*             m_sessionLobby;

    QHash<QTableWidgetItem*, int>   m_listSessions_items;
    QHash<int, TeraData*>           m_sessions;
    QHash<int, TeraData*>           m_session_types;

    void connectSignals();

    void refreshData();

    void updateUiSpacing();

    void showSessionLobby(const int &id_session_type, const int &id_session);

};

#endif // DASHBOARDWIDGET_H
