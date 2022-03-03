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
    void processDeviceSiteAccessReply(QList<TeraData>device_sites, QUrlQuery reply_query);
    void processStatsReply(TeraData stats, QUrlQuery reply_query);   

    void processPostOKReply(QString path);

    void btnUpdateAccess_clicked();

    void on_btnUserGroups_clicked();
    void on_icoUsers_clicked();
    void on_icoDevices_clicked();

    void userGroupsEditor_finished();
    void devicesEditor_finished();

    void on_tabNav_currentChanged(int index);
    void on_tabManageUsers_currentChanged(int index);
    void on_btnUpdateServices_clicked();
    void on_lstServices_itemChanged(QListWidgetItem *item);
    void on_lstDevices_itemChanged(QListWidgetItem *item);
    void on_btnUpdateDevices_clicked();
    void on_txtSearchDevices_textChanged(const QString &search_text);
    void on_btnEditDevices_clicked();

private:
    Ui::SiteWidget *ui;

    QMap<int, QListWidgetItem*>  m_listProjects_items;
    QMap<int, QListWidgetItem*>  m_listServices_items;
    QMap<int, QListWidgetItem*>  m_listServicesSites_items;
    QMap<int, QTableWidgetItem*> m_tableUserGroups_items;

    QMap<int, QListWidgetItem*>  m_listDevicesSites_items;
    QMap<int, QListWidgetItem*>  m_listDevices_items;

    BaseDialog*                  m_diag_editor;

    int                          m_devicesCount;

    void connectSignals();

    void updateUserGroupSiteAccess(const TeraData* access);
    void updateServiceSite(const TeraData* service_site);
    void updateDeviceSite(const TeraData* device_site);

    void updateControlsState();
    void updateFieldsValue();
    bool validateData();

    bool isSiteAdmin();

    void queryUserGroupsSiteAccess();
    void queryServiceSiteAccess();
    void queryDeviceSiteAccess();
};

#endif // SITEWIDGET_H
