#ifndef DEVICEWIDGET_H
#define DEVICEWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTreeWidgetItem>

#include "DataEditorWidget.h"
#include "ServiceConfigWidget.h"

namespace Ui {
class DeviceWidget;
}

class DeviceWidget : public DataEditorWidget
{
    Q_OBJECT

public:
    explicit DeviceWidget(ComManager* comMan, const TeraData* data = nullptr, QWidget *parent = nullptr);
    ~DeviceWidget();

    void saveData(bool signal=true);

private:
    Ui::DeviceWidget *ui;

    QHash<int, QTreeWidgetItem*>  m_treeSites_items;
    QHash<int, QTreeWidgetItem*>  m_treeProjects_items;
    QHash<int, QListWidgetItem*>  m_listParticipants_items;

    QList<int> m_deviceSites;       // Sites that this device is part of
    QList<int> m_devicesProjects;   // Projects that this device is part of

    void updateControlsState();
    void updateFieldsValue();
    bool validateData();
    bool validateSitesProjects();

    void connectSignals();
    void initUI();

    void updateSite(TeraData *site);
    void updateProject(TeraData * project);
    void updateDeviceProject(TeraData* device_project);
    void updateDeviceSite(TeraData* device_site);
    void updateParticipant(TeraData *participant);

    void postDeviceSites();
    void postDeviceProjects();

    QJsonArray getSelectedProjectsAsJsonArray();
    QJsonArray getSelectedSitesAsJsonArray();

    void queryDeviceAccess();

private slots:
    void processFormsReply(QString form_type, QString data);
    void processSitesReply(QList<TeraData> sites);
    void processDeviceSitesReply(QList<TeraData> device_sites);
    void processDeviceProjectsReply(QList<TeraData> device_projects);
    void processDeviceParticipantsReply(QList<TeraData> device_parts);
    void processProjectsReply(QList<TeraData> projects);

    void btnSaveSites_clicked();

    void deleteDataReply(QString path, int id);
    void postResultReply(QString path);

    void lstSites_itemChanged(QTreeWidgetItem* item, int column);

    void on_tabNav_currentChanged(int index);
    void on_lstParticipants_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_btnRemoveParticipant_clicked();
    void on_treeSites_itemChanged(QTreeWidgetItem *item, int column);
};

#endif // DEVICEWIDGET_H
