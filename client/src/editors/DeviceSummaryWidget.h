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

#ifndef OPENTERA_WEBASSEMBLY
#include "dialogs/SessionLobbyDialog.h"
#endif

namespace Ui {
class DeviceSummaryWidget;
}

class DeviceSummaryWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit DeviceSummaryWidget(ComManager* comMan, const TeraData* data = nullptr, const int &id_project = -1, QWidget *parent = nullptr);
    ~DeviceSummaryWidget() override;

    void saveData(bool signal=true) override;
    void connectSignals();

private:
    Ui::DeviceSummaryWidget *ui;

    QMap<int, TeraData*>            m_ids_session_types;

    BaseDialog*                     m_diag_editor;
#ifndef OPENTERA_WEBASSEMBLY
    SessionLobbyDialog*             m_sessionLobby;
#endif

    int 	m_idProject;

    void updateControlsState() override;
    void updateFieldsValue() override;
    void initUI();

    bool validateData() override;

private slots:
    void processFormsReply(QString form_type, QString data);
    void processSiteAccessReply(QList<TeraData> access_list, QUrlQuery reply_query);
    void processSessionTypesReply(QList<TeraData> session_types);
    void processDevicesReply(QList<TeraData> devices);
    void processStatsReply(TeraData stats, QUrlQuery reply_query);

    void ws_deviceEvent(opentera::protobuf::DeviceEvent event);
#ifndef OPENTERA_WEBASSEMBLY
    void sessionLobbyStartSessionRequested();
    void sessionLobbyStartSessionCancelled();
    void on_btnNewSession_clicked();
#endif
    void on_tabNav_currentChanged(int index);

    void sessionTotalCountUpdated(int new_count);
};

#endif // DEVICESUMMARYWIDGET_H
