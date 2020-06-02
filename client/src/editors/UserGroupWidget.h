#ifndef USERGROUPWIDGET_H
#define USERGROUPWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTableWidgetItem>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"
#include "BaseDialog.h"

namespace Ui {
class UserGroupWidget;
}

class UserGroupWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit UserGroupWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~UserGroupWidget();

    void saveData(bool signal=true);
    void setData(const TeraData* data);

private slots:
    void processFormsReply(QString form_type, QString data);
    void processSiteAccessReply(QList<TeraData> access, QUrlQuery reply_query);
    void processProjectAccessReply(QList<TeraData> access, QUrlQuery reply_query);
    void processUsersReply(QList<TeraData> users, QUrlQuery reply_query);

    void btnUpdateSiteAccess_clicked();
    void btnUpdateProjectAccess_clicked();

    void on_tabUserGroupInfos_currentChanged(int index);

private:
    Ui::UserGroupWidget *ui;

    QMap<int, QTableWidgetItem*> m_tableProjects_items;
    QMap<int, QTableWidgetItem*> m_tableSites_items;
    QMap<int, QListWidgetItem*>  m_lstUsers_items;

    void connectSignals();

    void updateSiteAccess(const TeraData* access);
    void updateProjectAccess(const TeraData* access);
    void updateUser(const TeraData* user);

    
    void updateControlsState();
    void updateFieldsValue();
    bool validateData();
};

#endif // USERGROUPWIDGET_H
