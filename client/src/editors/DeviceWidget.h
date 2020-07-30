#ifndef DEVICEWIDGET_H
#define DEVICEWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTreeWidgetItem>

#include "DataEditorWidget.h"

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

    QMap<int, QTreeWidgetItem*>  m_listSites_items;
    QMap<int, QTreeWidgetItem*>  m_listProjects_items;
    QMap<int, QTreeWidgetItem*>  m_listDeviceProjects_items;
    QMap<int, QListWidgetItem*>  m_listParticipants_items;
    QMap<int, QListWidgetItem*>  m_listSessionTypes_items;

    void updateControlsState();
    void updateFieldsValue();
    bool validateData();

    void connectSignals();

    void updateSite(TeraData *site);
    void updateProject(TeraData * project);
    void updateParticipant(TeraData *participant);
    void updateSessionType(TeraData *session_type);

private slots:
    void processFormsReply(QString form_type, QString data);
    void processSitesReply(QList<TeraData> sites);
    void processDeviceSitesReply(QList<TeraData> device_sites);
    void processDeviceProjectsReply(QList<TeraData> device_projects);
    void processDeviceParticipantsReply(QList<TeraData> device_parts);
    void processSessionTypesReply(QList<TeraData> session_types);
    void processProjectsReply(QList<TeraData> projects);

    void btnSaveSites_clicked();

    void lstSites_itemExpanded(QTreeWidgetItem* item);
    void lstSites_itemChanged(QTreeWidgetItem* item, int column);

    void on_tabNav_currentChanged(int index);
};

#endif // DEVICEWIDGET_H
