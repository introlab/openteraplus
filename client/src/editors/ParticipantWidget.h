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

#ifndef OPENTERA_WEBASSEMBLY
#include "dialogs/SessionLobbyDialog.h"
#endif

#include "dialogs/EmailInviteDialog.h"
#include "dialogs/DeviceAssignDialog.h"
#include "dialogs/QRCodeDialog.h"

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

    QMap<int, QListWidgetItem*>     m_listAvailDevices_items;   // int  = device_id
    QMap<int, QListWidgetItem*>     m_listDevices_items;        // int  = device_id

    QList<TeraData>                 m_services;
    QMap<int, QWidget*>             m_services_tabs;
    bool                            m_allowFileTransfers;   // Allow to attach files to a session?

    QList<TeraData>                 m_testTypes;

    QRCodeDialog*                   m_diag_qr = nullptr;
#ifndef OPENTERA_WEBASSEMBLY
    SessionLobbyDialog*             m_sessionLobby;
#endif

    QHash<int, TeraData*>           m_ids_session_types;

    void updateControlsState() override;
    void updateFieldsValue() override;
    void initUI();

    bool canStartNewSession(const int& id_session_type=0);

    bool validateData() override;

    void updateDeviceProject(TeraData* device_project);
    void updateDeviceParticipant(TeraData* device_participant);

    void updateServiceTabs();
    void addServiceTab(const TeraData& service);

    void refreshWebAccessUrl();

    bool isProjectAdmin();

private slots:
    void processFormsReply(QString form_type, QString data);
    void processSessionTypesReply(QList<TeraData> session_types);
    //void processDevicesReply(QList<TeraData> devices);
    void processDeviceProjectsReply(QList<TeraData> device_projects);
    void processDeviceParticipantsReply(QList<TeraData> device_participants);
    void processParticipantsReply(QList<TeraData> participants);
    void processTestTypesReply(QList<TeraData> test_types, QUrlQuery reply_query);
    void processServicesReply(QList<TeraData> services, QUrlQuery reply_query);
    void processStatsReply(TeraData stats, QUrlQuery reply_query);

    void deleteDataReply(QString path, int id);
    void ws_participantEvent(opentera::protobuf::ParticipantEvent event);

    void sessionTotalCountUpdated(int new_count);

    void btnAddDevice_clicked();
    void btnDelDevice_clicked();
#ifndef OPENTERA_WEBASSEMBLY
    void showSessionLobby(const int& id_session_type, const int& id_session);
    void sessionLobbyStartSessionRequested();
    void sessionLobbyStartSessionCancelled();
#endif

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
    void on_btnViewLink_clicked();
    void on_cmbServices_currentIndexChanged(int index);
    void on_btnEmailWeb_clicked();
    void on_cmbSessionType_currentIndexChanged(int index);
    void on_tabNav_currentChanged(int index);
    void on_lstAvailDevices_itemDoubleClicked(QListWidgetItem *item);
    void on_lstDevices_itemDoubleClicked(QListWidgetItem *item);
    void on_btnQR_clicked();
    void on_tabInfosDetails_currentChanged(int index);
    void on_tabServicesDetails_currentChanged(int index);
};

#endif // PARTICIPANTWIDGET_H
