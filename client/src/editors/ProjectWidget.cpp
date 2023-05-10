#include "ProjectWidget.h"
#include "ui_ProjectWidget.h"

#include "editors/DataListWidget.h"

ProjectWidget::ProjectWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::ProjectWidget)
{
    m_diag_editor = nullptr;
    m_refreshProjectParticipants = false;

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    // Limited by default
    m_limited = true;

    // Use base class to manage editing
    setEditorControls(ui->wdgProject, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    initUI();

    // Query forms definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_PROJECT));

    ui->wdgProject->setComManager(m_comManager);
    ui->wdgProject->setSectionsPosition(QTabWidget::North);

    if (data->isNew()){
        // Connect session-type site for new project
        connect(m_comManager, &ComManager::sessionTypesSitesReceived, this, &ProjectWidget::processSessionTypeSiteReply);
    }

    ProjectWidget::setData(data);

    if (!dataIsNew()){
        queryServicesProject();
    }

}

ProjectWidget::~ProjectWidget()
{
    delete ui;
}

void ProjectWidget::saveData(bool signal)
{
    // Project data
    QJsonDocument project_data = ui->wdgProject->getFormDataJson(m_data->isNew());

    if (dataIsNew()){
        // New data - also include session types
        QJsonObject base_obj = project_data.object();
        QJsonObject base_st = base_obj["project"].toObject();
        QJsonArray session_types;

        for(QListWidgetItem* item:qAsConst(m_listSessionTypes_items)){
            if (item->checkState() == Qt::Checked){
                int session_type_id = m_listSessionTypes_items.key(item);
                QJsonObject data_obj;
                data_obj.insert("id_session_type", session_type_id);
                session_types.append(data_obj);
            }
        }

        if (!session_types.isEmpty()){
            base_st.insert("sessiontypes", session_types);
            base_obj.insert("project", base_st);
            project_data.setObject(base_obj);
        }
    }

    //qDebug() << user_data.toJson();
    postDataRequest(WEB_PROJECTINFO_PATH, project_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgProject->getFormDataObject(TERADATA_PROJECT);
        if (m_data->isEnabled() && !new_data->isEnabled()){
            // Project was disabled. Also forces participants from that project refresh when we get the "Post" reply
            m_refreshProjectParticipants = true;
        }
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void ProjectWidget::setData(const TeraData *data)
{
    DataEditorWidget::setData(data);

    if (!dataIsNew()){
        // Query stats
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
        args.addQueryItem(WEB_QUERY_WITH_PARTICIPANTS, "1");
        queryDataRequest(WEB_STATS_PATH, args);

    }
}

void ProjectWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &ProjectWidget::processFormsReply);
    connect(m_comManager, &ComManager::projectAccessReceived, this, &ProjectWidget::processProjectAccessReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &ProjectWidget::processPostOKReply);
    connect(m_comManager, &ComManager::servicesProjectsReceived, this, &ProjectWidget::processServiceProjectsReply);
    connect(m_comManager, &ComManager::sessionTypesProjectsReceived, this, &ProjectWidget::processSessionTypeProjectReply);
    connect(m_comManager, &ComManager::statsReceived, this, &ProjectWidget::processStatsReply);
    connect(m_comManager, &ComManager::deleteResultsOK, this, &ProjectWidget::deleteDataReply);

    connect(ui->btnUpdateRoles, &QPushButton::clicked, this, &ProjectWidget::btnUpdateAccess_clicked);
    //connect(ui->btnDevices, &QPushButton::clicked, this, &ProjectWidget::btnDevices_clicked);

}

void ProjectWidget::initUI()
{
    // Default display
    ui->tabNav->setCurrentIndex(0);
    ui->tabManageUsers->setCurrentIndex(0);
    ui->tabManageServices->setCurrentIndex(0);

}

void ProjectWidget::updateUserProjectAccess(const TeraData *access)
{
    QTableWidgetItem* item;
    int id_user = access->getFieldValue("id_user").toInt();
    QString project_role = access->getFieldValue("project_access_role").toString();
    if (m_tableUsers_items.contains(id_user)){
        item = m_tableUsers_items[id_user];
        item->setText(DataEditorWidget::getRoleName(project_role));
    }else{
        // Not there - must add the user and role
        ui->tableUsers->setRowCount(ui->tableUsers->rowCount()+1);
        int current_row = ui->tableUsers->rowCount()-1;
        QTableWidgetItem* item = new QTableWidgetItem(QIcon(access->getIconFilenameForDataType(TERADATA_USER)),
                                                      access->getFieldValue("user_name").toString());
        if (access->hasFieldName("user_enabled")){
            if (access->getFieldValue("user_enabled").toBool()){
                item->setForeground(Qt::green);
            }else{
                item->setForeground(Qt::red);
            }
        }
        ui->tableUsers->setItem(current_row, 0, item);
        item = new QTableWidgetItem(DataEditorWidget::getRoleName(project_role));
        ui->tableUsers->setItem(current_row, 1, item);
        m_tableUsers_items.insert(id_user, item); // Store QTableWidget Item that has the role name.
    }
}

void ProjectWidget::updateUserGroupProjectAccess(const TeraData *access)
{
    QTableWidgetItem* item;
    int id_user_group = access->getFieldValue("id_user_group").toInt();
    QString project_role = access->getFieldValue("project_access_role").toString();
    QComboBox* combo_roles;
    if (m_tableUserGroups_items.contains(id_user_group)){
        item = m_tableUserGroups_items[id_user_group];
        combo_roles = dynamic_cast<QComboBox*>(ui->tableUsers->cellWidget(item->row(),1));

    }else{
        // Not there - must add the usergroup and role
        ui->tableUserGroups->setRowCount(ui->tableUserGroups->rowCount()+1);
        int current_row = ui->tableUserGroups->rowCount()-1;
        QTableWidgetItem* item = new QTableWidgetItem(QIcon(access->getIconFilenameForDataType(TERADATA_USERGROUP)),
                                                      access->getFieldValue("user_group_name").toString());
        ui->tableUserGroups->setItem(current_row, 0, item);
        combo_roles = buildRolesComboBox();
        ui->tableUserGroups->setCellWidget(current_row, 1, combo_roles);
        m_tableUserGroups_items.insert(id_user_group, item);
    }

    if (combo_roles){
        int index = -1;
        index = combo_roles->findData(project_role);
        if (index >= 0){
            combo_roles->setCurrentIndex(index);
        }else{
            combo_roles->setCurrentIndex(0);
        }
        combo_roles->setProperty("original_index", index);
        combo_roles->setEnabled(!m_limited);

        if (access->hasFieldName("project_access_inherited")){
            if (access->getFieldValue("project_access_inherited").toBool()){
                // Inherited access - disable combobox
                combo_roles->setDisabled(true);
            }
        }
    }
}

void ProjectWidget::updateDevice(const TeraData *device)
{
    int device_id = device->getId();

    // Get base item
    QTreeWidgetItem* item = m_treeDevices_items.value(device_id);
    if (!item)
        return; // No base device already in the list from devices projects!

    // Add participants to devices
    if (device->hasFieldName("device_participants")){
        QVariantList participants = device->getFieldValue("device_participants").toList();
        for(const QVariant &participant: qAsConst(participants)){
            QVariantHash part_info = participant.toHash();
            if (part_info.contains("id_project")){
                if (part_info["id_project"].toInt() != m_data->getId())
                    continue; // Filter participants not from this project
            }
            QString part_name = part_info["participant_name"].toString();
            bool already_there = false;
            for (int i=0; i<item->childCount(); i++){
                if (item->child(i)->text(0) == part_name){
                    already_there = true;
                    break;
                }
            }
            if (!already_there){
                QTreeWidgetItem* part_item = new QTreeWidgetItem();
                part_item->setText(0, part_name);
                part_item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_PARTICIPANT)));
                item->addChild(part_item);
            }
        }
    }
}

