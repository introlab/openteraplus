#ifndef PARTICIPANTWIDGET_H
#define PARTICIPANTWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QListWidgetItem>
#include <QToolButton>
#include <QFrame>
#include <QFileDialog>
#include <QStandardPaths>
#include <QSpacerItem>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"
#include "TeraSessionStatus.h"
#include "Utils.h"
#include "TeraSettings.h"
#include "ServiceConfigWidget.h"

#include "dialogs/SessionLobbyDialog.h"
#include "dialogs/EmailInviteDialog.h"
#include "dialogs/DeviceAssignDialog.h"
#include "dialogs/BaseDialog.h"

#include "widgets/TableDateWidgetItem.h"
#include "widgets/AssetsWidget.h"
#include "widgets/TestsWidget.h"

#include "services/BaseServiceWidget.h"

namespace Ui {
class ParticipantWidget;
}

class ParticipantWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit ParticipantWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~ParticipantWidget();

    void saveData(bool signal=true) override;
    void setData(const TeraData* data) override;

    void connectSignals();

private:
    Ui::ParticipantWidget *ui;

    QMap<int, QTableWidgetItem*>    m_listSessions_items; // ID Session to QTableWidgetItem* mapping

    QMap<int, TeraData*>            m_ids_session_types;
    QMap<int, TeraData*>            m_ids_sessions;       // ID Session to data mapping

    QMap<int, QListWidgetItem*>     m_listAvailDevices_items;   // int  = device_id
    QMap<int, QListWidgetItem*>     m_listDevices_items;        // int  = device_id

    QList<TeraData>                 m_services;
    QMap<int, QWidget*>             m_services_tabs;
    bool                            m_allowFileTransfers;   // Allow to attach files to a session?

    BaseDialog*                     m_diag_editor;
    SessionLobbyDialog*             m_sessionLobby;

    int                             m_totalSessions;
    int                             m_totalAssets;
    int                             m_totalTests;
    int                             m_currentSessions;
    bool                            m_sessionsLoading;

    // Infos when querying extra sessions
    int                             m_currentIdSession;
    bool                            m_currentSessionShowAssets;
    bool                            m_currentSessionShowTests;

    // Icons are defined here (speed up load)
    QIcon m_deleteIcon;
    QIcon m_viewIcon;
    QIcon m_downloadIcon;
    QIcon m_testIcon;
    QIcon m_resumeIcon;


    void setSessionsLoading(const bool& loading);
    void querySessions();

    void updateControlsState() override;
    void updateFieldsValue() override;
    void initUI();

    bool canStartNewSession(const int& id_session_type=0);
    void newSessionRequest(const QDateTime& session_datetime);

    bool validateData() override;

    void updateSession(const TeraData *session, const bool &auto_position);
    void updateDeviceProject(TeraData* device_project);
    void updateDeviceParticipant(TeraData* device_participant);

    void updateServiceTabs();
    void addServiceTab(const TeraData& service);

    void refreshWebAccessUrl();

    void updateCalendars(QDate left_date);
    QDate getMinimumSessionDate();
    QDate getMaximumSessionDate();

    bool eventFilter(QObject* o, QEvent* e) override;
    void displaySessionDetails(QTableWidgetItem* session_item, bool show_assets = false, bool show_tests = false);
    void showSessionEditor(TeraData *session_info);

    bool isProjectAdmin();


private slots:
    void processFormsReply(QString form_type, QString data);
    void processSessionsReply(QList<TeraData> sessions);
    void processSessionTypesReply(QList<TeraData> session_types);
    //void processDevicesReply(QList<TeraData> devices);
    void processDeviceProjectsReply(QList<TeraData> device_projects);
    void processDeviceParticipantsReply(QList<TeraData> device_participants);
    void processParticipantsReply(QList<TeraData> participants);
    void processServicesReply(QList<TeraData> services, QUrlQuery reply_query);
    void processStatsReply(TeraData stats, QUrlQuery reply_query);

    void deleteDataReply(QString path, int id);
    void ws_participantEvent(opentera::protobuf::ParticipantEvent event);
    void sessionAssetsCountChanged(int id_session, int new_count);
    void sessionTestsCountChanged(int id_session, int new_count);

    void btnDeleteSession_clicked();
    void btnAddDevice_clicked();
    void btnDelDevice_clicked();
    void btnDownloadSession_clicked();
    void btnViewSession_clicked();
    void btnResumeSession_clicked();
    void btnViewTests_clicked();

    void currentSelectedSessionChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
    void currentCalendarDateChanged(QDate current_date);
    void currentCalendarDateActivated(QDate current_date);
    void currentTypeFiltersChanged(QListWidgetItem* changed);
    void displayNextMonth();
    void displayPreviousMonth();

    void showSessionLobby(const int& id_session_type, const int& id_session);
    void sessionLobbyStartSessionRequested();
    void sessionLobbyStartSessionCancelled();

    void currentAvailDeviceChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void currentDeviceChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void on_chkEnabled_stateChanged(int checkState);
    void on_chkWebAccess_stateChanged(int checkState);
    void on_chkLogin_stateChanged(int arg1);
    void on_btnCopyWeb_clicked();
    void on_btnRandomPass_clicked();
    void on_btnSaveLogin_clicked();
    void on_txtUsername_textEdited(const QString &current);
    void on_txtPassword_textEdited(const QString &current);
    void on_btnNewSession_clicked();
    void on_btnCheckSessionTypes_clicked();
    void on_btnUnchekSessionTypes_clicked();
    void on_btnViewLink_clicked();
    void on_cmbServices_currentIndexChanged(int index);
    void on_btnEmailWeb_clicked();
    void on_cmbSessionType_currentIndexChanged(int index);
    void on_btnFilterSessionsTypes_clicked();
    void on_btnAddSession_clicked();
    void on_tableSessions_itemDoubleClicked(QTableWidgetItem *item);
    void on_btnAssetsBrowser_clicked();
    void on_tabNav_currentChanged(int index);
    void on_lstAvailDevices_itemDoubleClicked(QListWidgetItem *item);
    void on_lstDevices_itemDoubleClicked(QListWidgetItem *item);
    void on_btnTestsBrowser_clicked();
};

#endif // PARTICIPANTWIDGET_H
