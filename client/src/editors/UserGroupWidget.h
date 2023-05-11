#ifndef USERGROUPWIDGET_H
#define USERGROUPWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTableWidgetItem>

#include "editors/DataListWidget.h"
#include "DataEditorWidget.h"

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
    void processServiceRolesReply(QList<TeraData> roles, QUrlQuery reply_query);
    void processServiceAccessReply(QList<TeraData> access, QUrlQuery reply_query);
    void processUserUserGroupsReply(QList<TeraData> users_user_groups);
    void processPostOKReply(QString path);
    void processDeleteReply(QString path, int id);

    void btnUpdateSiteAccess_clicked();
    void btnUpdateProjectAccess_clicked();

    void comboSiteRole_changed(int index);
    void comboProjectRole_changed(int index);
    void comboServiceRole_changed(int index);

    void on_tabNav_currentChanged(int index);
    void on_btnUpdateUsers_clicked();

    void on_btnUpdateServicesRoles_clicked();

private:
    Ui::UserGroupWidget *ui;

    QMap<int, QTableWidgetItem*>        m_tableProjects_items;
    QMultiMap<int, QTableWidgetItem*>   m_tableProjectSite_items; // Map: id_site, project item
    QMap<int, QTableWidgetItem*>        m_tableSites_items;
    QMap<int, QTableWidgetItem*>        m_tableServices_items;

    QMap<int, QListWidgetItem*>         m_listUsersUserGroups_items;
    QMap<int, QListWidgetItem*>         m_listUsers_items;

    void connectSignals();

    void updateSiteAccess(const TeraData* access);
    void updateProjectAccess(const TeraData* access);
    void updateServiceRole(const TeraData* access);
    void updateUserUserGroup(const TeraData* uug);

    void updateControlsState();
    void updateFieldsValue();
    bool validateData();
};

#endif // USERGROUPWIDGET_H
