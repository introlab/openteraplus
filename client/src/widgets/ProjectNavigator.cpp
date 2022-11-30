#include "widgets/ProjectNavigator.h"
#include "GlobalMessageBox.h"
#include "ui_ProjectNavigator.h"
#include <QStyledItemDelegate>
#include "TeraSettings.h"


ProjectNavigator::ProjectNavigator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectNavigator)
{
    ui->setupUi(this);
    ui->cmbSites->setItemDelegate(new QStyledItemDelegate(ui->cmbSites));

    m_comManager = nullptr;
    m_newItemMenu = nullptr;
    m_currentSiteId = -1;
    m_currentProjectId = -1;
    m_currentGroupId = -1;
    m_currentParticipantId = -1;
    m_currentParticipantUuid.clear();
    m_selectionHold = false;
    m_siteJustChanged = false;
    m_sitesLoaded = false;
}

ProjectNavigator::~ProjectNavigator()
{
    delete ui;
    qDeleteAll(m_newItemActions);
    if (m_newItemMenu)
        delete m_newItemMenu;
}

void ProjectNavigator::setComManager(ComManager *comMan)
{
    m_comManager = comMan;

    connectSignals();

    initUi();
}

void ProjectNavigator::initUi()
{
    // Hide search frame
    ui->frameSearch->hide();

    ui->treeNavigator->setContextMenuPolicy(Qt::CustomContextMenu);

    // Initialize new items menu
    m_newItemMenu = new QMenu();
    m_newItemMenu->setMinimumWidth(100);
    //m_newItemMenu->setWindowTitle(tr("Nouveau"));
    //m_newItemMenu->setIcon(QIcon("://icons/new.png"));

    QAction* new_action = addNewItemAction(TERADATA_PROJECT, tr("Projet"));
    /*addNewItemAction(TERADATA_SITE, tr("Site"));
    new_action->setDisabled(true);
    new_action = */
    new_action->setDisabled(true);
    new_action = addNewItemAction(TERADATA_GROUP, tr("Groupe"));
    new_action->setDisabled(true);
    new_action = addNewItemAction(TERADATA_PARTICIPANT, tr("Participant"));
    new_action->setDisabled(true);
    ui->btnNewItem->setMenu(m_newItemMenu);

    // Load setting for filter inactive
    bool filter = TeraSettings::getUserSetting(m_comManager->getCurrentUser().getUuid(), SETTINGS_UI_FILTERINACTIVE).toBool();
    ui->btnFilterActive->setChecked(filter);

    // Request sites
    m_comManager->doGet(WEB_SITEINFO_PATH, QUrlQuery(WEB_QUERY_LIST));

}

int ProjectNavigator::getCurrentSiteId() const
{
    return m_currentSiteId;
}

QString ProjectNavigator::getCurrentSiteName() const
{
    return ui->cmbSites->currentText();
}

int ProjectNavigator::getCurrentProjectId() const
{
    return m_currentProjectId;
}

QString ProjectNavigator::getCurrentProjetName() const
{
    QTreeWidgetItem* proj_item = m_projects_items.value(m_currentProjectId);
    if (proj_item){
        return proj_item->text(0);
    }
    return "???";

}

int ProjectNavigator::getCurrentGroupId() const
{
    return m_currentGroupId;
}

QString ProjectNavigator::getCurrentGroupName() const
{
    QTreeWidgetItem* group_item = m_groups_items.value(m_currentGroupId);
    if (group_item){
        return group_item->text(0);
    }
    return "???";
}

int ProjectNavigator::getCurrentParticipantId() const
{
    if (m_currentParticipantId == -1){
        // Check if we have a uuid
        if (!m_currentParticipantUuid.isEmpty()){
            return m_participants[m_currentParticipantUuid].getId();
        }else{
            return -1;
        }
    }
    return m_currentParticipantId;
}

void ProjectNavigator::selectItem(const TeraDataTypes &data_type, const int &id)
{
    if (data_type == TERADATA_PROJECT){

        if (m_projects_items.contains(id)){
            setCurrentItem(m_projects_items[id]);
        }//else{
            // New item that was just added... save id for later!
            m_currentProjectId = id;
        //}
        return;
    }
    if (data_type == TERADATA_GROUP){
        if (m_groups_items.contains(id)){
            setCurrentItem(m_groups_items[id]);
        }//else{
            // New item that was just added... save id for later!
            m_currentGroupId = id;
        //}
        return;
    }
    if (data_type == TERADATA_PARTICIPANT){
        if (m_participants_items.contains(id)){
            setCurrentItem(m_participants_items[id]);
            m_currentParticipantUuid = getParticipantUuid(id);
        }else{
            m_currentParticipantId = id;
        }
        return;
    }

    if (data_type == TERADATA_SITE){
        for(int i=0; i<ui->cmbSites->count(); i++){
            if (ui->cmbSites->itemData(i).toInt() == id){
                ui->cmbSites->setCurrentIndex(i);
                m_currentSiteId = id;
                return;
            }
        }
        return;
    }


}

bool ProjectNavigator::selectItemByName(const TeraDataTypes &data_type, const QString &name)
{
    if (data_type == TERADATA_GROUP){
        foreach(QTreeWidgetItem* item, m_groups_items){
            if (item->text(0) == name){
                ui->treeNavigator->setCurrentItem(item);
                currentNavItemChanged(item, nullptr);
                return true;
            }
        }
    }

    if (data_type == TERADATA_PROJECT){
        foreach(QTreeWidgetItem* item, m_projects_items){
            if (item->text(0) == name){
                ui->treeNavigator->setCurrentItem(item);
                currentNavItemChanged(item, nullptr);
                return true;
            }
        }
    }

    if (data_type == TERADATA_PARTICIPANT){
        foreach(QTreeWidgetItem* item, m_participants_items){
            if (item->text(0) == name){
                 ui->treeNavigator->setCurrentItem(item);
                 currentNavItemChanged(item, nullptr);
                 return true;
            }
        }
    }

    return false;
}

