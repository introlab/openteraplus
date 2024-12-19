#ifndef SESSIONSLISTWIDGET_H
#define SESSIONSLISTWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QListWidgetItem>

#include "managers/ComManager.h"
#include "dialogs/BaseDialog.h"

#define QUERY_SESSION_LIMIT_PER_QUERY 50

namespace Ui {
class SessionsListWidget;
}

class SessionsListWidget : public QWidget
{
    Q_OBJECT

public:
    enum ViewMode{
        VIEW_NONE = 0,
        VIEW_PARTICIPANT_SESSIONS,
        VIEW_USER_SESSIONS,
        VIEW_DEVICE_SESSIONS
    };

    explicit SessionsListWidget(QWidget *parent = nullptr);
    ~SessionsListWidget();

    void setComManager(ComManager* comMan);
    void setViewMode(const ViewMode& mode, const QString &uuid, const int &id, const int &id_project = -1);

    void initUI();
    void connectSignals();

    void enableDeletion(const bool& enable);
    void enableFileTransfers(const bool& enable);
    void enableTestInvitations(QList<TeraData>* test_types);

    void setSessionsCount(const int& count);
    int getSessionsCount();

    void sessionWasDeleted(const int& id_session);

    void setSessionTypes(QList<TeraData> session_types); // This function will trigger a data load / refresh

    const TeraData* hasResumableSession(const int& id_session_type, const QDate& target_date);
    void newSessionRequest(const QDateTime& session_datetime);

private:
    Ui::SessionsListWidget          *ui;
    ComManager*                     m_comManager;
    ViewMode                        m_viewMode;

    QHash<int, QTableWidgetItem*>   m_listSessions_items; // ID Session to QTableWidgetItem* mapping
    QList<TeraData>*                m_testTypes = nullptr; // Tests types available to the widget

    BaseDialog*                     m_diag_editor;

    QHash<int, TeraData*>           m_ids_session_types;
    QHash<int, TeraData*>           m_ids_sessions;       // ID Session to data mapping

    int                             m_totalSessions;
    int                             m_totalAssets;
    int                             m_totalTests;
    int                             m_currentSessions;
    bool                            m_sessionsLoading;

    bool                            m_allowDeletion;
    bool                            m_allowFileTransfers;

    QString                         m_currentUuid;
    int                             m_currentId;
    int                             m_currentIdProject;

    // Icons are defined here (speed up load)
    QIcon m_deleteIcon;
    QIcon m_viewIcon;
    QIcon m_downloadIcon;
    QIcon m_testIcon;
    QIcon m_resumeIcon;

    // Infos when querying extra sessions
    int                             m_currentIdSession;
    bool                            m_currentSessionShowAssets;
    bool                            m_currentSessionShowTests;

    void updateCalendars(QDate left_date);
    QDate getMinimumSessionDate();
    QDate getMaximumSessionDate();

    void setSessionsLoading(const bool& loading);
    void querySessions();

    bool eventFilter(QObject* o, QEvent* e) override;

    void updateSession(const TeraData *session, const bool &auto_position);
    void showSessionEditor(TeraData *session_info);
    void displaySessionDetails(QTableWidgetItem* session_item, bool show_assets = false, bool show_tests = false);


private slots:
    void processSessionsReply(QList<TeraData> sessions, QUrlQuery args);
    void deleteDataReply(QString path, int id);

    void btnViewSession_clicked();
    void btnViewSessionAssets_clicked();
    void btnViewSessionTests_clicked();
    void btnResumeSession_clicked();

    void sessionAssetsCountChanged(int id_session, int new_count);
    void sessionTestsCountChanged(int id_session, int new_count);

    void on_btnDelSession_clicked();
    void on_tableSessions_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
    void on_lstFilters_itemChanged(QListWidgetItem *changed);

    void on_btnNextCal_clicked();
    void on_btnPrevCal_clicked();
    void currentCalendarDateChanged(QDate current_date);
    void currentCalendarDateActivated(QDate current_date);

    void on_btnCheckSessionTypes_clicked();

    void on_btnUncheckSessionTypes_clicked();

    void on_btnFilterSessionsTypes_clicked();

    void on_tableSessions_itemDoubleClicked(QTableWidgetItem *item);

    void on_btnAssetsBrowser_clicked();

    void on_btnTestsBrowser_clicked();

signals:
    void startSessionRequested(int id_session_type, int id_session);
    void sessionsCountUpdated(int new_total);
};

#endif // SESSIONSLISTWIDGET_H
