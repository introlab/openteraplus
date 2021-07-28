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

    void selectItem(const TeraDataTypes& data_type, const int& id);
    bool selectItemByName(const TeraDataTypes& data_type, const QString& name);
    bool selectItemByUuid(const TeraDataTypes& data_type, const QString& uuid);
    void removeItem(const TeraDataTypes& data_type, const int& id);

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

    QMap<int, QTreeWidgetItem*> m_projects_items;
    QMap<int, QTreeWidgetItem*> m_groups_items;
    QMap<int, QTreeWidgetItem*> m_participants_items;

    QMap<QString, TeraData>     m_participants; // UUID - TeraData mapping for participants, required for online status

    void updateSite(const TeraData* site);
    void updateProject(const TeraData* project);
    void updateGroup(const TeraData* group);
    void updateParticipant(const TeraData* participant);

    int getParticipantProjectId(QTreeWidgetItem *part_item);
    int getParticipantGroupId(QTreeWidgetItem *part_item);

    bool isParticipantFiltered(const QString &part_uuid);

    void updateAvailableActions(QTreeWidgetItem *current_item);
    TeraDataTypes getItemType(QTreeWidgetItem* item);

    QString getParticipantUuid(int participant_id);

    void connectSignals();

    void clearData(bool clear_state);

    // Ui items
    QList<QAction*> m_newItemActions;
    QMenu*          m_newItemMenu;

    QAction* addNewItemAction(const TeraDataTypes &data_type, const QString& label);
    QAction* getActionForDataType(const TeraDataTypes &data_type);

private slots:
     void newItemRequested();
     void deleteItemRequested();
     void refreshRequested();

     void processSitesReply(QList<TeraData> sites);
     void processProjectsReply(QList<TeraData> projects);
     void processGroupsReply(QList<TeraData> groups);
     void processParticipantsReply(QList<TeraData> participants);
     void ws_participantEvent(ParticipantEvent event);

     void processItemDeletedReply(QString path, int id);

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

signals:
     void dataDisplayRequest(TeraDataTypes data_type, int data_id);
     void dataDeleteRequest(TeraDataTypes data_type, int data_id);
     void currentSiteWasChanged(QString site_name, int site_id);

     void refreshButtonClicked();
};

#endif // PROJECTNAVIGATOR_H