void ProjectWidget::updateDeviceProject(const TeraData *device_proj)
{
    int id_device = device_proj->getFieldValue("id_device").toInt();
    int id_device_project = device_proj->getId();

    QTreeWidgetItem* item;
    if (m_treeDevices_items.contains(id_device)){
        item = m_treeDevices_items[id_device];
    }else{
        item = new QTreeWidgetItem();
        ui->treeDevices->addTopLevelItem(item);
        m_treeDevices_items[id_device] = item;
    }

    // Update values
    QString device_name = device_proj->getFieldValue("device_name").toString();
    if (!device_name.isEmpty())
        item->setText(0, device_name);

    if (device_proj->hasFieldName("device_available")){
        if (device_proj->getFieldValue("device_available").toBool()){
            item->setIcon(0, QIcon("://icons/device.png"));
        }else{
            item->setIcon(0, QIcon("://icons/device_installed.png"));
        }
    }else{
        item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_DEVICE)));
    }


    if (device_proj->getFieldValue("id_project").toInt() == m_data->getId()){
        item->setCheckState(0, Qt::Checked);
        if (!m_treeDevicesProjects_items.contains(id_device_project)){
            m_treeDevicesProjects_items[id_device_project] = item;
        }
    }else{
        item->setCheckState(0, Qt::Unchecked);
        if (m_treeDevicesProjects_items.contains(id_device_project)){
            m_treeDevicesProjects_items.remove(id_device_project);
        }
    }
}

void ProjectWidget::updateServiceProject(const TeraData *sp)
{
    int id_service_project = sp->getId();
    int id_service = sp->getFieldValue("id_service").toInt();
    QString service_name;
    if (sp->hasFieldName("service_name"))
        service_name = sp->getFieldValue("service_name").toString();

    QListWidgetItem* item;
    if (m_listServices_items.contains(id_service)){
        item = m_listServices_items[id_service];
    }else{
        // Must create a new item
        item = new QListWidgetItem(service_name);
        item->setIcon(QIcon(TeraData::getIconFilenameForDataType(TERADATA_SERVICE)));
        ui->lstServices->addItem(item);

        m_listServices_items[id_service] = item;
    }

    if (!service_name.isEmpty())
        item->setText(service_name);

    if (sp->getFieldValue("id_project").toInt() == m_data->getId()){
        item->setCheckState(Qt::Checked);
        if (!m_listServicesProjects_items.contains(id_service_project)){
            m_listServicesProjects_items[id_service_project] = item;
        }
    }else{
        item->setCheckState(Qt::Unchecked);
        if (m_listServicesProjects_items.contains(id_service_project)){
            m_listServicesProjects_items.remove(id_service_project);
        }
    }

    if (sp->hasFieldName("service_key")){
        m_services_keys[id_service] = sp->getFieldValue("service_key").toString();
    }

    /*if (sp->hasFieldName("service_system")){
        if (sp->getFieldValue("service_system").toBool() == true){
            item->setForeground(Qt::lightGray);
            QFont item_font = item->font();
            item_font.setItalic(true);
            item->setFont(item_font);
            if (!m_comManager->isCurrentUserSuperAdmin()){
                item->setData(Qt::UserRole, true); // Lock value
            }
        }
    }*/

}

