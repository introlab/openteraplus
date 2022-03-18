#ifndef USERGROUPWIDGET_H
#define USERGROUPWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTableWidgetItem>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"
#include "dialogs/BaseDialog.h"

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
    void processSitesReply(QList<TeraData> sites);
    void processProjectsReply(QList<TeraData> projects);
    void processUserUserGroupsReply(QList<TeraData> users_user_groups);
    void processPostOKReply(QString path);

    void btnUpdateSiteAccess_clicked();
    void btnUpdateProjectAccess_clicked();

    void comboSiteRole_changed(int index);
    void on_tabNav_currentChanged(int index);
    void on_btnUpdateUsers_clicked();

private:
    Ui::UserGroupWidget *ui;

    QMap<int, QTableWidgetItem*> m_tableProjects_items;
    QMultiMap<int, QTableWidgetItem*> m_tableProjectSite_items; // Map: id_site, project item
    QMap<int, QTableWidgetItem*> m_tableSites_items;

    QMap<int, QListWidgetItem*>  m_listUsersUserGroups_items;
    QMap<int, QListWidgetItem*>  m_listUsers_items;

    void connectSignals();

    void updateSiteAccess(const TeraData* access);
    void updateProjectAccess(const TeraData* access);
    void updateUserUserGroup(const TeraData* uug);

    
    void updateControlsState();
    void updateFieldsValue();
    bool validateData();
};

#endif // USERGROUPWIDGET_H
