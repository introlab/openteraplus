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
    void processStatsReply(TeraData stats, QUrlQuery reply_query);

    void processPostOKReply(QString path);

    void btnUpdateAccess_clicked();
    void btnProjects_clicked();

    void on_tabSiteInfos_currentChanged(int index);

    void on_btnManageProjects_clicked();

    void on_btnManageDevices_clicked();

    void on_btnManageUserGroups_clicked();

    void on_btnManageUsers_clicked();

private:
    Ui::SiteWidget *ui;

    QMap<int, QListWidgetItem*>  m_listProjects_items;
    QMap<int, QTableWidgetItem*> m_tableUserGroups_items;

    BaseDialog*                  m_diag_editor;

    void connectSignals();

    void updateUserGroupSiteAccess(const TeraData* access);
    void updateProject(const TeraData* project);

    void updateControlsState();
    void updateFieldsValue();
    bool validateData();

    bool isSiteAdmin();
};

#endif // SITEWIDGET_H