void ProjectWidget::updateSessionTypeProject(const TeraData *stp)
{
    int id_st_project = stp->getId();
    int id_session_type = stp->getFieldValue("id_session_type").toInt();
    QString st_name;
    if (stp->hasFieldName("session_type_name"))
        st_name = stp->getFieldValue("session_type_name").toString();

    QListWidgetItem* item;
    if (m_listSessionTypes_items.contains(id_session_type)){
        item = m_listSessionTypes_items[id_session_type];
    }else{
        // Must create a new item
        item = new QListWidgetItem(st_name);
        item->setIcon(QIcon(TeraData::getIconFilenameForDataType(TERADATA_SESSIONTYPE)));
        ui->lstSessionTypes->addItem(item);

        m_listSessionTypes_items[id_session_type] = item;
    }

    if (!st_name.isEmpty())
        item->setText(st_name);

    if (stp->getFieldValue("id_project").toInt() == m_data->getId()){
        item->setCheckState(Qt::Checked);
        if (!m_listSessionTypesProjects_items.contains(id_st_project)){
            m_listSessionTypesProjects_items[id_st_project] = item;
        }
    }else{
        item->setCheckState(Qt::Unchecked);
        if (m_listSessionTypesProjects_items.contains(id_st_project)){
            m_listSessionTypesProjects_items.remove(id_st_project);
        }
    }
}

void ProjectWidget::updateSessionTypeSite(const TeraData *sts)
{
    int id_session_type = sts->getFieldValue("id_session_type").toInt();
    QString st_name;
    if (sts->hasFieldName("session_type_name"))
        st_name = sts->getFieldValue("session_type_name").toString();

    QListWidgetItem* item;
    if (m_listSessionTypes_items.contains(id_session_type)){
        item = m_listSessionTypes_items[id_session_type];
    }else{
        // Must create a new item
        item = new QListWidgetItem(st_name);
        item->setIcon(QIcon(TeraData::getIconFilenameForDataType(TERADATA_SESSIONTYPE)));
        item->setCheckState(Qt::Unchecked);
        ui->lstSessionTypes->addItem(item);

        m_listSessionTypes_items[id_session_type] = item;
    }

    if (!st_name.isEmpty())
        item->setText(st_name);
}

void ProjectWidget::queryServicesProject()
{
    // Query services for this project
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
    args.addQueryItem(WEB_QUERY_WITH_SERVICES, "1");
    queryDataRequest(WEB_SERVICEPROJECTINFO_PATH, args);
}

void ProjectWidget::querySessionTypesProject()
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
    args.addQueryItem(WEB_QUERY_WITH_SESSIONTYPE, "1");
    queryDataRequest(WEB_SESSIONTYPEPROJECT_PATH, args);
}

void ProjectWidget::updateControlsState()
{   
    if (dataIsNew() && ui->tabNav->count() > 1){
        ui->grpSummary->hide();
        while (ui->tabNav->count() > 1){
            ui->tabNav->removeTab(1);
        }

        // Move projects list to first tab
        ui->tabSessionTypes->layout()->removeWidget(ui->lstSessionTypes);
        ui->tabDashboard->layout()->removeWidget(ui->frameButtons);

        QLabel* lbl = new QLabel(tr("Types de séances associés"));
        QFont labelFont;
        labelFont.setBold(true);
        labelFont.setPointSize(10);
        lbl->setFont(labelFont);

        ui->tabDashboard->layout()->addWidget(lbl);
        ui->tabDashboard->layout()->addWidget(ui->lstSessionTypes);
        ui->tabDashboard->layout()->addWidget(ui->frameButtons);

        ui->frameActions->hide(); // Can't add participant when creating new project

        // Query session types
        if (m_listSessionTypes_items.isEmpty()){
            QUrlQuery args;
            args.addQueryItem(WEB_QUERY_ID_SITE, m_data->getFieldValue("id_site").toString());
            queryDataRequest(WEB_SESSIONTYPESITE_PATH, args);
        }


    }else{
        bool is_project_admin = m_comManager->isCurrentUserProjectAdmin(m_data->getId());
        bool is_site_admin = isSiteAdmin();

        ui->tabManageServices->setTabVisible(ui->tabManageServices->indexOf(ui->tabServices), is_site_admin);

        ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabUsersDetails), is_project_admin);
        ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabDevices), is_project_admin);
        ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabSessionTypes), is_project_admin);
        ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabServicesDetails), is_site_admin || !m_services_tabs.isEmpty());

        ui->btnNewGroup->setEnabled(is_project_admin);

        ui->tabManageUsers->setTabVisible(ui->tabManageUsers->indexOf(ui->tabUserGroups), is_site_admin);

        // m_limited = true if current user isn't site admin
        ui->btnUpdateRoles->setVisible(!m_limited);
        ui->lblInherited->setVisible(!m_limited);

        ui->lstServices->setEnabled(is_site_admin);
        ui->btnUpdateServices->setVisible(is_site_admin);
        ui->btnUserGroups->setVisible(is_site_admin);

    }

}

void ProjectWidget::updateFieldsValue()
{
    if (m_data){
        ui->wdgProject->fillFormFromData(m_data->toJson());
        ui->lblTitle->setText(m_data->getName());
        ui->icoTitle->setPixmap(QPixmap(m_data->getIconStateFilename()));

        if (dataIsNew() && m_data->hasFieldName("id_site")){
            // New project - locked to that site
            ui->wdgProject->setFieldEnabled("id_site", false);
        }
    }
}

bool ProjectWidget::validateData()
{
    bool editedProjectEnabled = ui->wdgProject->getFieldValue("project_enabled").toBool();

    if (!editedProjectEnabled && m_data->getFieldValue("project_enabled").toBool()){
        // We changed from "enabled" to "disabled". User confirmation required before proceeding.
        GlobalMessageBox msg;
        GlobalMessageBox::StandardButton rval = msg.showYesNo(tr("Confirmation - désactivation"), tr("Le project sera désactivé.") + "\n\r" +
                                                              tr("Tous les participants seront aussi désactivés et les appareils associés à ceux-ci seront désassociés.") + "\n\r" +
                                                              tr("Êtes-vous sûrs de vouloir continuer?"));
        if (rval != GlobalMessageBox::Yes){
            undoButtonClicked();
            return false;
        }
    }
    return ui->wdgProject->validateFormData();
}

