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
    void processGroupsReply(QList<TeraData> groups);
    void processDevicesReply(QList<TeraData> devices);
    void processServiceProjectsReply(QList<TeraData> services);
    void processStatsReply(TeraData stats, QUrlQuery reply_query);

    void processPostOKReply(QString path);

    void deleteDataReply(QString path, int del_id);

    void btnUpdateAccess_clicked();

    void on_tabProjectInfos_currentChanged(int index);

    void on_btnUpdateServices_clicked();


    void on_icoUsers_clicked();

    void on_icoSessions_clicked();

    void on_btnUserGroups_clicked();
    void userGroupsEditor_finished();

    void on_tableSummary_itemDoubleClicked(QTableWidgetItem *item);

private:
    Ui::ProjectWidget *ui;

    QMap<int, QTableWidgetItem*>  m_tableUsers_items;
    QMap<int, QTableWidgetItem*>  m_tableUserGroups_items;
    QMap<int, QTableWidgetItem*>  m_tableParticipants_items;
    QMap<int, QListWidgetItem*>   m_listGroups_items;
    QMap<int, QTableWidgetItem*>  m_listDevices_items;

    QMap<int, QListWidgetItem*>   m_listServicesProjects_items;
    QMap<int, QListWidgetItem*>   m_listServices_items;

    BaseDialog*                   m_diag_editor;

    void connectSignals();
    void initUI();

    void updateUserProjectAccess(const TeraData* access);
    void updateUserGroupProjectAccess(const TeraData* access);
    void updateGroup(const TeraData* group);
    void updateDevice(const TeraData* device);
    void updateServiceProject(const TeraData* sp);

    void updateControlsState() override;
    void updateFieldsValue() override;
    bool validateData() override;

    bool isSiteAdmin();

    void queryUserGroupsProjectAccess();
};

#endif // ProjectWidget_H
