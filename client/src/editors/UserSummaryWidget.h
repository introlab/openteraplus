#ifndef USERSUMMARYWIDGET_H
#define USERSUMMARYWIDGET_H

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
class UserSummaryWidget;
}

class UserSummaryWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit UserSummaryWidget(ComManager* comMan, const TeraData* data = nullptr, const int &id_project = -1, QWidget *parent = nullptr);
    ~UserSummaryWidget();

    void saveData(bool signal=true);

    void connectSignals();

private:
    Ui::UserSummaryWidget *ui;

    QMap<int, TeraData*>            m_ids_session_types;

    BaseDialog*                     m_diag_editor;
    SessionLobbyDialog*             m_sessionLobby;

    int m_idProject;

    bool    m_passwordJustGenerated;
    bool    m_currentUserPasswordChanged;

    void updateControlsState();
    void updateFieldsValue();
    void initUI();

    bool validateData();

private slots:
    void processFormsReply(QString form_type, QString data);
    void processSiteAccessReply(QList<TeraData> access_list, QUrlQuery reply_query);
    void processSessionTypesReply(QList<TeraData> session_types);
    void processUsersReply(QList<TeraData> users);
    void processStatsReply(TeraData stats, QUrlQuery reply_query);

    void userFormValueChanged(QWidget* widget, QVariant value);
    void userFormValueHasFocus(QWidget* widget);

    void ws_userEvent(opentera::protobuf::UserEvent event);

    void sessionLobbyStartSessionRequested();
    void sessionLobbyStartSessionCancelled();

    void on_btnNewSession_clicked();

    void on_tabNav_currentChanged(int index);
};

#endif // USERSUMMARYWIDGET_H