bool ProjectWidget::isSiteAdmin()
{
    if (m_data){
        return m_comManager->isCurrentUserSiteAdmin(m_data->getFieldValue("id_site").toInt());
    }
    else{
        return false;
    }
}

void ProjectWidget::queryUserGroupsProjectAccess()
{
    QUrlQuery args;

    args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
    args.addQueryItem(WEB_QUERY_WITH_USERGROUPS, "1"); // Includes user groups without any access
    queryDataRequest(WEB_PROJECTACCESS_PATH, args);
}

void ProjectWidget::queryUsers()
{
    QUrlQuery args;

    args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
    args.addQueryItem(WEB_QUERY_BY_USERS, "1");
    queryDataRequest(WEB_PROJECTACCESS_PATH, args);
}

void ProjectWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type == WEB_FORMS_QUERY_PROJECT){
        ui->wdgProject->buildUiFromStructure(data);
        return;
    }
}

void ProjectWidget::processProjectAccessReply(QList<TeraData> access, QUrlQuery reply_query)
{
    if (!m_data)
        return;

    // Check if that reply is for us or not.
    if (!reply_query.hasQueryItem(WEB_QUERY_ID_PROJECT))
        return;

    if (reply_query.queryItemValue(WEB_QUERY_ID_PROJECT).toInt() != m_data->getId())
        return;

    if (reply_query.hasQueryItem(WEB_QUERY_BY_USERS)){
        for (int i=0; i<access.count(); i++) {
           updateUserProjectAccess(&access.at(i));
        }
    }else{
        // User groups in reply
        for (int i=0; i<access.count(); i++) {
           updateUserGroupProjectAccess(&access.at(i));
        }
    }
}

void ProjectWidget::processDevicesReply(QList<TeraData> devices)
{
    if (!m_data)
        return;

    for (int i=0; i<devices.count(); i++){
        //if (devices.at(i).getFieldValue("id_project").toInt() == m_data->getId()){
            updateDevice(&devices.at(i));
        //}
    }

}

void ProjectWidget::processDevicesProjectsReply(QList<TeraData> devices_projects)
{
    if (!m_data)
        return;

    for (int i=0; i<devices_projects.count(); i++){
        updateDeviceProject(&devices_projects.at(i));
    }

    for (int i=0; i<ui->treeDevices->topLevelItemCount(); i++){
        // Update used list from what is checked right now
        QTreeWidgetItem* item = ui->treeDevices->topLevelItem(i);
        if (item->checkState(0) == Qt::Unchecked){
            if (std::find(m_treeDevicesProjects_items.cbegin(), m_treeDevicesProjects_items.cend(), item) != m_treeDevicesProjects_items.cend()){
                m_treeDevicesProjects_items.remove(m_treeDevicesProjects_items.key(item));
            }
        }
        // Also remove participants
        qDeleteAll(item->takeChildren());
        item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_DEVICE)));
    }

    // Query participants for each device, using device API
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
    args.addQueryItem(WEB_QUERY_WITH_PARTICIPANTS, "1");
    queryDataRequest(WEB_DEVICEINFO_PATH, args);

    // New list - no change to be done anymore!
    ui->btnUpdateDevices->setEnabled(false);
}

void ProjectWidget::processServiceProjectsReply(QList<TeraData> services_projects)
{
    //QList<int> ids_service;

    foreach(TeraData service_project, services_projects){
        updateServiceProject(&service_project);

        // Add specific services configuration tabs
        if (!dataIsNew()){ // Don't add anything if a new project
            //ids_service.append(service_project.getFieldValue("id_service").toInt());
            addServiceTab(service_project);
        }

    }

    // New list received - disable save button
    ui->btnUpdateServices->setEnabled(false);

    // Update used list from what is checked right now
    for (int i=0; i<ui->lstServices->count(); i++){
        QListWidgetItem* item = ui->lstServices->item(i);
        if (item->checkState() == Qt::Unchecked){
            if (std::find(m_listServicesProjects_items.cbegin(), m_listServicesProjects_items.cend(), item) != m_listServicesProjects_items.cend()){
                m_listServicesProjects_items.remove(m_listServicesProjects_items.key(item));
            }
        }
    }

    // Remove service tabs not present anymore
    /*if (!dataIsNew()){
        QList<QWidget*> tabs_to_del;
        for(QWidget* tab: qAsConst(m_services_tabs)){
            if (!ids_service.contains(m_services_tabs.key(tab))){
                tabs_to_del.append(tab);
            }
        }
        for(QWidget *tab: tabs_to_del){
            ui->tabNav->removeTab(ui->tabNav->indexOf(tab));
            tab->deleteLater();
            m_services_tabs.remove(m_services_tabs.key(tab));
        }
    }*/

    /*bool services_empty = m_services.isEmpty();
    foreach(TeraData service, services){
        if(service.getFieldValue("id_project").toInt() ==  m_data->getId()){
            updateService(&service);
        }
    }

    if (services_empty){
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
        args.addQueryItem(WEB_QUERY_WITH_USERGROUPS, "1"); // Includes user groups without any access
        queryDataRequest(WEB_SERVICEACCESSINFO_PATH, args);
    }*/
}

void ProjectWidget::processSessionTypeProjectReply(QList<TeraData> stp_list, QUrlQuery reply_query)
{
    foreach(TeraData st_project, stp_list){
        updateSessionTypeProject(&st_project);
    }

    // New list received - disable save button
    ui->btnUpdateSessionTypes->setEnabled(false);

    // Update used list from what is checked right now
    for (int i=0; i<ui->lstSessionTypes->count(); i++){
        QListWidgetItem* item = ui->lstSessionTypes->item(i);
        if (item->checkState() == Qt::Unchecked){
            if (std::find(m_listSessionTypesProjects_items.cbegin(), m_listSessionTypesProjects_items.cend(), item) != m_listSessionTypesProjects_items.cend()){
                m_listSessionTypesProjects_items.remove(m_listSessionTypesProjects_items.key(item));
            }
        }
    }
}

