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
    void processServiceProjectsReply(QList<TeraData> services);
    void processStatsReply(TeraData stats, QUrlQuery reply_query);

    void processPostOKReply(QString path);

    void deleteDataReply(QString path, int del_id);

    void btnUpdateAccess_clicked();
    void btnDevices_clicked();

    void on_tabProjectInfos_currentChanged(int index);

    void on_btnUpdateServices_clicked();

private:
    Ui::ProjectWidget *ui;

    QMap<int, QTableWidgetItem*>  m_tableUsers_items;
    QMap<int, QTableWidgetItem*>  m_tableUserGroups_items;
    QMap<int, QTableWidgetItem*>  m_tableParticipants_items;
    QMap<int, QListWidgetItem*>   m_listGroups_items;
    QMap<int, QListWidgetItem*>   m_listSessionTypes_items;
    QMap<int, QTableWidgetItem*>  m_listDevices_items;

    QMap<int, QListWidgetItem*>   m_listServicesProjects_items;
    QMap<int, QListWidgetItem*>   m_listServices_items;

    QDialog*                      m_diag_editor;

    void connectSignals();

    void updateUserProjectAccess(const TeraData* access);
    void updateUserGroupProjectAccess(const TeraData* access);
    void updateGroup(const TeraData* group);
    void updateDevice(const TeraData* device);
    void updateSessionType(const TeraData* st);
    void updateServiceProject(const TeraData* sp);

    void updateControlsState();
    void updateFieldsValue();
    bool validateData();
};

#endif // ProjectWidget_H
