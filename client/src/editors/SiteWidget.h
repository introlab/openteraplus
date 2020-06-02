#ifndef SITEWIDGET_H
#define SITEWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTableWidgetItem>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"
#include "BaseDialog.h"

namespace Ui {
class SiteWidget;
}

class SiteWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit SiteWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~SiteWidget();

    void saveData(bool signal=true);

    void setData(const TeraData* data);

private slots:
    void processFormsReply(QString form_type, QString data);
    void processSiteAccessReply(QList<TeraData> access, QUrlQuery reply_query);
    void processProjectsReply(QList<TeraData> projects);
    void processDevicesReply(QList<TeraData> devices);

    void processPostOKReply(QString path);

    void btnUpdateAccess_clicked();
    void btnProjects_clicked();
    void btnDevices_clicked();

    void on_tabSiteInfos_currentChanged(int index);

private:
    Ui::SiteWidget *ui;

    QMap<int, QTableWidgetItem*> m_tableUsers_items;
    QMap<int, QListWidgetItem*>  m_listProjects_items;
    QMap<int, QTableWidgetItem*> m_listDevices_items;
    QMap<int, QTableWidgetItem*> m_tableUserGroups_items;

    BaseDialog*                  m_diag_editor;

    void connectSignals();

    void updateUserSiteAccess(const TeraData* access);
    void updateUserGroupSiteAccess(const TeraData* access);
    void updateProject(const TeraData* project);
    void updateDevice(const TeraData* device);

    void updateControlsState();
    void updateFieldsValue();
    bool validateData();
};

#endif // SITEWIDGET_H