void ProjectWidget::processSessionTypeSiteReply(QList<TeraData> sts_list, QUrlQuery reply_query)
{
    foreach(TeraData st_site, sts_list){
        updateSessionTypeSite(&st_site);
    }
}

void ProjectWidget::processStatsReply(TeraData stats, QUrlQuery reply_query)
{
    // Check if stats are for us
    if (!reply_query.hasQueryItem("id_project"))
        return;

    if (reply_query.queryItemValue("id_project").toInt() != m_data->getId())
        return;

    // Fill stats information
    ui->lblUsers->setText(stats.getFieldValue("users_total_count").toString() + tr(" Utilisateurs") + "\n"
                          + stats.getFieldValue("users_enabled_count").toString() + " Utilisateurs actifs");
    ui->lblGroups->setText(stats.getFieldValue("participants_groups_count").toString() + tr(" Groupes participants"));
    ui->lblParticipant->setText(stats.getFieldValue("participants_total_count").toString() + tr(" Participants") + "\n"
                                + stats.getFieldValue("participants_enabled_count").toString() + tr(" Participants actifs"));
    ui->lblSessions->setText(stats.getFieldValue("sessions_total_count").toString() + tr(" Séances planifiées \nou réalisées"));

    // Fill participants information
    if (stats.hasFieldName("participants")){
        ui->tableSummary->clearContents();
        m_tableParticipants_items.clear();

        QVariantList parts_list = stats.getFieldValue("participants").toList();

        for(const QVariant &part:qAsConst(parts_list)){
            QVariantMap part_info = part.toMap();
            int part_id = part_info["id_participant"].toInt();

            ui->tableSummary->setRowCount(ui->tableSummary->rowCount()+1);
            int current_row = ui->tableSummary->rowCount()-1;
            QTableWidgetItem* item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_PARTICIPANT)),
                                                                part_info["participant_name"].toString());
            m_tableParticipants_items[part_id] = item;
            ui->tableSummary->setItem(current_row, 0, item);

            item = new QTableWidgetItem();
            QString status;
            if (part_info["participant_enabled"].toBool() == true){
                status = tr("Actif");
                item->setForeground(Qt::green);
            }else{
                status = tr("Inactif");
                item->setForeground(Qt::red);
            }
            item->setText(status);
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableSummary->setItem(current_row, 1, item);

            item = new TableNumberWidgetItem(part_info["participant_sessions_count"].toString());
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableSummary->setItem(current_row, 2, item);

            item = new TableDateWidgetItem(part_info["participant_first_session"].toDateTime().toLocalTime().toString("dd-MM-yyyy hh:mm:ss"));
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableSummary->setItem(current_row, 3, item);

            QDateTime last_session_datetime = part_info["participant_last_session"].toDateTime().toLocalTime();
            item = new TableDateWidgetItem(last_session_datetime.toString("dd-MM-yyyy hh:mm:ss"));
            if (part_info["participant_enabled"].toBool() == true && last_session_datetime.isValid()){
                // Set background color
                QColor back_color = TeraForm::getGradientColor(0, 7, 14, static_cast<int>(last_session_datetime.daysTo(QDateTime::currentDateTime())));
                back_color.setAlphaF(0.5);
                item->setBackground(back_color);
            }
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableSummary->setItem(current_row, 4, item);

            QString last_connect;
            QDateTime last_connect_datetime;
            if (part_info.contains("participant_last_online")){
                last_connect_datetime =  part_info["participant_last_online"].toDateTime().toLocalTime();
                if (last_connect_datetime.isValid())
                    last_connect = last_connect_datetime.toString("dd-MM-yyyy hh:mm:ss");
            }
            item = new TableDateWidgetItem(last_connect);
            item->setTextAlignment(Qt::AlignCenter);

            if (part_info["participant_enabled"].toBool() == true && last_connect_datetime.isValid()){
                // Set background color
                QColor back_color = TeraForm::getGradientColor(0, 5, 10, static_cast<int>(last_connect_datetime.daysTo(QDateTime::currentDateTime())));
                back_color.setAlphaF(0.5);
                item->setBackground(back_color);
            }
            ui->tableSummary->setItem(current_row, 5, item);
        }
        ui->tableSummary->resizeColumnsToContents();
        ui->tableSummary->sortByColumn(4, Qt::DescendingOrder); // Sort by last session date
    }
}

void ProjectWidget::processPostOKReply(QString path)
{
    if (path == WEB_PROJECTINFO_PATH){
        // Update current user access list for the newly created project
        m_comManager->doUpdateCurrentUser();
        if (m_refreshProjectParticipants){
            // Also refresh participants list to show disabled participants
            m_refreshProjectParticipants = false;
            QUrlQuery args;
            args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
            queryDataRequest(WEB_PARTICIPANTINFO_PATH, args);
        }
    }
    if (path == WEB_SESSIONTYPEPROJECT_PATH || path == WEB_TESTTYPEPROJECT_PATH){
        // Also update associated services
        queryServicesProject();
    }
}

