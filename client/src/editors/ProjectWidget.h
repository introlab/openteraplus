#ifndef ProjectWidget_H
#define ProjectWidget_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTableWidgetItem>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"

namespace Ui {
class ProjectWidget;
}

class ProjectWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit ProjectWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~ProjectWidget();

    void saveData(bool signal=true);

    void setData(const TeraData* data);

private slots:
    void processFormsReply(QString form_type, QString data);
    void processProjectAccessReply(QList<TeraData> access, QUrlQuery reply_query);
    void processGroupsReply(QList<TeraData> groups);
    void processDevicesReply(QList<TeraData> devices);
    void processSessionTypesProjectReply(QList<TeraData> stps);
    void processServiceProjectRolesReply(QList<TeraData> roles);
    void processServiceProjectsReply(QList<TeraData> services);

    void processPostOKReply(QString path);

    void btnUpdateAccess_clicked();
    void btnDevices_clicked();

    void on_tabProjectInfos_currentChanged(int index);

    void on_btnUpdateServiceRoles_clicked();

private:
    Ui::ProjectWidget *ui;

    QMap<int, QTableWidgetItem*>  m_tableUsers_items;
    QMap<int, QTableWidgetItem*>  m_tableUserGroups_items;
    QMap<int, QListWidgetItem*>   m_listGroups_items;
    QMap<int, QListWidgetItem*>   m_listSessionTypes_items;
    QMap<int, QTableWidgetItem*>  m_listDevices_items;
    QMap<int, QTableWidgetItem*>  m_tableServiceRoles_items;

    QMap<int, TeraData*>          m_services;

    QDialog*                      m_diag_editor;

    void connectSignals();

    void updateUserProjectAccess(const TeraData* access);
    void updateUserGroupProjectAccess(const TeraData* access);
    void updateUserGroupServiceRole(const TeraData* role);
    void updateGroup(const TeraData* group);
    void updateDevice(const TeraData* device);
    void updateSessionType(const TeraData* st);
    void updateService(const TeraData* service);

    QMap<int, QString> getRolesForService(const int &service_id);

    void updateControlsState();
    void updateFieldsValue();
    bool validateData();
};

#endif // ProjectWidget_H