bool ProjectNavigator::selectItemByUuid(const TeraDataTypes &data_type, const QString &uuid)
{
    if (data_type == TERADATA_PARTICIPANT){
        if (m_participants.contains(uuid)){
            QTreeWidgetItem* item = m_participants_items[m_participants[uuid].getId()];
            if (item){
                ui->treeNavigator->setCurrentItem(item);
                currentNavItemChanged(item, nullptr);
                return true;
            }
        }
        // Set current participant Uuid to select it now or when we have it
        m_currentParticipantUuid = uuid;

    }

    return false;
}

bool ProjectNavigator::selectParentItem()
{
    if (ui->treeNavigator->currentItem()){
        if (ui->treeNavigator->currentItem()->parent()){
            setCurrentItem(ui->treeNavigator->currentItem()->parent());
            currentNavItemChanged(ui->treeNavigator->currentItem(), nullptr);
            return true;
        }
    }
    return false;
}

void ProjectNavigator::removeItem(const TeraDataTypes &data_type, const int &id)
{
    //bool handled = true;
    QTreeWidgetItem* removed_item = nullptr;
    switch(data_type){
    case TERADATA_SITE:
        // Check if we have that site in the list
        for(int i=0; i<ui->cmbSites->count(); i++){
            if (ui->cmbSites->itemData(i).toInt() == id){
                ui->cmbSites->removeItem(i);
            }
        }
        break;
    case TERADATA_PROJECT:
        if (m_projects_items.contains(id)){
            for (int i=0; i<ui->treeNavigator->topLevelItemCount(); i++){
                if (ui->treeNavigator->topLevelItem(i) == m_projects_items[id]){
                    removed_item = m_projects_items[id] ; //ui->treeNavigator->takeTopLevelItem(i);
                    m_projects_items.remove(id);
                    break;
                }
            }
        }
        break;
    case TERADATA_GROUP:
    case TERADATA_PARTICIPANT:
    {
        if (data_type==TERADATA_GROUP){
            if (m_groups_items.contains(id)){
                removed_item = m_groups_items[id];
                m_groups_items.remove(id);
            }
        }
        if (data_type==TERADATA_PARTICIPANT){
            if (m_participants_items.contains(id)){
                removed_item = m_participants_items[id];
                m_participants_items.remove(id);
            }
        }
    }
        break;
    default: ;
        // Don't do anything!
       // handled = false;
    }
    /*if (handled)
        refreshRequested();*/

    if (removed_item){
        // Check to remove all childs from appropriate mappings
        for (int i=0; i<removed_item->childCount(); i++){
            removeChildItemData(removed_item->child(i));
        }

        if (removed_item->parent()){
            if (removed_item == ui->treeNavigator->currentItem()){
                // Select parent item if current item was deleted
                selectParentItem();
            }
        }
        delete removed_item;
    }
}

QTreeWidgetItem *ProjectNavigator::getCurrentItem()
{
    return ui->treeNavigator->currentItem();
}

void ProjectNavigator::setOnHold(const bool &hold)
{
    m_selectionHold = hold;
}

void ProjectNavigator::refreshCurrentItem()
{
    currentNavItemChanged(ui->treeNavigator->currentItem(), nullptr);
}

bool ProjectNavigator::hasCurrentItem()
{
    return (ui->treeNavigator->currentItem() != nullptr);
}

void ProjectNavigator::connectSignals()
{
    connect(m_comManager, &ComManager::sitesReceived, this, &ProjectNavigator::processSitesReply);
    connect(m_comManager, &ComManager::projectsReceived, this, &ProjectNavigator::processProjectsReply);
    connect(m_comManager, &ComManager::groupsReceived, this, &ProjectNavigator::processGroupsReply);
    connect(m_comManager, &ComManager::participantsReceived, this, &ProjectNavigator::processParticipantsReply);
    connect(m_comManager, &ComManager::statsReceived, this, &ProjectNavigator::processStatsReply);
    //connect(m_comManager, &ComManager::deleteResultsOK, this, &ProjectNavigator::processItemDeletedReply);
    connect(m_comManager, &ComManager::currentUserUpdated, this, &ProjectNavigator::processCurrentUserUpdated);

    connect(m_comManager->getWebSocketManager(), &WebSocketManager::participantEventReceived, this, &ProjectNavigator::ws_participantEvent);

    //void (QComboBox::* comboIndexChangedSignal)(int) = &QComboBox::currentIndexChanged;
    //connect(ui->cmbSites, comboIndexChangedSignal, this, &ProjectNavigator::currentSiteChanged);
    connect(ui->btnEditSite, &QPushButton::clicked, this, &ProjectNavigator::btnEditSite_clicked);
    //connect(ui->treeNavigator, &QTreeWidget::currentItemChanged, this, &ProjectNavigator::currentNavItemChanged);
    connect(ui->treeNavigator, &QTreeWidget::itemExpanded, this, &ProjectNavigator::navItemExpanded);
    connect(ui->treeNavigator, &QTreeWidget::itemClicked, this, &ProjectNavigator::navItemClicked);
    connect(ui->treeNavigator, &ProjectNavigatorTree::moveRequest, this, &ProjectNavigator::moveItemRequested);
    connect(ui->btnDeleteItem, &QPushButton::clicked, this, &ProjectNavigator::deleteItemRequested);
    connect(ui->btnRefresh, &QToolButton::clicked, this, &ProjectNavigator::refreshRequested);
}

void ProjectNavigator::clearData(bool clear_state)
{
    // Clear all data
    m_projects_items.clear();
    m_groups_items.clear();
    m_participants_items.clear();
    m_participants.clear();
    if (clear_state){
        m_currentProjectId = -1;
        m_currentGroupId = -1;
        m_currentParticipantId = -1;
        m_currentParticipantUuid.clear();
    }
    ui->treeNavigator->clear();
}

void ProjectNavigator::removeChildItemData(QTreeWidgetItem *item)
{
    for (int i=0; i<item->childCount(); i++){
        removeChildItemData(item->child(i));
    }

    TeraDataTypes child_type = getItemType(item);
    // Items will be deleted when removed from TreeWidget
    switch (child_type){
        case TERADATA_PROJECT:
                m_projects_items.remove(m_projects_items.key(item));
            break;
        case TERADATA_GROUP:
                m_groups_items.remove(m_groups_items.key(item));
            break;
        case TERADATA_PARTICIPANT:
                m_participants_items.remove(m_participants_items.key(item));
            break;
        default:
            break;
    }
}