void ProjectWidget::deleteDataReply(QString path, int del_id)
{
    if (path == WEB_SERVICEPROJECTINFO_PATH){       
        // A service-project association was deleted
        if (m_listServicesProjects_items.contains(del_id)){
            // Remove tab if needed
            int id_service =  m_listServices_items.key(m_listServicesProjects_items[del_id]);
            if (m_services_tabs.contains(id_service)){
                QWidget* tab = m_services_tabs[id_service];
                ui->tabNav->removeTab(ui->tabNav->indexOf(tab));
                tab->deleteLater();
                m_services_tabs.remove(m_services_tabs.key(tab));
            }

            m_listServicesProjects_items[del_id]->setCheckState(Qt::Unchecked);
            m_listServicesProjects_items.remove(del_id);
            ui->btnUpdateServices->setEnabled(false);

        }
    }

    if (path == WEB_PARTICIPANTINFO_PATH){
        // A participant got deleted - check if it affects the current display
        if (m_tableParticipants_items.contains(del_id)){
            ui->tableSummary->removeRow(m_tableParticipants_items[del_id]->row());
            m_tableParticipants_items.remove(del_id);
        }
    }

}

void ProjectWidget::btnUpdateAccess_clicked()
{

    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray roles;

    QMap<int, QTableWidgetItem*>::iterator i;
    //for (int i=0; i<m_tableUserGroups_items.count(); i++){
    for (i=m_tableUserGroups_items.begin(); i!=m_tableUserGroups_items.end(); i++){
        int user_group_id = i.key();
        int row = m_tableUserGroups_items[user_group_id]->row();
        QComboBox* combo_roles = dynamic_cast<QComboBox*>(ui->tableUserGroups->cellWidget(row,1));
        if (combo_roles->property("original_index").toInt() != combo_roles->currentIndex()){
            QJsonObject data_obj;
            // Ok, value was modified - must add!
            QJsonValue role = combo_roles->currentData().toString();
            data_obj.insert("id_project", m_data->getId());
            data_obj.insert("id_user_group", user_group_id);
            data_obj.insert("project_access_role", role);
            roles.append(data_obj);
            combo_roles->setProperty("original_index", combo_roles->currentIndex());
        }
    }

    if (!roles.isEmpty()){
        base_obj.insert("project_access", roles);
        document.setObject(base_obj);
        postDataRequest(WEB_PROJECTACCESS_PATH, document.toJson());
    }
}

void ProjectWidget::on_btnUpdateServices_clicked()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonObject project_obj;
    QJsonArray services_projects;
    //QList<int> todel_ids;
    bool removed_services = false;

    project_obj.insert("id_project", m_data->getId());
    for (int i=0; i<ui->lstServices->count(); i++){
        QListWidgetItem* item = ui->lstServices->item(i);
        int service_id = m_listServices_items.key(item, 0);
        //if (m_listServicesProjects_items.values().contains(item)){
        if (item->checkState() == Qt::Checked){
            // New item selected
            QJsonObject item_obj;
            item_obj.insert("id_service", service_id);
            services_projects.append(item_obj);
        }
        if (item->checkState() == Qt::Unchecked){
            // Service was unselected
            //todel_ids.append(service_id);
            if (std::find(m_listServicesProjects_items.cbegin(), m_listServicesProjects_items.cend(), item) != m_listServicesProjects_items.cend()){
                removed_services = true;
            }
        }
     }

    if (removed_services){
        GlobalMessageBox msgbox;
        int rval = msgbox.showYesNo(tr("Suppression de service associé"), tr("Au moins un service a été retiré de ce project. S'il y a des types de séances qui utilisent ce service, elles ne seront plus accessibles.\nSouhaitez-vous continuer?"));
        if (rval != GlobalMessageBox::Yes){
            return;
        }
    }

    project_obj.insert("services", services_projects);
    base_obj.insert("project", project_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_SERVICEPROJECTINFO_PATH, document.toJson());
}

void ProjectWidget::on_icoUsers_clicked()
{
    ui->tabNav->setCurrentWidget(ui->tabUsersDetails);
}

void ProjectWidget::on_icoSessions_clicked()
{
    ui->tabNav->setCurrentWidget(ui->tabSessionTypes);
}

void ProjectWidget::on_btnUserGroups_clicked()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }

    m_diag_editor = new BaseDialog(this);
    //DataListWidget* list_widget = new DataListWidget(m_comManager, TERADATA_USERGROUP, nullptr);
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
    DataListWidget* list_widget = new DataListWidget(m_comManager, TERADATA_USERGROUP, WEB_PROJECTACCESS_PATH, args, QStringList("project_access_role"), nullptr);
    list_widget->setPermissions(isSiteAdmin(), isSiteAdmin());
    list_widget->setFilterText(tr("Seuls les groupes utilisateurs ayant un accès au projet sont affichés."));
    m_diag_editor->setCentralWidget(list_widget);

    m_diag_editor->setWindowTitle(tr("Groupes Utilisateurs"));
    m_diag_editor->setMinimumSize(size().width(), 2*size().height()/3);

    connect(m_diag_editor, &BaseDialog::finished, this, &ProjectWidget::userGroupsEditor_finished);
    m_diag_editor->open();
}

void ProjectWidget::userGroupsEditor_finished()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
        m_diag_editor = nullptr;
    }

    // Refresh user groups informations
    ui->tableUserGroups->clearContents();
    ui->tableUserGroups->setRowCount(0);
    m_tableUserGroups_items.clear();
    queryUserGroupsProjectAccess();
}

void ProjectWidget::usersEditor_finished()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
        m_diag_editor = nullptr;
    }

    // Refresh user groups informations
    ui->tableUsers->clearContents();
    ui->tableUsers->setRowCount(0);
    m_tableUsers_items.clear();
    queryUsers();
}

void ProjectWidget::on_tableSummary_itemDoubleClicked(QTableWidgetItem *item)
{
    QTableWidgetItem* base_item = ui->tableSummary->item(item->row(), 0);

    int id_participant = m_tableParticipants_items.key(base_item, -1);

    if (id_participant>=0){
        emit dataDisplayRequest(TERADATA_PARTICIPANT, id_participant);
    }


}

