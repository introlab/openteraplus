#include "widgets/ProjectNavigator.h"
#include "GlobalMessageBox.h"
#include "ui_ProjectNavigator.h"
#include <QStyledItemDelegate>


ProjectNavigator::ProjectNavigator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectNavigator)
{
    ui->setupUi(this);
    ui->cmbSites->setItemDelegate(new QStyledItemDelegate);

    m_comManager = nullptr;
    m_newItemMenu = nullptr;
    m_currentSiteId = -1;
    m_currentProjectId = -1;
    m_currentGroupId = -1;
    m_currentParticipantId = -1;
    m_currentParticipantUuid.clear();
    m_selectionHold = false;
    m_siteJustChanged = false;
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

    // Initialize new items menu
    m_newItemMenu = new QMenu();
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

    // Request sites
    m_comManager->doQuery(WEB_SITEINFO_PATH, QUrlQuery(WEB_QUERY_LIST));

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

void ProjectNavigator::selectItem(const TeraDataTypes &data_type, const int &id)
{
    if (data_type == TERADATA_PROJECT){

        if (m_projects_items.contains(id)){
            ui->treeNavigator->setCurrentItem(m_projects_items[id]);
        }//else{
            // New item that was just added... save id for later!
            m_currentProjectId = id;
        //}
        return;
    }
    if (data_type == TERADATA_GROUP){
        if (m_groups_items.contains(id)){
            ui->treeNavigator->setCurrentItem(m_groups_items[id]);
        }//else{
            // New item that was just added... save id for later!
            m_currentGroupId = id;
        //}
        return;
    }
    if (data_type == TERADATA_PARTICIPANT){
        if (m_participants_items.contains(id)){
            ui->treeNavigator->setCurrentItem(m_participants_items[id]);
            m_currentParticipantUuid = getParticipantUuid(id);
        }else{
            m_currentParticipantId = id;
        }
        return;
    }


}

bool ProjectNavigator::selectItemByName(const TeraDataTypes &data_type, const QString &name)
{
    if (data_type == TERADATA_GROUP){
        for(int i=0; i<m_groups_items.count(); i++){
            if (m_groups_items.values()[i]->text(0) == name){
                ui->treeNavigator->setCurrentItem(m_groups_items.values()[i]);
                currentNavItemChanged(m_groups_items.values()[i], nullptr);
                return true;
            }
        }
    }

    if (data_type == TERADATA_PROJECT){
        for(int i=0; i<m_projects_items.count(); i++){
            if (m_projects_items.values()[i]->text(0) == name){
                QTreeWidgetItem* item = m_projects_items.values()[i];
                ui->treeNavigator->setCurrentItem(item);
                currentNavItemChanged(item, nullptr);
                return true;
            }
        }
    }

    if (data_type == TERADATA_PARTICIPANT){
        for(int i=0; i<m_participants_items.count(); i++){
            if (m_participants_items.values()[i]->text(0) == name){
                ui->treeNavigator->setCurrentItem(m_participants_items.values()[i]);
                currentNavItemChanged(m_participants_items.values()[i], nullptr);
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

void ProjectNavigator::removeItem(const TeraDataTypes &data_type, const int &id)
{
    bool handled = true;
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
                    delete ui->treeNavigator->takeTopLevelItem(i);
                    m_projects_items.remove(id);
                    break;
                }
            }
        }
        break;
    case TERADATA_GROUP:
    case TERADATA_PARTICIPANT:
    {
        QTreeWidgetItem* item = nullptr;
        if (data_type==TERADATA_GROUP){
            if (m_groups_items.contains(id))
                item = m_groups_items[id];
        }
        if (data_type==TERADATA_PARTICIPANT){
            if (m_participants_items.contains(id))
                item = m_participants_items[id];
        }
        if (item){
            if (item->parent()){
                for (int i=0; i<item->parent()->childCount(); i++){
                    if (item->parent()->child(i) == item){
                        delete item->parent()->takeChild(i);
                        if (data_type==TERADATA_GROUP)
                            m_groups_items.remove(id);
                        if (data_type==TERADATA_PARTICIPANT){
                            //m_participants.remove(m_participants_items.key(item));
                            m_participants_items.remove(id);
                        }
                        break;
                    }
                }
            }
        }
    }
        break;
    default: ;
        // Don't do anything!
        handled = false;
    }
    if (handled)
        refreshRequested();
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
    //connect(m_comManager, &ComManager::deleteResultsOK, this, &ProjectNavigator::processItemDeletedReply);
    connect(m_comManager, &ComManager::currentUserUpdated, this, &ProjectNavigator::processCurrentUserUpdated);

    connect(m_comManager->getWebSocketManager(), &WebSocketManager::participantEventReceived, this, &ProjectNavigator::ws_participantEvent);

    void (QComboBox::* comboIndexChangedSignal)(int) = &QComboBox::currentIndexChanged;
    connect(ui->cmbSites, comboIndexChangedSignal, this, &ProjectNavigator::currentSiteChanged);
    connect(ui->btnEditSite, &QPushButton::clicked, this, &ProjectNavigator::btnEditSite_clicked);
    //connect(ui->treeNavigator, &QTreeWidget::currentItemChanged, this, &ProjectNavigator::currentNavItemChanged);
    connect(ui->treeNavigator, &QTreeWidget::itemExpanded, this, &ProjectNavigator::navItemExpanded);
    connect(ui->treeNavigator, &QTreeWidget::itemClicked, this, &ProjectNavigator::navItemClicked);
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
        m_projects_items[id_project] = item;
    }

    item->setText(0, project->getName());
    item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_PROJECT)));

    if (m_currentProjectId == id_project && m_currentProjectId >0 && !m_selectionHold){
        // Load details
        ui->treeNavigator->setCurrentItem(item);
       item->setExpanded(true);
    }

    //updateAvailableActions(nullptr);
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
                int current_group_project = m_projects_items.key(m_groups_items[id_group]->parent());
                if (current_group_project != id_project){
                    // It has - change group if available
                    if (m_projects_items.contains(id_project)){
                        m_groups_items[id_group]->parent()->removeChild(m_groups_items[id_group]);
                        m_projects_items[id_project]->addChild(m_groups_items[id_group]);
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
        QTreeWidgetItem* project_item = m_projects_items[id_project];
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
       ui->treeNavigator->setCurrentItem(item);
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
            if (id_part_group > 0){
                // Participant is not in the correct group, change it
                QTreeWidgetItem* old_group = m_groups_items[id_part_group];
                if (old_group)
                    old_group->removeChild(item);
            }else{
                // Participant was in a project before, find and remove it
                QTreeWidgetItem* parent_project = m_projects_items[id_project];
                if (parent_project)
                    parent_project->removeChild(item);
            }

            if (id_group > 0){
                // Participant has a group to attach to
                if (m_groups_items.contains(id_group)){
                    m_groups_items[id_group]->addChild(item);
                }else{
                    // Not in a displayed group, delete it
                    delete item;
                    return;
                }
            }else{
                // Participant doesn't have a group, attached to the project itself
                QTreeWidgetItem* project_item = m_projects_items[id_project];
                if (project_item){
                    if (project_item->isExpanded()){
                        project_item->addChild(item);
                    }
                }else{
                    // Not an expanded project, delete it!
                    delete item;
                    return;
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
                    /*project_item->addChild(item);
                    m_participants_items[id_participant] = item;
                }else{
                    // No project expanded, add to the list, but also query for other participants in that project
                    delete item;
                    return;*/
                }
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
        //project_item->setExpanded(true);
    }
    /*if (group->hasFieldName("group_participant_count")){
        if (group->getFieldValue("group_participant_count").toInt() > 0){
            item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }
    }*/

    item->setText(0, participant->getName());
    if (participant->hasBusyStateField() || participant->hasOnlineStateField())
        item->setIcon(0, QIcon(participant->getIconStateFilename()));
    /*else{
        item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_PARTICIPANT)));
    }*/
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
        ui->treeNavigator->setCurrentItem(item);
     }

}

