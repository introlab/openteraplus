#ifndef DEVICESUMMARYWIDGET_H
#define DEVICESUMMARYWIDGET_H

#include <QWidget>
#include <QStyledItemDelegate>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"
#include "dialogs/BaseDialog.h"
#include "TeraSessionStatus.h"
#include "Utils.h"
#include "ServiceConfigWidget.h"
#include "dialogs/SessionLobbyDialog.h"

namespace Ui {
class DeviceSummaryWidget;
}

class DeviceSummaryWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit DeviceSummaryWidget(ComManager* comMan, const TeraData* data = nullptr, const int &id_project = -1, QWidget *parent = nullptr);
    ~DeviceSummaryWidget();

    void saveData(bool signal=true);
    void connectSignals();

private:
    Ui::DeviceSummaryWidget *ui;

    QMap<int, TeraData*>            m_ids_session_types;

    BaseDialog*                     m_diag_editor;
    SessionLobbyDialog*             m_sessionLobby;

    int 	m_idProject;

    void updateControlsState();
    void updateFieldsValue();
    void initUI();

    bool validateData();

private slots:
    void processFormsReply(QString form_type, QString data);
    void processSiteAccessReply(QList<TeraData> access_list, QUrlQuery reply_query);
    void processSessionTypesReply(QList<TeraData> session_types);
    void processDevicesReply(QList<TeraData> devices);
    void processStatsReply(TeraData stats, QUrlQuery reply_query);

    void ws_deviceEvent(opentera::protobuf::DeviceEvent event);

    void sessionLobbyStartSessionRequested();
    void sessionLobbyStartSessionCancelled();

    void on_btnNewSession_clicked();
    void on_tabNav_currentChanged(int index);

    void sessionTotalCountUpdated(int new_count);
};

#endif // DEVICESUMMARYWIDGET_H
