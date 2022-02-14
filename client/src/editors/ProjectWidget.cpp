#include "ProjectWidget.h"
#include "ui_ProjectWidget.h"

#include "editors/DataListWidget.h"
#include "Logger.h"

ProjectWidget::ProjectWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::ProjectWidget)
{
    m_diag_editor = nullptr;

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
    QJsonDocument site_data = ui->wdgProject->getFormDataJson(m_data->isNew());

    //qDebug() << user_data.toJson();
    postDataRequest(WEB_PROJECTINFO_PATH, site_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgProject->getFormDataObject(TERADATA_PROJECT);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void ProjectWidget::setData(const TeraData *data)
{
    DataEditorWidget::setData(data);

    if (!dataIsNew()){
        // Loads first detailled informations tab
        on_tabProjectInfos_currentChanged(0);

        // Query stats
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
        args.addQueryItem(WEB_QUERY_WITH_PARTICIPANTS, "1");
        queryDataRequest(WEB_STATS_PATH, args);

    }else{
        ui->tabProjectInfos->setEnabled(false);
    }
}

void ProjectWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &ProjectWidget::processFormsReply);
    connect(m_comManager, &ComManager::projectAccessReceived, this, &ProjectWidget::processProjectAccessReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &ProjectWidget::processPostOKReply);
    connect(m_comManager, &ComManager::servicesProjectsReceived, this, &ProjectWidget::processServiceProjectsReply);
    connect(m_comManager, &ComManager::statsReceived, this, &ProjectWidget::processStatsReply);
    connect(m_comManager, &ComManager::deleteResultsOK, this, &ProjectWidget::deleteDataReply);

    connect(ui->btnUpdateRoles, &QPushButton::clicked, this, &ProjectWidget::btnUpdateAccess_clicked);
    //connect(ui->btnDevices, &QPushButton::clicked, this, &ProjectWidget::btnDevices_clicked);

}

void ProjectWidget::initUI()
{
    // Default display
    ui->tabNav->setCurrentIndex(0);
    ui->tabProjectInfos->setCurrentIndex(0);

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

void ProjectWidget::updateGroup(const TeraData *group)
{
    int id_group = group->getId();
    if (m_listGroups_items.contains(id_group)){
        QListWidgetItem* item = m_listGroups_items[id_group];
        item->setText(group->getName());
    }else{
        QListWidgetItem* item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_GROUP)), group->getName());
        ui->lstGroups->addItem(item);
        m_listGroups_items[id_group] = item;
    }
}

void ProjectWidget::updateDevice(const TeraData *device)
{
    int id_device = device->getId();
    QString participants_string = tr("Aucun");
    int part_count = 0;

    // Build participant string if availables
    if (device->hasFieldName("device_participants")){
        QVariantList participants = device->getFieldValue("device_participants").toList();
        participants_string = "";
        for (int i=0; i<participants.count(); i++){
            QVariantMap part_info = participants.at(i).toMap();
            // Only consider participants of that project.
            if (part_info["id_project"].toInt() == m_data->getId()){
                if (part_count>0)
                    participants_string += ", ";
                participants_string += part_info["participant_name"].toString();
                part_count++;
            }
        }
    }

    // Ignore devices without any participant in that project
    if (participants_string.isEmpty())
        return;

    if (m_listDevices_items.contains(id_device)){
        QTableWidgetItem* item = m_listDevices_items[id_device];
        item->setText(device->getName());
        ui->lstDevices->item(item->row(), 1)->setText(participants_string);
    }else{
        ui->lstDevices->setRowCount(ui->lstDevices->rowCount()+1);
        QString device_name = device->getName();
        if (device_name.isEmpty())
            device_name = tr("(Appareil sans nom)");
        QTableWidgetItem* item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_DEVICE)), device_name);
        ui->lstDevices->setItem(ui->lstDevices->rowCount()-1, 0, item);
        m_listDevices_items[id_device] = item;

        item = new QTableWidgetItem(participants_string);
        ui->lstDevices->setItem(ui->lstDevices->rowCount()-1, 1, item);
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

    if (id_service_project > 0){
        if (!m_listServicesProjects_items.contains(id_service_project)){
            m_listServicesProjects_items[id_service_project] = item;
        }
    }
    if (!service_name.isEmpty())
        item->setText(service_name);

    if (sp->getFieldValue("id_project").toInt() == m_data->getId()){
        item->setCheckState(Qt::Checked);
    }else{
        item->setCheckState(Qt::Unchecked);
    }

    if (sp->hasFieldName("service_key")){
        m_services_keys[id_service] = sp->getFieldValue("service_key").toString();
    }

}