void ProjectNavigator::updateSite(const TeraData *site)
{
    int index = ui->cmbSites->findData(site->getId());
    if (index>=0){
        // Site already present, update infos
        ui->cmbSites->setItemText(index, site->getName());
    }else{
        // Add the site
        ui->cmbSites->addItem(site->getName(), site->getId());
    }
}

void ProjectNavigator::updateProject(const TeraData *project)
{
    int id_project = project->getId();

    // Check if project is for the current site
    if (project->getFieldValue("id_site").toInt() != m_currentSiteId){
        // Maybe that project changed site...
        if (m_projects_items.contains(id_project)){
            // It did - remove it from list.
            delete m_projects_items[id_project];
            m_projects_items.remove(id_project);
        }
        return;
    }

    QTreeWidgetItem* item;

    if (m_projects_items.contains(id_project)){
        // Project already there
        item = m_projects_items[id_project];
    }else{
        // New project - add it.
        item = new QTreeWidgetItem();
        item->setData(0, Qt::UserRole, id_project);
        ui->treeNavigator->addTopLevelItem(item);
        item->setData(1, Qt::UserRole, QVariantHash()); // Stats data - empty here.
        if (!isAdvancedView()){
            if (project->hasFieldName("project_participant_group_count")){
                if (project->getFieldValue("project_participant_group_count").toInt() > 0){
                    item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
                }
            }
            if (project->hasFieldName("project_participant_count")){
                if (project->getFieldValue("project_participant_count").toInt() > 0){
                    item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
                }
            }
        }else{
            item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator); // Always show indicator in advanced view
        }
        m_projects_items[id_project] = item;
    }

    item->setText(0, project->getName());
    item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_PROJECT)));

    if (m_currentProjectId == id_project && m_currentProjectId >0 && !m_selectionHold){
        // Load details
        setCurrentItem(item);
        item->setExpanded(true);
    }

    //updateAvailableActions(nullptr);
}

void ProjectNavigator::updateProjectAdvanced(QTreeWidgetItem *project_item)
{
    if (!isAdvancedView())
        return; // Only in advanced view!

    QVariantHash stats = project_item->data(1, Qt::UserRole).toHash();

    // Advanced view projects always have 3 childs (some may be hidden): Participants, Users, Devices
    // If not, we must add those items!
    if (project_item->childCount() != 3){
        qDeleteAll(project_item->takeChildren());

        QTreeWidgetItem* static_item = new QTreeWidgetItem();
        static_item->setText(0, tr("Participants"));
        static_item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_PARTICIPANT)));
        static_item->setForeground(0, QColor(255, 255, 140));
        static_item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        static_item->setData(0, Qt::UserRole, TERADATA_PARTICIPANT);
        project_item->addChild(static_item);

        static_item = new QTreeWidgetItem();
        static_item->setText(0, tr("Utilisateurs"));
        static_item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_USER)));
        static_item->setForeground(0, QColor(255, 255, 140));
        static_item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        static_item->setData(0, Qt::UserRole, TERADATA_USER);
        project_item->addChild(static_item);

        static_item = new QTreeWidgetItem();
        static_item->setText(0, tr("Appareils"));
        static_item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_DEVICE)));
        static_item->setForeground(0, QColor(255, 255, 140));
        static_item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        static_item->setData(0, Qt::UserRole, TERADATA_DEVICE);
        project_item->addChild(static_item);
    }

    // Check what stats we have
    // Participants
    int count = 0;
    if (ui->btnFilterActive->isChecked()){ // We include only enabled participants
        if (stats.contains("participants_enabled_count")){
            count = stats["participants_enabled_count"].toInt();
        }
    }else{
        if (stats.contains("participants_total_count")){
           count = stats["participants_total_count"].toInt();
        }
    }
    project_item->child(0)->setText(0, tr("Participants") + " (" + QString::number(count) + ")");
    project_item->child(0)->setHidden(count <= 0);

    // Users
    count = 0;
    if (ui->btnFilterActive->isChecked()){ // We include only enabled participants
        if (stats.contains("users_enabled_count")){
            count = stats["users_enabled_count"].toInt();
        }
    }else{
        if (stats.contains("users_total_count")){
           count = stats["users_total_count"].toInt();
        }
    }
    project_item->child(1)->setText(0, tr("Utilisateurs") + " (" + QString::number(count) + ")");
    project_item->child(1)->setHidden(count <= 0);

    // Devices
    count = 0;
    if (ui->btnFilterActive->isChecked()){ // We include only enabled participants
        if (stats.contains("devices_enabled_count")){
            count = stats["devices_enabled_count"].toInt();
        }
    }else{
        if (stats.contains("devices_total_count")){
           count = stats["devices_total_count"].toInt();
        }
    }
    project_item->child(2)->setText(0, tr("Appareils") + " (" + QString::number(count) + ")");
    project_item->child(2)->setHidden(count <= 0);

}

