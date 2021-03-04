#ifndef PARTICIPANTWIDGET_H
#define PARTICIPANTWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>
#include <QListWidgetItem>
#include <QToolButton>
#include <QFrame>
#include <QFileDialog>
#include <QStandardPaths>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"
#include "dialogs/DeviceAssignDialog.h"
#include "dialogs/BaseDialog.h"
#include "TeraSessionStatus.h"
#include "Utils.h"
//#include "StartSessionDialog.h"
#include "ServiceConfigWidget.h"
#include "dialogs/SessionLobbyDialog.h"

#include "dialogs/EmailInviteDialog.h"

#include "widgets/TableDateWidgetItem.h"

namespace Ui {
class ParticipantWidget;
}

class ParticipantWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit ParticipantWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~ParticipantWidget();

    void saveData(bool signal=true);

    void connectSignals();

private:
    Ui::ParticipantWidget *ui;

    QMap<int, QTableWidgetItem*>    m_listSessions_items; // ID Session to QTableWidgetItem* mapping

    QMap<int, TeraData*>            m_ids_session_types;
    QMap<int, TeraData*>            m_ids_sessions;       // ID Session to data mapping

    QMap<int, QListWidgetItem*>     m_listAvailDevices_items;   // int  = device_id
    QMap<int, QListWidgetItem*>     m_listDevices_items;        // int  = device_id

    QList<TeraData>                 m_services;

    BaseDialog*                     m_diag_editor;
    SessionLobbyDialog*             m_sessionLobby;

    void updateControlsState();
    void updateFieldsValue();
    void initUI();

    bool canStartNewSession();

    bool validateData();

    void updateSession(TeraData *session);
    void updateDeviceProject(TeraData* device_project);
    void updateDeviceParticipant(TeraData* device_participant);

    void refreshWebAccessUrl();

    void updateCalendars(QDate left_date);
    QDate getMinimumSessionDate();

private slots:
    void processFormsReply(QString form_type, QString data);
    void processSessionsReply(QList<TeraData> sessions);
    void processSessionTypesReply(QList<TeraData> session_types);
    //void processDevicesReply(QList<TeraData> devices);
    void processDeviceProjectsReply(QList<TeraData> device_projects);
    void processDeviceParticipantsReply(QList<TeraData> device_participants);
    void processParticipantsReply(QList<TeraData> participants);
    void processServicesReply(QList<TeraData> services);
    void deleteDataReply(QString path, int id);
    void onDownloadCompleted(DownloadedFile* file);
    void ws_participantEvent(ParticipantEvent event);

    void btnDeleteSession_clicked();
    void btnAddDevice_clicked();
    void btnDelDevice_clicked();
    void btnDownloadSession_clicked();
    void btnDowloadAll_clicked();
    void btnViewSession_clicked();
    void btnResumeSession_clicked();

    void currentSelectedSessionChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
    void currentCalendarDateChanged(QDate current_date);
    void displaySessionDetails(QTableWidgetItem* session_item);
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
    void on_txtPasswordConfirm_textEdited(const QString &current);
    void on_tabInfos_currentChanged(int index);
    void on_btnNewSession_clicked();
    void on_btnCheckSessionTypes_clicked();
    void on_btnUnchekSessionTypes_clicked();
    void on_btnViewLink_clicked();
    void on_cmbServices_currentIndexChanged(int index);
    void on_btnEmailWeb_clicked();
    void on_cmbSessionType_currentIndexChanged(int index);
    void on_btnFilterSessionsTypes_clicked();
};

#endif // PARTICIPANTWIDGET_H
