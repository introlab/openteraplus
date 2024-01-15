#ifndef PROJECTNAVIGATOR_H
#define PROJECTNAVIGATOR_H

#include <QWidget>
#include <QAction>
#include <QPushButton>
#include <QMenu>

#include <QTreeWidgetItem>

#include "managers/ComManager.h"

namespace Ui {
class ProjectNavigator;
}

class ProjectNavigator : public QWidget
{
    Q_OBJECT
    friend class ProjectNavigatorTree;

public:
    explicit ProjectNavigator(QWidget *parent = nullptr);
    ~ProjectNavigator();

    void setComManager(ComManager* comMan);

    void initUi();

    int getCurrentSiteId() const;
    QString getCurrentSiteName() const;
    int getCurrentProjectId() const;
    QString getCurrentProjetName() const;
    int getCurrentGroupId() const;
    QString getCurrentGroupName() const;
    int getCurrentParticipantId() const;

    void selectItem(const TeraDataTypes& data_type, const int& id);
    bool selectItemByName(const TeraDataTypes& data_type, const QString& name);
    bool selectItemByUuid(const TeraDataTypes& data_type, const QString& uuid);
    bool selectParentItem();

    void removeItem(const TeraDataTypes& data_type, const int& id);
    QTreeWidgetItem* getCurrentItem();
    TeraDataTypes getCurrentItemType();
    int getCurrentItemId();

    void setOnHold(const bool& hold);
    void refreshCurrentItem();
    bool hasCurrentItem();

private:
    Ui::ProjectNavigator        *ui;
    ComManager*                 m_comManager;
    int                         m_currentSiteId;
    int                         m_currentProjectId;
    int                         m_currentGroupId;
    QString                     m_currentParticipantUuid;
    int                         m_currentParticipantId;

    bool                        m_selectionHold;
    bool                        m_siteJustChanged;
    bool                        m_sitesLoaded;

    QHash<int, QTreeWidgetItem*>        m_projects_items;
    QHash<int, QTreeWidgetItem*>        m_groups_items;
    QHash<int, QTreeWidgetItem*>        m_participants_items;
    QMultiHash<int, QTreeWidgetItem*>   m_users_items;
    QMultiHash<int, QTreeWidgetItem*>   m_devices_items;

    QHash<QString, TeraData>    m_participants; // UUID - TeraData mapping for participants, required for online status
    QHash<QString, TeraData>    m_users;
    QHash<QString, TeraData>    m_devices;

    void updateSite(const TeraData* site);
    void updateProject(const TeraData* project);
    void updateProjectAdvanced(QTreeWidgetItem* project_item);
    void updateGroup(const TeraData* group);
    void updateParticipant(const TeraData* participant);
    void updateUser(const TeraData* user, const int &id_project = 0);
    void updateDevice(const TeraData* device, const int &id_project = 0);

    void queryParticipantsAndGroupsForProject(const int& id_project);
    void queryUsersForProject(const int& id_project);
    void queryDevicesForProject(const int& id_project);
    void queryStatsForProject(const int& id_project);
    //void updateCountsForProject(const int& id_project);

    int getParticipantProjectId(QTreeWidgetItem *part_item);
    int getParticipantGroupId(QTreeWidgetItem *part_item);

    QTreeWidgetItem* getProjectItem(const int& id_project, const TeraDataTypes& data_type = TERADATA_NONE);
    QTreeWidgetItem* getProjectForItem(const QTreeWidgetItem *item);
    QTreeWidgetItem* getParticipantGroupForItem(const QTreeWidgetItem* item);

    bool isParticipantFiltered(const QString &part_uuid);
    bool isAdvancedView();

    void setCurrentItem(QTreeWidgetItem* item);
    void selectItem(QTreeWidgetItem* item);

    void updateAvailableActions(QTreeWidgetItem *current_item);
    TeraDataTypes getItemType(QTreeWidgetItem* item);

    QString getParticipantUuid(int participant_id);

    void connectSignals();

    void clearData(bool clear_state);

    void removeChildItemData(QTreeWidgetItem* item);

    // Ui items
    QList<QAction*> m_newItemActions;
    QMenu*          m_newItemMenu;

    QAction* addNewItemAction(const TeraDataTypes &data_type, const QString& label);
    QAction* getActionForDataType(const TeraDataTypes &data_type);
    QAction* getMenuActionForDataType(const TeraDataTypes &data_type);

private slots:
     void newItemRequested();
     void deleteItemRequested();
     void refreshRequested();

     void processSitesReply(const QList<TeraData> sites);
     void processProjectsReply(const QList<TeraData> projects);
     void processGroupsReply(const QList<TeraData> groups);
     void processParticipantsReply(const QList<TeraData> participants, const QUrlQuery reply_args);
     void processUsersReply(const QList<TeraData> users, const QUrlQuery reply_args);
     void processDevicesReply(const QList<TeraData> devices, const QUrlQuery reply_args);
     void processStatsReply(TeraData stats, QUrlQuery reply_query);
     void ws_participantEvent(opentera::protobuf::ParticipantEvent event);
     void ws_userEvent(opentera::protobuf::UserEvent event);
     void ws_deviceEvent(opentera::protobuf::DeviceEvent event);

     void processItemDeletedReply(QString path, int id);
     void moveItemRequested(QTreeWidgetItem* src, QTreeWidgetItem* target);

     void processCurrentUserUpdated();

     void currentSiteChanged(bool requestDisplay=true);
     void currentNavItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
     void navItemClicked(QTreeWidgetItem* item);
     void navItemExpanded(QTreeWidgetItem* item);
     void btnEditSite_clicked();

     void on_btnFilterActive_toggled(bool checked);
     void on_btnSearch_toggled(bool checked);
     void on_txtNavSearch_textChanged(const QString &search_text);
     void on_cmbSites_currentIndexChanged(int index);
     void on_btnDashboard_clicked();
     void on_treeNavigator_customContextMenuRequested(const QPoint &pos);
     void on_btnAdvanced_clicked();

signals:
     void dataDisplayRequest(TeraDataTypes data_type, int data_id);
     void dataDeleteRequest(TeraDataTypes data_type, int data_id);
     void currentSiteWasChanged(QString site_name, int site_id);

     void refreshButtonClicked();
};

#endif // PROJECTNAVIGATOR_H