int ProjectNavigator::getParticipantProjectId(QTreeWidgetItem *part_item)
{
    QTreeWidgetItem* current_item = part_item;

    while (current_item->parent()){
        current_item = current_item->parent();
        if (m_projects_items.values().contains(current_item))
            return m_projects_items.key(current_item);
    }
    return -1;

}

int ProjectNavigator::getParticipantGroupId(QTreeWidgetItem *part_item)
{
    QTreeWidgetItem* current_item = part_item;

    while (current_item->parent()){
        current_item = current_item->parent();
        if (m_groups_items.values().contains(current_item))
            return m_groups_items.key(current_item);
    }
    return -1;
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

void ProjectNavigator::updateAvailableActions(QTreeWidgetItem* current_item)
{
    // Get user access for current site and project
    bool is_site_admin = m_comManager->isCurrentUserSiteAdmin(m_currentSiteId);
    bool is_project_admin = m_comManager->isCurrentUserProjectAdmin(m_currentProjectId);
    bool at_least_one_enabled = false;
    TeraDataTypes item_type = getItemType(current_item);

    // New project
    //ui->btnEditSite->setVisible(is_site_admin);
    QAction* new_project = getActionForDataType(TERADATA_PROJECT);
    if (new_project){
        new_project->setEnabled(is_site_admin);
        if (new_project->isEnabled()) at_least_one_enabled = true;
    }

    // New group
    QAction* new_group = getActionForDataType(TERADATA_GROUP);
    if (new_group){
        new_group->setEnabled(is_project_admin);
        if (new_group->isEnabled()) at_least_one_enabled = true;
    }

    // New participant
    QAction* new_part = getActionForDataType(TERADATA_PARTICIPANT);
    if (new_part){
        new_part->setEnabled(/*is_project_admin &&*/ (item_type == TERADATA_GROUP || item_type == TERADATA_PARTICIPANT || item_type == TERADATA_PROJECT));
        if (new_part->isEnabled()) at_least_one_enabled = true;
    }

    ui->btnNewItem->setEnabled(at_least_one_enabled);

    // Delete button
    ui->btnDeleteItem->setEnabled(false);

    if (item_type==TERADATA_PROJECT && is_site_admin){
        ui->btnDeleteItem->setEnabled(true);
    }
    if (item_type==TERADATA_GROUP && is_project_admin){
        ui->btnDeleteItem->setEnabled(true);
    }
    if (item_type==TERADATA_PARTICIPANT && is_project_admin){
        ui->btnDeleteItem->setEnabled(true);
    }

}

TeraDataTypes ProjectNavigator::getItemType(QTreeWidgetItem *item)
{
    if (m_projects_items.values().contains(item)){
        return TERADATA_PROJECT;
    }

    if (m_groups_items.values().contains(item)){
        return TERADATA_GROUP;
    }

    if (m_participants_items.values().contains(item)){
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
    QIcon icon = QIcon(TeraData::getIconFilenameForDataType(data_type));
    if (data_type == TERADATA_PARTICIPANT)
        icon = QIcon("://icons/patient_installed.png");
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
    m_comManager->doQuery(WEB_PROJECTINFO_PATH, query);

    emit refreshButtonClicked();
}

void ProjectNavigator::processSitesReply(QList<TeraData> sites)
{
    for (int i=0; i<sites.count(); i++){
        updateSite(&sites.at(i));
    }

    if (ui->cmbSites->count()==1){
        // Select the only site in the list
        ui->cmbSites->setCurrentIndex(0);
    }
}

void ProjectNavigator::processProjectsReply(QList<TeraData> projects)
{
    for (int i=0; i<projects.count(); i++){
        updateProject(&projects.at(i));
    }

    if (m_siteJustChanged){
        m_siteJustChanged = false;
        if (m_projects_items.count() == 1){
            // Select the first project since the only one in the list
            navItemClicked(m_projects_items.first());
        }
    }
}

void ProjectNavigator::processGroupsReply(QList<TeraData> groups)
{
    for (int i=0; i<groups.count(); i++){
        updateGroup(&groups.at(i));
    }
}

void ProjectNavigator::processParticipantsReply(QList<TeraData> participants)
{
    for (int i=0; i<participants.count(); i++){
        updateParticipant(&participants.at(i));
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

void ProjectNavigator::processCurrentUserUpdated()
{
    updateAvailableActions(ui->treeNavigator->currentItem());
}

void ProjectNavigator::currentSiteChanged()
{
    m_currentSiteId = ui->cmbSites->currentData().toInt();
    m_siteJustChanged = true;
    //qDebug() << "Current Site Changed";

    // Display site
    emit dataDisplayRequest(TERADATA_SITE, m_currentSiteId);

    // Clear all data
    clearData(true);

    // Update UI according to actions availables
    updateAvailableActions(nullptr);

    // Query projects for that site
    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_currentSiteId));
    query.addQueryItem(WEB_QUERY_LIST, "true");
    m_comManager->doQuery(WEB_PROJECTINFO_PATH, query);

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
    if (m_projects_items.values().contains(item)){
        // Project: load groups for that project
        int id = m_projects_items.key(item);

        QUrlQuery query;
        query.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(id));
        query.addQueryItem(WEB_QUERY_LIST, "true");
        m_comManager->doQuery(WEB_GROUPINFO_PATH, query);

        // Also loads participant not in a group
        query.clear();
        query.addQueryItem(WEB_QUERY_NO_GROUP,"true");
        query.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(id));
        //query.addQueryItem(WEB_QUERY_WITH_STATUS, "1");
        m_comManager->doQuery(WEB_PARTICIPANTINFO_PATH, query);

    }

    // PARTICIPANT GROUP
    if (m_groups_items.values().contains(item)){
        // We have a participants group
        int id = m_groups_items.key(item);

        // Request participants for that group
        QUrlQuery query;
        query.addQueryItem(WEB_QUERY_ID_GROUP, QString::number(id));
        query.addQueryItem(WEB_QUERY_LIST, "true");
        //query.addQueryItem(WEB_QUERY_WITH_STATUS, "1");
        m_comManager->doQuery(WEB_PARTICIPANTINFO_PATH, query);

    }
}

void ProjectNavigator::btnEditSite_clicked()
{
    emit dataDisplayRequest(TERADATA_SITE, m_currentSiteId);
}

void ProjectNavigator::on_btnFilterActive_toggled(bool checked)
{
    Q_UNUSED(checked)
    for (int i=0; i<m_participants.count(); i++){
        bool filtered = isParticipantFiltered(m_participants.values()[i].getUuid());
        QTreeWidgetItem* item = m_participants_items[m_participants.values()[i].getId()];
        if (item){
            item->setHidden(filtered);
        }
    }

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
    foreach(QString part_uuid, m_participants.keys()){
        QTreeWidgetItem* item = m_participants_items[m_participants[part_uuid].getId()];
        if (item)
            item->setHidden(isParticipantFiltered(part_uuid));
    }
}