void ProjectWidget::addServiceTab(const TeraData &service_project)
{
    int id_service = service_project.getFieldValue("id_service").toInt();
    if (m_services_tabs.contains(id_service)) // Already there
        return;

    if (service_project.getFieldValue("id_project").toInt() != m_data->getId())
        return; // Service not enabled for that project

    QString service_key = m_services_keys[id_service];//service_project.getFieldValue("service_key").toString();
    bool is_project_admin = m_comManager->isCurrentUserProjectAdmin(m_data->getId());

    // Dance Service
    if (service_key == "DanceService"){
        if (is_project_admin){
            DanceConfigWidget* wdg = new DanceConfigWidget(m_comManager, m_data->getId());
            //ui->tabNav->addTab(wdg, QIcon("://icons/service.png"), service_project.getFieldValue("service_name").toString());
            QString service_name = service_key;
            if (m_listServices_items.contains(id_service)){
                service_name = m_listServices_items[id_service]->text();
            }
            ui->tabManageServices->insertTab(0, wdg, QIcon("://icons/config.png"), service_name);
            m_services_tabs.insert(id_service, wdg);
        }
    }
}

void ProjectWidget::on_lstServices_itemChanged(QListWidgetItem *item)
{
    /*if (item->checkState() == Qt::Unchecked){
        // Check if it is uncheckable - system services can only be deselected by super admins
        if (item->data(Qt::UserRole).toBool() == true){
            item->setCheckState(Qt::Checked);
            GlobalMessageBox msgbox;
            msgbox.showWarning(tr("Impossible de retirer"), tr("Ce service a été sélectionné par un administrateur système. Impossible de le retirer du projet sans son intervention"));
            return;
        }
    }*/

    // Check for changed items
    bool has_changes = false;
    if (m_listServicesProjects_items.key(item) > 0 && item->checkState() == Qt::Unchecked){
        // Item deselected
        has_changes = true;
    }else{
        if (m_listServicesProjects_items.key(item, -1) <= 0 && item->checkState() == Qt::Checked){
            // Item selected
            has_changes = true;
        }
    }

    ui->btnUpdateServices->setEnabled(has_changes);

}


void ProjectWidget::on_tabNav_currentChanged(int index)
{
    // Load data depending on selected tab
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
    QWidget* current_tab = ui->tabNav->widget(index);

    if (current_tab == ui->tabDevices){
        // Devices
        if (m_treeDevices_items.isEmpty()){
            connect(m_comManager, &ComManager::devicesReceived, this, &ProjectWidget::processDevicesReply);           
            connect(m_comManager, &ComManager::deviceProjectsReceived, this, &ProjectWidget::processDevicesProjectsReply);
            args.addQueryItem(WEB_QUERY_WITH_DEVICES, "1");
            queryDataRequest(WEB_DEVICEPROJECTINFO_PATH, args);
        }
    }

    if (current_tab == ui->tabUsersDetails){
        // Users
        if (m_tableUsers_items.isEmpty()){
            queryUsers();
        }
    }

    if (current_tab == ui->tabSessionTypes){
        // Session types
        /*if (!ui->wdgSessionTypes->layout()){
            QHBoxLayout* layout = new QHBoxLayout();
            layout->setMargin(0);
            ui->wdgSessionTypes->setLayout(layout);
        }
        if (ui->wdgSessionTypes->layout()->count() == 0){
            args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
            DataListWidget* stlist_editor = new DataListWidget(m_comManager, TERADATA_SESSIONTYPE, WEB_SESSIONTYPEPROJECT_PATH, args, QStringList(), ui->wdgSessionTypes);
            // m_limited = true = user only, not project admin
            stlist_editor->setPermissions(!m_limited, !m_limited);
            stlist_editor->setFilterText(tr("Seuls les types de séance associés au projet sont affichés."));
            ui->wdgSessionTypes->layout()->addWidget(stlist_editor);
        }*/

        if (m_listSessionTypesProjects_items.isEmpty()){
            querySessionTypesProject();
        }
    }

    if (current_tab == ui->tabServicesDetails){
        ui->tabManageServices->setCurrentIndex(0);
    }

}


void ProjectWidget::on_tabManageUsers_currentChanged(int index)
{
    // Load data depending on selected tab
    // QUrlQuery args;
    QWidget* current_tab = ui->tabManageUsers->widget(index);

    if (current_tab == ui->tabUserGroups){
        // User groups
        if (m_tableUserGroups_items.isEmpty()){
            // Query
           queryUserGroupsProjectAccess();
        }
    }
}

void ProjectWidget::on_tabManageServices_currentChanged(int index)
{

    QWidget* current_tab = ui->tabManageServices->widget(index);
    if (current_tab == ui->tabServices){
        // Services
       if (m_listServicesProjects_items.isEmpty()){
           queryServicesProject();
       }
    }

    // Service configuration tabs

}


void ProjectWidget::on_btnUpdateSessionTypes_clicked()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonObject project_obj;
    QJsonArray st_projects;
    //bool removed_sts = false;

    project_obj.insert("id_project", m_data->getId());
    for (int i=0; i<ui->lstSessionTypes->count(); i++){
        QListWidgetItem* item = ui->lstSessionTypes->item(i);
        int session_type_id = m_listSessionTypes_items.key(item, 0);
        if (item->checkState() == Qt::Checked){
            // New item selected
            QJsonObject item_obj;
            item_obj.insert("id_session_type", session_type_id);
            st_projects.append(item_obj);
        }
        /*if (item->checkState() == Qt::Unchecked){
            // Unselected
            if (std::find(m_listSessionTypesProjects_items.cbegin(), m_listSessionTypesProjects_items.cend(), item) != m_listSessionTypesProjects_items.cend()){
                removed_sts = true;
            }
        }*/
     }

    /*if (removed_sts){
        GlobalMessageBox msgbox;
        int rval = msgbox.showYesNo(tr("Suppression de type de séance associé"), tr("Au moins un type de séance a été retiré de ce project. S'il y a des types de séances qui utilisent ce service, elles ne seront plus accessibles.\nSouhaitez-vous continuer?"));
        if (rval != GlobalMessageBox::Yes){
            return;
        }
    }*/

    project_obj.insert("sessiontypes", st_projects);
    base_obj.insert("project", project_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_SESSIONTYPEPROJECT_PATH, document.toJson());
}


