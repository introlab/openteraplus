#ifndef SITEWIDGET_H
#define SITEWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTableWidgetItem>

#include "DataEditorWidget.h"
#include "dialogs/BaseDialog.h"

namespace Ui {
class SiteWidget;
}

class SiteWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit SiteWidget(ComManager* comMan, const TeraData* data = nullptr, const bool configMode = false, QWidget *parent = nullptr);
    ~SiteWidget();

    void saveData(bool signal=true);
    void setData(const TeraData* data);

private slots:
    void processFormsReply(QString form_type, QString data);
    void processSiteAccessReply(QList<TeraData> access, QUrlQuery reply_query);
    void processServiceSiteAccessReply(QList<TeraData> service_sites, QUrlQuery reply_query);
    void processDeviceSiteAccessReply(QList<TeraData>device_sites, QUrlQuery reply_query);
    void processSessionTypeSiteAccessReply(QList<TeraData>st_sites, QUrlQuery reply_query);
    void processTestTypeSiteAccessReply(QList<TeraData>tt_sites, QUrlQuery reply_query);
    void processStatsReply(TeraData stats, QUrlQuery reply_query);

    void siteValueHasChanged(QWidget* widget, QVariant value);

    void processPostOKReply(QString path);

    void btnUpdateAccess_clicked();

    void on_btnUserGroups_clicked();
    void on_icoUsers_clicked();
    void on_icoDevices_clicked();

    void userGroupsEditor_finished();
    void devicesEditor_finished();
    void sessionTypesEditor_finished();

    void on_tabNav_currentChanged(int index);
    void on_tabManageUsers_currentChanged(int index);
    void on_btnUpdateServices_clicked();
    void on_lstServices_itemChanged(QListWidgetItem *item);
    void on_lstDevices_itemChanged(QListWidgetItem *item);
    void on_btnUpdateDevices_clicked();
    void on_txtSearchDevices_textChanged(const QString &search_text);
    void on_btnEditDevices_clicked();

    void on_lstSessionTypes_itemChanged(QListWidgetItem *item);
    void on_btnUpdateSessionTypes_clicked();
    void on_btnEditSessionTypes_clicked();

    void on_lstTestTypes_itemChanged(QListWidgetItem *item);

    void on_btnUpdateTestTypes_clicked();

    void on_tabManageServices_currentChanged(int index);

private:
    Ui::SiteWidget *ui;

    QMap<int, QListWidgetItem*>  m_listProjects_items;
    QMap<int, QListWidgetItem*>  m_listServices_items;
    QHash<int, QString>          m_services_keys;
    QMap<int, QListWidgetItem*>  m_listServicesSites_items;
    QMap<int, QTableWidgetItem*> m_tableUserGroups_items;

    QMap<int, QListWidgetItem*>  m_listDevicesSites_items;
    QMap<int, QListWidgetItem*>  m_listDevices_items;

    QMap<int, QListWidgetItem*>  m_listSessionTypeSites_items;
    QMap<int, QListWidgetItem*>  m_listSessionTypes_items;

    QMap<int, QListWidgetItem*>  m_listTestTypeSites_items;
    QMap<int, QListWidgetItem*>  m_listTestTypes_items;

    QHash<int, QWidget*>         m_services_tabs;

    BaseDialog*                  m_diag_editor;

    int                          m_devicesCount;
    bool                         m_configMode;

    void connectSignals();

    void updateUserGroupSiteAccess(const TeraData* access);
    void updateServiceSite(const TeraData* service_site);
    void updateDeviceSite(const TeraData* device_site);
    void updateSessionTypeSite(const TeraData* st_site);
    void updateTestTypeSite(const TeraData* tt_site);

    void updateControlsState();
    void updateFieldsValue();
    bool validateData();

    void addServiceTab(const TeraData& service_site);

    bool isSiteAdmin();

    void queryUserGroupsSiteAccess();
    void queryServiceSiteAccess();
    void queryDeviceSiteAccess();
    void querySessionTypeSiteAccess();
    void queryTestTypeSiteAccess();
};

#endif // SITEWIDGET_H