void ProjectWidget::queryServicesProject()
{
    // Query services for this project
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
    args.addQueryItem(WEB_QUERY_WITH_SERVICES, "1");
    queryDataRequest(WEB_SERVICEPROJECTINFO_PATH, args);
}

void ProjectWidget::updateControlsState()
{   
    if (dataIsNew()){
        ui->grpSummary->hide();
        if (ui->tabNav->count() > 1){
            ui->tabNav->removeTab(1);
        }
    }else{
        bool is_site_admin = isSiteAdmin();

        // m_limited = true if current user isn't project admin
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

        if (dataIsNew() && m_data->hasFieldName("id_site")){
            // New project - locked to that site
            ui->wdgProject->setFieldEnabled("id_site", false);
        }
    }
}

bool ProjectWidget::validateData()
{
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


void ProjectWidget::processGroupsReply(QList<TeraData> groups)
{
    if (!m_data)
        return;

    for (int i=0; i<groups.count(); i++){
        if (groups.at(i).getFieldValue("id_project") == m_data->getId()){
            updateGroup(&groups.at(i));
        }
    }

    /*if (isLoading()){
        // Query kits for that site (depending on projects first to have names)
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getId()));
        args.addQueryItem(WEB_QUERY_LIST, "");
        queryDataRequest(WEB_KITINFO_PATH, args);
    }*/
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

void ProjectWidget::on_tabProjectInfos_currentChanged(int index)
{
    // Load data depending on selected tab
    QUrlQuery args;
    QWidget* current_tab = ui->tabProjectInfos->widget(index);

    if (current_tab == ui->tabGroups){
        // Groups
        if (m_listGroups_items.isEmpty()){
            connect(m_comManager, &ComManager::groupsReceived, this, &ProjectWidget::processGroupsReply);

            args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
            args.addQueryItem(WEB_QUERY_LIST, "true");
            queryDataRequest(WEB_GROUPINFO_PATH, args);
        }
    }

    if (current_tab == ui->tabDevices){
        // Devices
        if (m_listDevices_items.isEmpty()){
            connect(m_comManager, &ComManager::devicesReceived, this, &ProjectWidget::processDevicesReply);

            args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getFieldValue("id_site").toInt()));
            args.addQueryItem(WEB_QUERY_WITH_PARTICIPANTS, "");
            queryDataRequest(WEB_DEVICEINFO_PATH, args);
        }
    }

    if (current_tab == ui->tabUsers){
        // Users
        if (m_tableUsers_items.isEmpty()){
            args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
            args.addQueryItem(WEB_QUERY_BY_USERS, "1");
            queryDataRequest(WEB_PROJECTACCESS_PATH, args);
        }
    }

    if (current_tab == ui->tabSessionTypes){
        // Session types
        if (!ui->wdgSessionTypes->layout()){
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
        }
    }

    if (current_tab == ui->tabUserGroups){
        // User groups
        if (m_tableUserGroups_items.isEmpty()){
            // Query
           queryUserGroupsProjectAccess();
        }
    }

    if (current_tab == ui->tabServices){
        // Services
       if (m_listServicesProjects_items.isEmpty()){
           queryServicesProject();
       }
    }


}
/*
void ProjectWidget::on_btnUpdateServiceRoles_clicked()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray roles;
    QList<int> roles_to_delete;

    for (int i=0; i<ui->tableServices->rowCount(); i++){
        int id_service = ui->tableServices->item(i,0)->data(Qt::UserRole).toInt();
        int id_user_group = ui->tableServices->item(i,1)->data(Qt::UserRole).toInt();
        QComboBox* combo_roles = dynamic_cast<QComboBox*>(ui->tableServices->cellWidget(i,2));
        int id_service_project_role = combo_roles->property("id_service_project_role").toInt();
        if (combo_roles->property("original_index").toInt() != combo_roles->currentIndex()){
            if (combo_roles->currentIndex()==0){
                // No role now - must delete!
                if (id_service_project_role != 0)
                    roles_to_delete.append(id_service_project_role);
            }else{
                QJsonObject data_obj;
                // Ok, value was modified - must add!
                QJsonValue role = combo_roles->currentData().toString();
                data_obj.insert("id_service", id_service);
                data_obj.insert("id_project", m_data->getId());
                data_obj.insert("id_user_group", id_user_group);
                data_obj.insert("id_service_role", role);
                data_obj.insert("id_service_project_role", id_service_project_role);
                roles.append(data_obj);
            }
            combo_roles->setProperty("original_index", combo_roles->currentIndex());
        }
    }

    // Delete queries
    for (int id_service_project_role:roles_to_delete){
        deleteDataRequest(WEB_SERVICEACCESSINFO_PATH, id_service_project_role);
    }

    // Update query
    base_obj.insert("service_project_role", roles);
    document.setObject(base_obj);
    postDataRequest(WEB_SERVICEACCESSINFO_PATH, document.toJson());
}
*/

