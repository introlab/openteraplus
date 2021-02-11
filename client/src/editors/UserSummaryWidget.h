#ifndef USERSUMMARYWIDGET_H
#define USERSUMMARYWIDGET_H

#include <QWidget>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"
#include "dialogs/BaseDialog.h"
#include "TeraSessionStatus.h"
#include "Utils.h"
#include "ServiceConfigWidget.h"
#include "dialogs/SessionLobbyDialog.h"

namespace Ui {
class UserSummaryWidget;
}

class UserSummaryWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit UserSummaryWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~UserSummaryWidget();

    void saveData(bool signal=true);

    void connectSignals();

private:
    Ui::UserSummaryWidget *ui;

    QMap<int, TeraData*>            m_ids_session_types;

    BaseDialog*                     m_diag_editor;
    SessionLobbyDialog*             m_sessionLobby;

    void updateControlsState();
    void updateFieldsValue();
    void initUI();

    bool validateData();

private slots:
    void processSessionTypesReply(QList<TeraData> session_types);
    void ws_userEvent(UserEvent event);

    void sessionLobbyStartSessionRequested();
    void sessionLobbyStartSessionCancelled();

    void on_btnNewSession_clicked();

};

#endif // USERSUMMARYWIDGET_H
