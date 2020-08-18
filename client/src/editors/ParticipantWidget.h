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
#include "DeviceAssignDialog.h"
#include "BaseDialog.h"
#include "TeraSessionStatus.h"
#include "Utils.h"
#include "StartSessionDialog.h"

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
    QMap<int, TeraData*>            m_ids_sessions;

    QMap<int, QListWidgetItem*>     m_listAvailDevices_items;   // int  = device_id
    QMap<int, QListWidgetItem*>     m_listDevices_items;        // int  = device_id

    BaseDialog*                     m_diag_editor;

    void updateControlsState();
    void updateFieldsValue();
    void initUI();

    bool validateData();

    void updateSession(TeraData *session);
    void updateDeviceProject(TeraData* device_project);
    void updateDeviceParticipant(TeraData* device_participant);

    void updateCalendars(QDate left_date);
    QDate getMinimumSessionDate();

private slots:
    void processFormsReply(QString form_type, QString data);
    void processSessionsReply(QList<TeraData> sessions);
    void processSessionTypesReply(QList<TeraData> session_types);
    //void processDevicesReply(QList<TeraData> devices);
    void processDeviceProjectsReply(QList<TeraData> device_projects);
    void processDeviceParticipantsReply(QList<TeraData> device_participants);
    void deleteDataReply(QString path, int id);
    void onDownloadCompleted(DownloadedFile* file);

    void btnDeleteSession_clicked();
    void btnAddDevice_clicked();
    void btnDelDevice_clicked();
    void btnDownloadSession_clicked();
    void btnDowloadAll_clicked();
    void btnViewSession_clicked();

    void currentSelectedSessionChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
    void currentCalendarDateChanged(QDate current_date);
    void displaySessionDetails(QTableWidgetItem* session_item);
    void currentTypeFiltersChanged(QListWidgetItem* changed);
    void displayNextMonth();
    void displayPreviousMonth();

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
    void on_btnCheckSesstionTypes_clicked();
    void on_btnUnchekSessionTypes_clicked();
};

#endif // PARTICIPANTWIDGET_H