void ProjectWidget::on_btnUpdateServices_clicked()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray services_projects;
    QList<int> todel_ids;

    for (int i=0; i<ui->lstServices->count(); i++){
        QListWidgetItem* item = ui->lstServices->item(i);
        int service_id = m_listServicesProjects_items.key(item, 0);
        //if (m_listServicesProjects_items.values().contains(item)){
        if (service_id >0){
            if (item->checkState() == Qt::Unchecked){
                // Service was unselected
                //todel_ids.append(m_listServicesProjects_items.key(item));
                todel_ids.append(service_id);
            }
        }else{
            if (item->checkState() == Qt::Checked){
                // New item selected
                QJsonObject item_obj;
                item_obj.insert("id_service", m_listServices_items.key(item));
                item_obj.insert("id_project", m_data->getId());
                services_projects.append(item_obj);
            }
        }
     }

    // Delete queries
    for (int id_service_project:todel_ids){
        deleteDataRequest(WEB_SERVICEPROJECTINFO_PATH, id_service_project);
    }

    // Update query
    if (!services_projects.isEmpty()){
        base_obj.insert("service_project", services_projects);
        document.setObject(base_obj);
        postDataRequest(WEB_SERVICEPROJECTINFO_PATH, document.toJson());
    }
}

void ProjectWidget::on_icoUsers_clicked()
{
    ui->tabProjectInfos->setCurrentWidget(ui->tabUsers);
    ui->tabNav->setCurrentWidget(ui->tabDetails);
}

void ProjectWidget::on_icoSessions_clicked()
{
    ui->tabProjectInfos->setCurrentWidget(ui->tabSessionTypes);
    ui->tabNav->setCurrentWidget(ui->tabDetails);
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

    // Dance Service
    if (service_key == "DanceService"){
        DanceConfigWidget* wdg = new DanceConfigWidget(m_comManager, m_data->getId());
        //ui->tabNav->addTab(wdg, QIcon("://icons/service.png"), service_project.getFieldValue("service_name").toString());
        QString service_name = service_key;
        if (m_listServices_items.contains(id_service)){
            service_name = m_listServices_items[id_service]->text();
        }
        ui->tabNav->addTab(wdg, QIcon("://icons/service.png"), service_name);
        m_services_tabs.insert(id_service, wdg);
    }
}
