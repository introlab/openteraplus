#ifndef ProjectWidget_H
#define ProjectWidget_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include <QTableWidgetItem>

#include "DataEditorWidget.h"
#include "dialogs/BaseDialog.h"

namespace Ui {
class ProjectWidget;
}

class ProjectWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit ProjectWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~ProjectWidget();

    void saveData(bool signal=true) override;

    void setData(const TeraData* data) override;

private slots:
    void processFormsReply(QString form_type, QString data);
    void processProjectAccessReply(QList<TeraData> access, QUrlQuery reply_query);
    void processParticipantsReply(QList<TeraData> participants, QUrlQuery reply_query);
    void processDevicesReply(QList<TeraData> devices);
    void processDevicesProjectsReply(QList<TeraData> devices_projects);
    void processServiceProjectsReply(QList<TeraData> services);
    void processSessionTypeProjectReply(QList<TeraData> stp_list, QUrlQuery reply_query);
    void processSessionTypeSiteReply(QList<TeraData> sts_list, QUrlQuery reply_query);
    void processTestTypeProjectReply(QList<TeraData> ttp_list, QUrlQuery reply_query);
    void processTestTypeSiteReply(QList<TeraData> tts_list, QUrlQuery reply_query);
    void processStatsReply(TeraData stats, QUrlQuery reply_query);
    void processPostOKReply(QString path);
    void deleteDataReply(QString path, int del_id);

    void btnUpdateAccess_clicked();

    void on_btnUpdateServices_clicked();
    void on_icoUsers_clicked();
    void on_icoSessions_clicked();
    void on_btnUserGroups_clicked();

    void userGroupsEditor_finished();
    void usersEditor_finished();

    void on_tableSummary_itemDoubleClicked(QTableWidgetItem *item);
    void on_lstServices_itemChanged(QListWidgetItem *item);
    void on_tabNav_currentChanged(int index);
    void on_tabManageUsers_currentChanged(int index);
    void on_tabManageServices_currentChanged(int index);

    void on_btnUpdateSessionTypes_clicked();
    void on_lstSessionTypes_itemChanged(QListWidgetItem *item);
    void on_treeDevices_itemChanged(QTreeWidgetItem *item, int column);
    void on_btnUpdateDevices_clicked();
    void on_btnNewParticipant_clicked();
    void on_btnNewGroup_clicked();
    void on_btnDelete_clicked();
    void on_tableSummary_itemSelectionChanged();
    void on_btnManageUsers_clicked();

    void on_chkShowInactive_stateChanged(int state);

    void on_lstTestTypes_itemChanged(QListWidgetItem *item);

    void on_btnUpdateTestTypes_clicked();

private:
    Ui::ProjectWidget               *ui;

    QHash<int, QTableWidgetItem*>   m_tableUsers_items;
    QMap<int,  QTableWidgetItem*>   m_tableUserGroups_items;
    QHash<int, QTableWidgetItem*>   m_tableParticipants_items;
    QHash<int, QListWidgetItem*>    m_listGroups_items;

    QHash<int, QTreeWidgetItem*>    m_treeDevices_items;
    QHash<int, QTreeWidgetItem*>    m_treeDevicesProjects_items;

    QHash<int, QListWidgetItem*>    m_listServicesProjects_items;
    QHash<int, QListWidgetItem*>    m_listServices_items;
    QHash<int, QString>             m_services_keys;

    QHash<int, QListWidgetItem*>    m_listSessionTypesProjects_items;
    QHash<int, QListWidgetItem*>    m_listSessionTypes_items;

    QHash<int, QListWidgetItem*>    m_listTestTypesProjects_items;
    QHash<int, QListWidgetItem*>    m_listTestTypes_items;

    QHash<int, QWidget*>            m_services_tabs;

    BaseDialog*                     m_diag_editor;

    bool                            m_refreshProjectParticipants;

    void connectSignals();
    void initUI();

    void updateUserProjectAccess(const TeraData* access);
    void updateUserGroupProjectAccess(const TeraData* access);
    void updateDevice(const TeraData* device);
    void updateDeviceProject(const TeraData* device_proj);
    void updateServiceProject(const TeraData* sp);
    void updateSessionTypeProject(const TeraData* stp);
    void updateSessionTypeSite(const TeraData* sts);
    void updateTestTypeProject(const TeraData* ttp);
    void updateTestTypeSite(const TeraData* tts);
    void updateParticipant(const TeraData* participant);

    void queryServicesProject();
    void querySessionTypesProject();
    void queryTestTypesProject();
    void queryUserGroupsProjectAccess();
    void queryUsers();

    void addServiceTab(const TeraData& service_project);

    void updateControlsState() override;
    void updateFieldsValue() override;
    bool validateData() override;

    bool isSiteAdmin();


};

#endif // ProjectWidget_H