void ProjectNavigator::updateGroup(const TeraData *group)
{
    int id_group = group->getId();
    int id_project = group->getFieldValue("id_project").toInt();

    // Check if current project exists
    /*if (!m_projects_items.contains(id_project)){
        LOG_WARNING("Project ID " + QString::number(id_project) + " not found for group " + group->getName(), "ProjectNavigator::updateGroup");
        return;
    }*/

    // Check if group is for the current project
    if (id_project != m_currentProjectId){
        // Maybe that group changed project...
        if (m_groups_items.contains(id_group)){
            if (m_groups_items[id_group] != nullptr){
                QTreeWidgetItem* project_item;
                if (!isAdvancedView())
                    project_item = m_groups_items[id_group]->parent();
                else
                    project_item = m_groups_items[id_group]->parent()->parent();
                int current_group_project = m_projects_items.key(project_item);
                if (current_group_project != id_project){
                    // It has - change group if available
                    if (m_projects_items.contains(id_project)){
                        m_groups_items[id_group]->parent()->removeChild(m_groups_items[id_group]);
                        //m_projects_items[id_project]->addChild(m_groups_items[id_group]);
                        getProjectItem(id_project, TERADATA_GROUP)->insertChild(0, m_groups_items[id_group]);
                        if (isAdvancedView()){
                            queryStatsForProject(id_project);
                            queryStatsForProject(current_group_project);

                        }
                    }else{
                        // Changed site also! Remove it completely from display
                        delete m_groups_items[id_group];
                        m_groups_items.remove(id_group);
                        return;
                    }
                }
            }
        }
    }

    QTreeWidgetItem* item = nullptr;
    if (m_groups_items.contains(id_group)){
        // group already there
        item = m_groups_items[id_group];
    }
    if (!item){
        // New group - add it.
        item = new QTreeWidgetItem();
        item->setData(0, Qt::UserRole, id_group);
        QTreeWidgetItem* project_item = getProjectItem(id_project, TERADATA_GROUP);
        //project_item->addChild(item);
        project_item->insertChild(0, item); // Groups always first
        m_groups_items[id_group] = item;
        //project_item->setExpanded(true);
    }
    if (group->hasFieldName("group_participant_count")){
        if (group->getFieldValue("group_participant_count").toInt() > 0){
            item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }
    }

    item->setText(0, group->getName());
    item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_GROUP)));

   if (m_currentGroupId == id_group && m_currentGroupId >0 && !item->isExpanded()){
        // Load groups details
       setCurrentItem(item);
       item->setExpanded(true);
    }
}

void ProjectNavigator::updateParticipant(const TeraData *participant)
{
    int id_participant = participant->getId();
    int id_group = participant->getFieldValue("id_participant_group").toInt();
    int id_project = participant->getFieldValue("id_project").toInt();

    QTreeWidgetItem* item;
    if (m_participants_items.contains(id_participant)){
        // Participant already there
        item = m_participants_items[id_participant];

        // Check if we need to change its group
        int id_part_group = m_groups_items.key(item->parent());
        if (id_part_group != id_group){
            // Participant changed group
            if (id_part_group > 0){
                // Was already in a group before
                QTreeWidgetItem* old_group = m_groups_items[id_part_group];
                if (old_group){
                    old_group->removeChild(item);
                    old_group->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicatorWhenChildless);
                }
                if(isAdvancedView()){
                    // Update counts
                    int id_part_project = m_projects_items.key(item->parent()->parent()->parent());
                    queryStatsForProject(id_part_project);
                }
            }else{
                // Participant was not in a group but now is - remove from original project
                QTreeWidgetItem* parent_item;
                if (!isAdvancedView()){
                    parent_item = item->parent();
                }else{
                    parent_item = item->parent()->parent();
                }
                int id_part_project = m_projects_items.key(parent_item);
                //QTreeWidgetItem* parent_project = m_projects_items[id_part_project];
                QTreeWidgetItem* parent_project = getProjectItem(id_part_project, TERADATA_PARTICIPANT);
                if (parent_project){
                    parent_project->removeChild(item);
                    if(isAdvancedView()){
                        // Update counts
                        queryStatsForProject(id_part_project);
                    }
                }
            }

            if (id_group > 0){
                // Participant has a group to attach to
                if (m_groups_items.contains(id_group)){
                    QTreeWidgetItem* group_item = m_groups_items[id_group];
                    if (group_item->isExpanded()){
                        m_groups_items[id_group]->addChild(item);
                    }else{
                        // Not an expanded group, delete it!
                        m_participants_items.remove(id_participant);
                        delete item;
                        group_item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
                        return;
                    }
                    if(isAdvancedView()){
                        // Update counts
                        queryStatsForProject(id_project);
                    }
                }else{
                    // Not in a displayed group, delete it
                    m_participants_items.remove(id_participant);
                    delete item;
                    if(isAdvancedView()){
                        // Update counts
                        queryStatsForProject(id_project);
                    }
                    return;
                }
            }else{
                // Participant doesn't have a group, attached to the project itself
                QTreeWidgetItem* project_item = getProjectItem(id_project, TERADATA_PARTICIPANT);//m_projects_items[id_project];
                if (project_item){
                    if (project_item->isExpanded()){
                        project_item->addChild(item);
                    }
                    if(isAdvancedView()){
                        // Update counts
                        queryStatsForProject(id_project);
                    }
                }else{
                    // Not an expanded project, delete it!
                    m_participants_items.remove(id_participant);
                    delete item;
                    if(isAdvancedView()){
                        // Update counts
                        queryStatsForProject(id_project);
                    }
                    return;
                }
            }
        }else{
            // Check if we need to change its project
            int id_part_project;
            if (!isAdvancedView())
                id_part_project = m_projects_items.key(item->parent());
            else
                id_part_project = m_projects_items.key(item->parent()->parent());

            if (id_part_project != id_project && id_part_project>0){
                QTreeWidgetItem* project_item = getProjectItem(id_project, TERADATA_PARTICIPANT);//m_projects_items[id_project];
                if (project_item){
                    if (item->parent())
                        item->parent()->removeChild(item);
                    if (project_item->isExpanded()){
                        project_item->addChild(item);
                        if(isAdvancedView()){
                            // Update counts
                            queryStatsForProject(id_part_project);
                            queryStatsForProject(id_project);
                        }
                    }else{
                        // Not an expanded project, delete it!
                        m_participants_items.remove(id_participant);
                        delete item;
                        if(isAdvancedView()){
                            // Update counts
                            queryStatsForProject(id_part_project);
                            queryStatsForProject(id_project);
                        }
                        return;
                    }
                }
            }
        }

    }else{
        // New participant - add it.
        item = new QTreeWidgetItem();
        item->setData(0, Qt::UserRole, id_participant);

        if (id_group>0){
            // Participant has a group
            QTreeWidgetItem* group_item = m_groups_items[id_group];
            if (group_item){
                // In a group currently displayed
                group_item->addChild(item);
                m_participants_items[id_participant] = item;
                if (!group_item->isExpanded())
                    group_item->setExpanded(true);
            }else{
                // No group displayed, set current variables and expand the group project
                m_currentGroupId = id_group;
                m_currentProjectId = participant->getFieldValue("id_project").toInt();
                QTreeWidgetItem* project_item = m_projects_items[m_currentProjectId];
                if (project_item){
                    if (!project_item->isExpanded()){
                        project_item->setExpanded(true);
                        if (isAdvancedView()){
                            // Expand participants in advanced view
                            getProjectItem(m_currentProjectId, TERADATA_PARTICIPANT)->setExpanded(true);
                        }
                    }
                }

                delete item;
                return;
            }
        }else{
            // Participant has no group - attach it to its project
            QTreeWidgetItem* project_item = m_projects_items[id_project];
            if (project_item){
                if (!project_item->isExpanded()){
                    project_item->setExpanded(true);
                    if (isAdvancedView()){
                        project_item->child(0)->setExpanded(true);
                    }

                }
                // Get item to attach to
                project_item = getProjectItem(id_project, TERADATA_PARTICIPANT);
                project_item->addChild(item);
                m_participants_items[id_participant] = item;
                /*if (m_currentParticipantUuid == participant->getUuid())
                    ui->treeNavigator->setCurrentItem(item);*/
            }else{
                // No project displayed, don't add to the list!
                delete item;
                return;
            }
        }
    }

    item->setText(0, participant->getName());
    if (participant->hasBusyStateField() || participant->hasOnlineStateField())
        item->setIcon(0, QIcon(participant->getIconStateFilename()));
    m_participants[participant->getUuid()] = *participant;

    // Apply filter, if needed
    bool filtered = isParticipantFiltered(participant->getUuid());
    item->setHidden(filtered);
    if (m_currentParticipantId == id_participant){
        m_currentParticipantUuid = participant->getUuid();
        m_currentParticipantId = -1;
    }
    if (m_currentParticipantUuid == participant->getUuid() && !m_currentParticipantUuid.isEmpty() && !filtered){
         // Select current participant
        setCurrentItem(item);
     }

}

