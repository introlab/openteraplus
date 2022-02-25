#ifndef SITEWIDGET_H
#define SITEWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTableWidgetItem>

#include "DataEditorWidget.h"
#include "GlobalMessageBox.h"
#include "dialogs/BaseDialog.h"

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
    void processServiceSiteAccessReply(QList<TeraData> service_sites, QUrlQuery reply_query);
    void processStatsReply(TeraData stats, QUrlQuery reply_query);

    void updateServiceSite(const TeraData* service_site);

    void processPostOKReply(QString path);

    void btnUpdateAccess_clicked();

    void on_btnUserGroups_clicked();

    void on_icoUsers_clicked();
    void on_icoProjects_clicked();
    void on_icoDevices_clicked();

    void userGroupsEditor_finished();

    void on_tabNav_currentChanged(int index);

    void on_tabManageUsers_currentChanged(int index);

    void on_btnUpdateServices_clicked();

    void on_lstServices_itemChanged(QListWidgetItem *item);

private:
    Ui::SiteWidget *ui;

    QMap<int, QListWidgetItem*>  m_listProjects_items;
    QMap<int, QListWidgetItem*>  m_listServices_items;
    QMap<int, QListWidgetItem*>  m_listServicesSites_items;
    QMap<int, QTableWidgetItem*> m_tableUserGroups_items;

    BaseDialog*                  m_diag_editor;

    void connectSignals();

    void updateUserGroupSiteAccess(const TeraData* access);
    void updateControlsState();
    void updateFieldsValue();
    bool validateData();

    bool isSiteAdmin();

    void queryUserGroupsSiteAccess();
    void queryServiceSiteAccess();
};

#endif // SITEWIDGET_H
