#ifndef GROUPWIDGET_H
#define GROUPWIDGET_H

#include <QWidget>

#include <QTableWidgetItem>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"

#include "widgets/TableDateWidgetItem.h"
#include "widgets/TableNumberWidgetItem.h"

#ifndef OPENTERA_WEBASSEMBLY
#include "dialogs/SessionLobbyDialog.h"
#endif

namespace Ui {
class GroupWidget;
}

class GroupWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit GroupWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~GroupWidget() override;

    void saveData(bool signal=true) override;

    void connectSignals();

private:
    Ui::GroupWidget *ui;

    QMap<int, QTableWidgetItem*>    m_tableParticipants_items;
    QList<int>            m_activeParticipants;

    QMap<int, TeraData*>            m_ids_session_types;
#ifndef OPENTERA_WEBASSEMBLY
    SessionLobbyDialog*             m_sessionLobby;
#endif
    void updateControlsState() override;
    void updateFieldsValue() override;

    void setData(const TeraData* data) override;

    bool canStartSession();

    bool validateData() override;

private slots:
    void processFormsReply(QString form_type, QString data);
    void processStatsReply(TeraData stats, QUrlQuery reply_query);
    void processSessionTypesReply(QList<TeraData> session_types);
    void postResultReply(QString path);

    void deleteDataReply(QString path, int del_id);
#ifndef OPENTERA_WEBASSEMBLY
    void showSessionLobby(const int& id_session_type, const int& id_session);
    void sessionLobbyStartSessionRequested();
    void sessionLobbyStartSessionCancelled();
    void on_btnNewSession_clicked();
#endif
    void on_btnNewParticipant_clicked();
    void on_btnDelete_clicked();
    void on_tableSummary_itemSelectionChanged();

    void on_chkShowInactive_stateChanged(int checked);
};

#endif // GROUPWIDGET_H