void ProjectNavigator::queryParticipantsAndGroupsForProject(const int &id_project)
{
    // Project: load groups for that project
    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(id_project));
    query.addQueryItem(WEB_QUERY_LIST, "1");
    m_comManager->doGet(WEB_GROUPINFO_PATH, query);

    // Also loads participant not in a group
    query.clear();
    query.addQueryItem(WEB_QUERY_NO_GROUP,"1");
    query.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(id_project));
    //query.addQueryItem(WEB_QUERY_WITH_STATUS, "1");
    m_comManager->doGet(WEB_PARTICIPANTINFO_PATH, query);
}

void ProjectNavigator::queryStatsForProject(const int &id_project)
{
    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(id_project));
    m_comManager->doGet(WEB_STATS_PATH, query);
}

int ProjectNavigator::getParticipantProjectId(QTreeWidgetItem *part_item)
{
    QTreeWidgetItem* current_item = part_item;

    while (current_item->parent()){
        current_item = current_item->parent();
        int current_key = m_projects_items.key(current_item, -1);
        if (current_key>=0)
            return current_key;
    }
    return -1;

}

int ProjectNavigator::getParticipantGroupId(QTreeWidgetItem *part_item)
{
    QTreeWidgetItem* current_item = part_item;

    while (current_item->parent()){
        current_item = current_item->parent();
        int current_key = m_groups_items.key(current_item, -1);
        if (current_key>=0)
            return current_key;
    }
    return -1;
}

QTreeWidgetItem *ProjectNavigator::getProjectItem(const int &id_project, const TeraDataTypes &data_type)
{
    QTreeWidgetItem* project_item = m_projects_items[id_project];
    if (isAdvancedView()){
        if (project_item->childCount()>=3){
            if (data_type == TERADATA_PARTICIPANT || data_type == TERADATA_GROUP){
                project_item = project_item->child(0);
            }
            if (data_type == TERADATA_USER){
                project_item = project_item->child(1);
            }
            if (data_type == TERADATA_DEVICE){
                project_item = project_item->child(2);
            }
        }
    }
    return project_item;
}

bool ProjectNavigator::isParticipantFiltered(const QString &part_uuid)
{
    // No participant with that uuid!
    if (!m_participants.contains(part_uuid))
        return true;

    bool filtered = false;

    // Check for "active only" button
    if (ui->btnFilterActive->isChecked()){
        bool active = m_participants[part_uuid].isEnabled();
        filtered = !active;
    }

    // Check for text filtering
    if (ui->btnSearch->isChecked() && !filtered){
        filtered = !m_participants[part_uuid].getName().contains(ui->txtNavSearch->text(), Qt::CaseInsensitive);
    }

    return filtered;
}

bool ProjectNavigator::isAdvancedView()
{
    return ui->btnAdvanced->isChecked();
}

void ProjectNavigator::setCurrentItem(QTreeWidgetItem *item)
{
    ui->treeNavigator->setCurrentItem(item);
    updateAvailableActions(item);
}

void ProjectNavigator::selectItem(QTreeWidgetItem *item)
{
    TeraDataTypes data_type = getItemType(item);
    if (data_type == TERADATA_PROJECT){
        int id = m_projects_items.key(item);
        setCurrentItem(m_projects_items[id]);
        m_currentProjectId = id;
        return;
    }
    if (data_type == TERADATA_GROUP){
        int id = m_groups_items.key(item);
        setCurrentItem(m_groups_items[id]);
        m_currentGroupId = id;
        return;
    }
    if (data_type == TERADATA_PARTICIPANT){
        int id = m_participants_items.key(item);
        setCurrentItem(m_participants_items[id]);
        m_currentParticipantUuid = getParticipantUuid(id);
        return;
    }
}