void ProjectWidget::on_lstSessionTypes_itemChanged(QListWidgetItem *item)
{
    // Check for changed items
    bool has_changes = false;
    if (m_listSessionTypesProjects_items.key(item) > 0 && item->checkState() == Qt::Unchecked){
        // Item deselected
        has_changes = true;
    }else{
        if (m_listSessionTypesProjects_items.key(item, -1) <= 0 && item->checkState() == Qt::Checked){
            // Item selected
            has_changes = true;
        }
    }

    ui->btnUpdateSessionTypes->setEnabled(has_changes);
}

void ProjectWidget::on_treeDevices_itemChanged(QTreeWidgetItem *item, int column)
{
    if (column != 0)
        return;

    // Check for changed items
    bool has_changes = false;
    if (m_treeDevicesProjects_items.key(item, -1) > 0 && item->checkState(0) == Qt::Unchecked){
        // Item deselected
        has_changes = true;
    }else{
        if (m_treeDevicesProjects_items.key(item, -1) <= 0 && item->checkState(0) == Qt::Checked){
            // Item selected
            has_changes = true;
        }
    }

    ui->btnUpdateDevices->setEnabled(has_changes);
}


void ProjectWidget::on_btnUpdateDevices_clicked()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonObject device_obj;
    QJsonArray device_projects;
    bool removed_dp = false;

    device_obj.insert("id_project", m_data->getId());
    for (int i=0; i<ui->treeDevices->topLevelItemCount(); i++){
        QTreeWidgetItem* item = ui->treeDevices->topLevelItem(i);
        int device_id = m_treeDevices_items.key(item, 0);
        if (item->checkState(0) == Qt::Checked){
            QJsonObject item_obj;
            item_obj.insert("id_device", device_id);
            device_projects.append(item_obj);
        }
        if (item->checkState(0) == Qt::Unchecked && item->childCount()>0){
            // Unselected
            if (std::find(m_treeDevicesProjects_items.cbegin(), m_treeDevicesProjects_items.cend(), item) != m_treeDevicesProjects_items.cend()){
                removed_dp = true;
            }
        }
     }

    if (removed_dp){
        GlobalMessageBox msgbox;
        int rval = msgbox.showYesNo(tr("Suppression d'appareil associé"), tr("Au moins un appareil associé à un / des participants a été retiré de ce project.\nCes participants ne pourront plus utiliser cet appareil.\nSouhaitez-vous continuer?"));
        if (rval != GlobalMessageBox::Yes){
            return;
        }
    }

    device_obj.insert("devices", device_projects);
    base_obj.insert("project", device_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_DEVICEPROJECTINFO_PATH, document.toJson());
}


void ProjectWidget::on_btnNewParticipant_clicked()
{
    emit dataDisplayRequest(TERADATA_PARTICIPANT, 0);
}


void ProjectWidget::on_btnNewGroup_clicked()
{
    emit dataDisplayRequest(TERADATA_GROUP, 0);
}


void ProjectWidget::on_btnDelete_clicked()
{
    GlobalMessageBox diag;
    QMessageBox::StandardButton answer = QMessageBox::No;
    if (ui->tableSummary->selectionModel()->selectedRows().count() == 1){
        QTableWidgetItem* base_item = ui->tableSummary->item(ui->tableSummary->currentRow(),0);
        answer = diag.showYesNo(tr("Suppression?"), tr("Êtes-vous sûrs de vouloir supprimer """) + base_item->text() + """?");
    }else{
        answer = diag.showYesNo(tr("Suppression?"), tr("Êtes-vous sûrs de vouloir supprimer tous les participants sélectionnés?"));
    }

    if (answer == QMessageBox::Yes){
        // We must delete!
        foreach(QModelIndex selected_row, ui->tableSummary->selectionModel()->selectedRows()){
            QTableWidgetItem* base_item = ui->tableSummary->item(selected_row.row(),0); // Get item at index 0 which is linked to session id
            //m_comManager->doDelete(TeraData::getPathForDataType(TERADATA_SESSION), m_listSessions_items.key(base_item));
            m_comManager->doDelete(TeraData::getPathForDataType(TERADATA_PARTICIPANT), m_tableParticipants_items.key(base_item));
        }
    }
}


void ProjectWidget::on_tableSummary_itemSelectionChanged()
{
    ui->btnDelete->setEnabled(!ui->tableSummary->selectedItems().isEmpty());
}

void ProjectWidget::on_btnManageUsers_clicked()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }

    m_diag_editor = new BaseDialog(this);
    //DataListWidget* list_widget = new DataListWidget(m_comManager, TERADATA_USERGROUP, nullptr);
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
    args.addQueryItem(WEB_QUERY_BY_USERS, "1");
    DataListWidget* list_widget = new DataListWidget(m_comManager, TERADATA_USER, WEB_PROJECTACCESS_PATH, args, QStringList("project_access_role"), nullptr);
    list_widget->setPermissions(m_comManager->getCurrentUserProjectRole(m_data->getId()) == "admin", isSiteAdmin());
    list_widget->setFilterText(tr("Seuls les utilisateurs ayant un accès au projet sont affichés."));
    m_diag_editor->setCentralWidget(list_widget);

    m_diag_editor->setWindowTitle(tr("Utilisateurs"));
    m_diag_editor->setMinimumSize(size().width(), 3*size().height()/4);

    connect(m_diag_editor, &BaseDialog::finished, this, &ProjectWidget::usersEditor_finished);
    m_diag_editor->open();
}

