#ifndef ProjectWidget_H
#define ProjectWidget_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTableWidgetItem>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"
#include "dialogs/BaseDialog.h"

#include "widgets/TableDateWidgetItem.h"
#include "widgets/TableNumberWidgetItem.h"

// Service specific config widgets
#include "services/DanceService/DanceConfigWidget.h"

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
    void processDevicesReply(QList<TeraData> devices);
    void processServiceProjectsReply(QList<TeraData> services);
    void processSessionTypeProjectReply(QList<TeraData> stp_list, QUrlQuery reply_query);
    void processSessionTypeSiteReply(QList<TeraData> sts_list, QUrlQuery reply_query);
    void processStatsReply(TeraData stats, QUrlQuery reply_query);

    void processPostOKReply(QString path);

    void deleteDataReply(QString path, int del_id);

    void btnUpdateAccess_clicked();

    void on_btnUpdateServices_clicked();
    void on_icoUsers_clicked();
    void on_icoSessions_clicked();
    void on_btnUserGroups_clicked();
    void userGroupsEditor_finished();
    void on_tableSummary_itemDoubleClicked(QTableWidgetItem *item);

    void on_lstServices_itemChanged(QListWidgetItem *item);

    void on_tabNav_currentChanged(int index);
    void on_tabManageUsers_currentChanged(int index);
    void on_tabManageServices_currentChanged(int index);

    void on_btnUpdateSessionTypes_clicked();

    void on_lstSessionTypes_itemChanged(QListWidgetItem *item);

private:
    Ui::ProjectWidget *ui;

    QMap<int, QTableWidgetItem*>  m_tableUsers_items;
    QMap<int, QTableWidgetItem*>  m_tableUserGroups_items;
    QMap<int, QTableWidgetItem*>  m_tableParticipants_items;
    QMap<int, QListWidgetItem*>   m_listGroups_items;
    QMap<int, QTableWidgetItem*>  m_listDevices_items;

    QMap<int, QListWidgetItem*>   m_listServicesProjects_items;
    QMap<int, QListWidgetItem*>   m_listServices_items;
    QMap<int, QString>            m_services_keys;

    QMap<int, QListWidgetItem*>   m_listSessionTypesProjects_items;
    QMap<int, QListWidgetItem*>   m_listSessionTypes_items;

    QMap<int, QWidget*>           m_services_tabs;

    BaseDialog*                   m_diag_editor;

    void connectSignals();
    void initUI();

    void updateUserProjectAccess(const TeraData* access);
    void updateUserGroupProjectAccess(const TeraData* access);
    void updateDevice(const TeraData* device);
    void updateServiceProject(const TeraData* sp);
    void updateSessionTypeProject(const TeraData* stp);
    void updateSessionTypeSite(const TeraData* sts);

    void queryServicesProject();
    void querySessionTypesProject();

    void addServiceTab(const TeraData& service_project);

    void updateControlsState() override;
    void updateFieldsValue() override;
    bool validateData() override;

    bool isSiteAdmin();

    void queryUserGroupsProjectAccess();
};

#endif // ProjectWidget_H