void ProjectNavigator::updateAvailableActions(QTreeWidgetItem* current_item)
{
    // Get user access for current site and project
    TeraDataTypes item_type = getItemType(current_item);
    int project_id;
    if (item_type == TERADATA_PROJECT){
        project_id = m_projects_items.key(current_item);
    }else{
        project_id = m_currentProjectId;
    }
    bool is_site_admin = m_comManager->isCurrentUserSiteAdmin(m_currentSiteId);
    bool is_project_admin = m_comManager->isCurrentUserProjectAdmin(project_id);
    bool at_least_one_enabled = false;


    // New project
    //ui->btnEditSite->setVisible(is_site_admin);
    QAction* new_project = getActionForDataType(TERADATA_PROJECT);
    if (new_project){
        new_project->setEnabled(is_site_admin);
        //if (new_project->isEnabled()) at_least_one_enabled = true;
        new_project->setVisible(is_site_admin);
        if (new_project->isVisible()) at_least_one_enabled = true;
    }

    // New group
    QAction* new_group = getActionForDataType(TERADATA_GROUP);
    if (new_group){
        new_group->setEnabled(is_project_admin);
        //if (new_group->isEnabled()) at_least_one_enabled = true;
        new_group->setVisible(is_project_admin);
        if (new_group->isVisible()) at_least_one_enabled = true;
    }

    // New participant
    QAction* new_part = getActionForDataType(TERADATA_PARTICIPANT);
    if (new_part){
        bool allowed = /*is_project_admin &&*/project_id > 0 && (item_type == TERADATA_GROUP || item_type == TERADATA_PARTICIPANT || item_type == TERADATA_PROJECT);
        new_part->setEnabled(allowed);
        //if (new_part->isEnabled()) at_least_one_enabled = true;
        new_part->setVisible(allowed);
        if (new_part->isVisible())
            at_least_one_enabled = true;
    }

    ui->btnNewItem->setEnabled(at_least_one_enabled);

    // Delete button
    ui->btnDeleteItem->setEnabled(false);
    //ui->btnDeleteItem->setVisible(false);
    ui->btnDeleteItem->setVisible(is_site_admin || is_project_admin);

    if (item_type==TERADATA_PROJECT && is_site_admin){
        ui->btnDeleteItem->setEnabled(true);
        //ui->btnDeleteItem->setVisible(true);
    }
    if (item_type==TERADATA_GROUP && is_project_admin){
        ui->btnDeleteItem->setEnabled(true);
        //ui->btnDeleteItem->setVisible(true);
    }
    if (item_type==TERADATA_PARTICIPANT && is_project_admin){
        ui->btnDeleteItem->setEnabled(true);
        //ui->btnDeleteItem->setVisible(true);
    }

}

TeraDataTypes ProjectNavigator::getItemType(QTreeWidgetItem *item)
{
    if (std::find(m_projects_items.cbegin(), m_projects_items.cend(), item) != m_projects_items.cend()){
        return TERADATA_PROJECT;
    }

    if (std::find(m_groups_items.cbegin(), m_groups_items.cend(), item) != m_groups_items.cend()){
        return TERADATA_GROUP;
    }

    if (std::find(m_participants_items.cbegin(), m_participants_items.cend(), item) != m_participants_items.cend()){
        return TERADATA_PARTICIPANT;
    }

    return TERADATA_NONE;
}

QString ProjectNavigator::getParticipantUuid(int participant_id)
{
    foreach(TeraData part, m_participants){
        if (part.getId() == participant_id)
            return part.getUuid();
    }
    return QString();
}

QAction *ProjectNavigator::addNewItemAction(const TeraDataTypes &data_type, const QString &label)
{
    QIcon icon;// = QIcon(TeraData::getIconFilenameForDataType(data_type));
    if (data_type == TERADATA_PARTICIPANT)
        icon = QIcon("://icons/patient_new.png");
    if (data_type == TERADATA_PROJECT)
        icon = QIcon("://icons/project_new.png");
    if (data_type == TERADATA_GROUP)
        icon = QIcon("://icons/group_new.png");
    if (icon.isNull())
        icon = QIcon(TeraData::getIconFilenameForDataType(data_type));

    QAction* new_action = new QAction(icon, label);
    new_action->setData(data_type);
    m_newItemActions.append(new_action);

    connect(new_action, &QAction::triggered, this, &ProjectNavigator::newItemRequested);
    m_newItemMenu->addAction(new_action);

    return new_action;
}

QAction *ProjectNavigator::getActionForDataType(const TeraDataTypes &data_type)
{
    QAction* action = nullptr;

    for (int i=0; i<m_newItemActions.count(); i++){
        if (static_cast<TeraDataTypes>(m_newItemActions.at(i)->data().toInt()) == data_type){
            action = m_newItemActions.at(i);
            break;
        }
    }

    return action;
}

void ProjectNavigator::newItemRequested()
{
    QAction* action = dynamic_cast<QAction*>(QObject::sender());
    if (action){
        TeraDataTypes data_type = static_cast<TeraDataTypes>(action->data().toInt());
        emit dataDisplayRequest(data_type, 0);
    }
}

void ProjectNavigator::deleteItemRequested()
{
    if (!ui->treeNavigator->currentItem())
        return;

    GlobalMessageBox diag;
    QMessageBox::StandardButton answer = diag.showYesNo(tr("Suppression?"),
                                                        tr("Êtes-vous sûrs de vouloir supprimer """) + ui->treeNavigator->currentItem()->text(0) + """?");
    if (answer == QMessageBox::Yes){
        // We must delete!
        TeraDataTypes item_type = getItemType(ui->treeNavigator->currentItem());
        int id_todel = ui->treeNavigator->currentItem()->data(0, Qt::UserRole).toInt();
        emit dataDeleteRequest(item_type, id_todel);
    }
}

void ProjectNavigator::refreshRequested()
{
    /*if (ui->treeNavigator->currentItem()){
        currentNavItemChanged(ui->treeNavigator->currentItem(), nullptr);
    }*/
    // Clear all data
    clearData(false);

    // Query projects for that site
    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_currentSiteId));
    query.addQueryItem(WEB_QUERY_LIST, "true");
    m_comManager->doGet(WEB_PROJECTINFO_PATH, query);

    emit refreshButtonClicked();
}

void ProjectNavigator::processSitesReply(const QList<TeraData> sites)
{
    for (int i=0; i<sites.count(); i++){
        updateSite(&sites.at(i));
    }


    if (ui->cmbSites->count()==1){
        // Select the only site in the list
        ui->cmbSites->setCurrentIndex(0);
    }else{
        if (m_comManager){
            if (m_sitesLoaded == false){ // Only select the last site on first connection
                int lastSiteId = TeraSettings::getUserSetting(m_comManager->getCurrentUser().getUuid(), SETTINGS_LASTSITEID).toInt();
                if (getCurrentSiteId() != lastSiteId){
                    selectItem(TERADATA_SITE, lastSiteId);
                    //m_siteJustChanged = true;
                }
            }
        }
    }
    if (!m_sitesLoaded){
        m_sitesLoaded = true;
        currentSiteChanged(false);
    }


}

void ProjectNavigator::processProjectsReply(const QList<TeraData> projects)
{
    for (int i=0; i<projects.count(); i++){
        updateProject(&projects.at(i));
    }

    if (m_siteJustChanged){
        m_siteJustChanged = false;
        if (m_projects_items.count() == 1){
            // Select the first project since the only one in the list
            //navItemClicked(m_projects_items.first()); // Commented to display dashboard for that site instead
        }
    }
}

void ProjectNavigator::processGroupsReply(const QList<TeraData> groups)
{
    for (int i=0; i<groups.count(); i++){
        updateGroup(&groups.at(i));
    }
}

void ProjectNavigator::processParticipantsReply(const QList<TeraData> participants, const QUrlQuery reply_args)
{
    if (reply_args.hasQueryItem(WEB_QUERY_ID_SITE))
        return;

    for (int i=0; i<participants.count(); i++){
        updateParticipant(&participants.at(i));
    }
}

void ProjectNavigator::processStatsReply(TeraData stats, QUrlQuery reply_query)
{
    // Only process stats in "advanced" view
    if (!isAdvancedView())
        return;

    // Projects stats
    if (reply_query.hasQueryItem(WEB_QUERY_ID_PROJECT)){
        // Find the associated project
        int id_project = reply_query.queryItemValue(WEB_QUERY_ID_PROJECT).toInt();
        if (m_projects_items.contains(id_project)){
            QTreeWidgetItem* item = m_projects_items[id_project];

            // Save stats in item
            item->setData(1, Qt::UserRole, stats.getFieldValues());

            // Update item stats
            updateProjectAdvanced(item);
        }
    }
}

void ProjectNavigator::ws_participantEvent(ParticipantEvent event)
{
    QString part_uuid = QString::fromStdString(event.participant_uuid());

    if (!m_participants.contains(part_uuid))
        return; // Participant isn't currently displayed, so nothing to do!

    TeraData* part_data = &m_participants[part_uuid];
    int part_id = part_data->getId();
    QTreeWidgetItem* part_item = m_participants_items[part_id];

    if (!part_item){
        LOG_ERROR("Couldn't find the participant " + part_uuid + " in the TreeWidgetItem list - mismatch, possible bug...", "ProjectNavigator::ws_participantEvent");
        return; // Mismatch between saved data and list of TreeWidgetItems... Shouldn't happen!
    }

    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_CONNECTED){
        part_data->setOnline(true);
    }
    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_DISCONNECTED){
        part_data->setOnline(false);
    }
    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_JOINED_SESSION){
        part_data->setBusy(true);
    }
    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_LEFT_SESSION){
        part_data->setBusy(false);
    }
    updateParticipant(part_data);
}

void ProjectNavigator::processItemDeletedReply(QString path, int id)
{
    if (path == WEB_SITEINFO_PATH){
        //qDebug() << "Site deleted!";
        // Check if we need to remove that site from the list
        removeItem(TERADATA_SITE, id);
    }else{
        refreshRequested();
    }
}

void ProjectNavigator::moveItemRequested(QTreeWidgetItem *src, QTreeWidgetItem *target)
{
    TeraDataTypes src_type = getItemType(src);
    TeraDataTypes target_type = getItemType(target);
    QJsonDocument document;
    QJsonObject data_obj;
    QJsonObject base_obj;

    bool proceed = false;

    if (src_type == TERADATA_GROUP){
        if (target_type == TERADATA_PROJECT){
            // Change group to another project
            data_obj.insert("id_" + TeraData::getDataTypeName(src_type), m_groups_items.key(src));
            data_obj.insert("id_" + TeraData::getDataTypeName(target_type), m_projects_items.key(target));

            base_obj.insert("group", data_obj);
            document.setObject(base_obj);
        }
        proceed = true;
    }

    if (src_type == TERADATA_PARTICIPANT){
        data_obj.insert("id_" + TeraData::getDataTypeName(src_type), m_participants_items.key(src));
        if (target_type == TERADATA_GROUP){
            data_obj.insert("id_" + TeraData::getDataTypeName(target_type), m_groups_items.key(target));
            proceed = true;
        }
        if (target_type == TERADATA_PROJECT){
            data_obj.insert("id_" + TeraData::getDataTypeName(target_type), m_projects_items.key(target));
            // Clear participant group
            data_obj.insert("id_participant_group", 0);
            proceed = true;
        }
        base_obj.insert("participant", data_obj);
        document.setObject(base_obj);
    }

    if (proceed){
        m_comManager->doPost(TeraData::getPathForDataType(src_type), document.toJson());
    }
}

void ProjectNavigator::processCurrentUserUpdated()
{
    updateAvailableActions(ui->treeNavigator->currentItem());
}

void ProjectNavigator::currentSiteChanged(bool requestDisplay)
{

    if (!m_sitesLoaded)
        return;

    m_currentSiteId = ui->cmbSites->currentData().toInt();

    // Save last site in settings
    if (m_comManager){
        TeraSettings::setUserSetting(m_comManager->getCurrentUser().getUuid(), SETTINGS_LASTSITEID, m_currentSiteId);
    }

    m_siteJustChanged = true;
    //qDebug() << "Current Site Changed " << m_currentSiteId;

    // Display site
    if (requestDisplay)
        emit dataDisplayRequest(TERADATA_SITE, m_currentSiteId);

    // Clear all data
    clearData(true);

    // Update UI according to actions availables
    updateAvailableActions(nullptr);

    // Query projects for that site
    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_currentSiteId));
    query.addQueryItem(WEB_QUERY_LIST, "true");
    m_comManager->doGet(WEB_PROJECTINFO_PATH, query);

    // Emit signal
    emit currentSiteWasChanged(ui->cmbSites->currentText(), m_currentSiteId);
}

void ProjectNavigator::currentNavItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    Q_UNUSED(previous)

    if (!current)
        return;

    current->setExpanded(true); // Will call "navItemExpanded" and expands the item
    TeraDataTypes item_type = getItemType(current);

    // PROJECT
    if (item_type==TERADATA_PROJECT){
        // We have a project
        //navItemExpanded(current);
        int id = m_projects_items.key(current);
        m_currentProjectId = id;
        m_currentGroupId = -1;
        emit dataDisplayRequest(TERADATA_PROJECT, m_currentProjectId);
    }

    // PARTICIPANT GROUP
    if (item_type==TERADATA_GROUP){
        // Ensure project id is correctly set
        m_currentProjectId = m_participants_items.key(current->parent());
        // We have a participants group
        //navItemExpanded(current);
        int id = m_groups_items.key(current);
        m_currentGroupId = id;
        id = m_projects_items.key(current->parent());
        m_currentProjectId = id;
        //qDebug() << "Request to display group id: " << m_currentGroupId;
        emit dataDisplayRequest(TERADATA_GROUP, m_currentGroupId);
    }

    // PARTICIPANT
    if (item_type==TERADATA_PARTICIPANT){
        // Ensure that group and project ids are correctly set
        m_currentGroupId = getParticipantGroupId(current);
        m_currentProjectId = getParticipantProjectId(current);
        int id = m_participants_items.key(current);
        m_currentParticipantUuid = getParticipantUuid(id);
        emit dataDisplayRequest(TERADATA_PARTICIPANT, id);
    }

    // Update available actions (new items)
    updateAvailableActions(current);
}

void ProjectNavigator::navItemClicked(QTreeWidgetItem *item)
{

    currentNavItemChanged(item, nullptr);

}

void ProjectNavigator::navItemExpanded(QTreeWidgetItem *item)
{
    // PROJECT
    if (std::find(m_projects_items.cbegin(), m_projects_items.cend(), item) != m_projects_items.cend()){
        int id = m_projects_items.key(item);
        if (!isAdvancedView()){
           queryParticipantsAndGroupsForProject(id);
        }else{
            // Query stats to display static items
            queryStatsForProject(id);
        }

    }

    // PARTICIPANT GROUP
    if (std::find(m_groups_items.cbegin(), m_groups_items.cend(), item) != m_groups_items.cend()){
        // We have a participants group
        int id = m_groups_items.key(item);

        // Request participants for that group
        QUrlQuery query;
        query.addQueryItem(WEB_QUERY_ID_GROUP, QString::number(id));
        query.addQueryItem(WEB_QUERY_LIST, "true");
        //query.addQueryItem(WEB_QUERY_WITH_STATUS, "1");
        m_comManager->doGet(WEB_PARTICIPANTINFO_PATH, query);

    }

    if (isAdvancedView()){
        if (item->parent()){
            // Check if we have an aggregrator item, such as all users or participants
            TeraDataTypes parent_type = getItemType(item->parent());
            if (parent_type == TERADATA_PROJECT){
                // Parent is project, decide what to do based on the data type
                int id = m_projects_items.key(item->parent());
                TeraDataTypes item_type = static_cast<TeraDataTypes>(item->data(0, Qt::UserRole).toInt());
                switch (item_type){
                case TERADATA_PARTICIPANT:
                    queryParticipantsAndGroupsForProject(id);
                    break;
                case TERADATA_USER:
                    break;
                case TERADATA_DEVICE:
                    break;
                default:
                    break;
                }
            }
        }
    }
}

void ProjectNavigator::btnEditSite_clicked()
{
    emit dataDisplayRequest(TERADATA_SITE, m_currentSiteId);
}

void ProjectNavigator::on_btnFilterActive_toggled(bool checked)
{
    foreach(TeraData value, m_participants){
        bool filtered = isParticipantFiltered(value.getUuid());
        QTreeWidgetItem* item = m_participants_items[value.getId()];
        if (item){
            item->setHidden(filtered);
        }
    }

    // Save new setting
    TeraSettings::setUserSetting(m_comManager->getCurrentUser().getUuid(), SETTINGS_UI_FILTERINACTIVE, checked);

}

void ProjectNavigator::on_btnSearch_toggled(bool checked)
{
    ui->frameSearch->setVisible(checked);
    if (!checked)
        ui->txtNavSearch->clear();
}

void ProjectNavigator::on_txtNavSearch_textChanged(const QString &search_text)
{
    Q_UNUSED(search_text)

    // Apply filters
    //foreach(QString part_uuid, m_participants.keys()){
    foreach(TeraData data, m_participants){
        QString part_uuid = m_participants.key(data);
        QTreeWidgetItem* item = m_participants_items[m_participants[part_uuid].getId()];
        if (item)
            item->setHidden(isParticipantFiltered(part_uuid));
    }
}

void ProjectNavigator::on_cmbSites_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    currentSiteChanged(false); // Set to true to request site info display
}


void ProjectNavigator::on_btnDashboard_clicked()
{
    // Query to display dashboard (default view)
    emit dataDisplayRequest(TERADATA_NONE, 0);
}


void ProjectNavigator::on_treeNavigator_customContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem* pointed_item = ui->treeNavigator->itemAt(pos);

    if (!pointed_item){
        // No item clicked - remove everything except projects
        getActionForDataType(TERADATA_GROUP)->setVisible(false);
        getActionForDataType(TERADATA_PARTICIPANT)->setVisible(false);
    }else{
        selectItem(pointed_item);
    }

    if (m_newItemMenu)
            m_newItemMenu->exec(ui->treeNavigator->mapToGlobal(pos));

    getActionForDataType(TERADATA_GROUP)->setVisible(true);
    getActionForDataType(TERADATA_PARTICIPANT)->setVisible(true);
    updateAvailableActions(ui->treeNavigator->currentItem());

}


void ProjectNavigator::on_btnAdvanced_clicked()
{
    // Force a refresh of the data for that site
    currentSiteChanged(true);
}

