#include "SiteWidget.h"
#include "ui_SiteWidget.h"

#include "editors/DataListWidget.h"

SiteWidget::SiteWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::SiteWidget)
{
    m_diag_editor = nullptr;


    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    // Limited by default
    m_limited = true;

    // Use base class to manage editing
    setEditorControls(ui->wdgSite, ui->btnEdit, ui->frameSave, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    // Query forms definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_SITE));

    ui->wdgSite->setComManager(m_comManager);
    SiteWidget::setData(data);

    // Set default tabs
    ui->tabNav->setCurrentIndex(0);
    ui->tabManageUsers->setCurrentIndex(0);
}

SiteWidget::~SiteWidget()
{
    delete ui;

}

void SiteWidget::saveData(bool signal)
{
    // Site data
    QJsonDocument site_data = ui->wdgSite->getFormDataJson(m_data->isNew());

    //qDebug() << user_data.toJson();
    postDataRequest(WEB_SITEINFO_PATH, site_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgSite->getFormDataObject(TERADATA_SITE);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void SiteWidget::setData(const TeraData *data)
{
    DataEditorWidget::setData(data);

    if (!dataIsNew()){
        // Loads first detailled informations tab
        //on_tabSiteInfos_currentChanged(0);

        // Query stats
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getFieldValue("id_site").toInt()));
        queryDataRequest(WEB_STATS_PATH, args);
    }
}

void SiteWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &SiteWidget::processFormsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &SiteWidget::processPostOKReply);
    connect(m_comManager, &ComManager::siteAccessReceived, this, &SiteWidget::processSiteAccessReply);
    connect(m_comManager, &ComManager::servicesSitesReceived, this, &SiteWidget::processServiceSiteAccessReply);
    connect(m_comManager, &ComManager::statsReceived, this, &SiteWidget::processStatsReply);

    connect(ui->btnUpdateRoles, &QPushButton::clicked, this, &SiteWidget::btnUpdateAccess_clicked);

}

void SiteWidget::updateUserGroupSiteAccess(const TeraData *access)
{
    QTableWidgetItem* item;
    int id_user_group = access->getFieldValue("id_user_group").toInt();
    QString site_role = access->getFieldValue("site_access_role").toString();
    QComboBox* combo_roles;
    QTableWidgetItem* inherited_item;

    if (m_tableUserGroups_items.contains(id_user_group)){
        item = m_tableUserGroups_items[id_user_group];
        combo_roles = dynamic_cast<QComboBox*>(ui->tableUserGroups->cellWidget(item->row(),1));
        inherited_item = ui->tableUserGroups->item(item->row(), 2);

    }else{
        // Not there - must add the usergroup and role
        ui->tableUserGroups->setRowCount(ui->tableUserGroups->rowCount()+1);
        int current_row = ui->tableUserGroups->rowCount()-1;
        QTableWidgetItem* item = new QTableWidgetItem(QIcon(access->getIconFilenameForDataType(TERADATA_USERGROUP)),
                                                      access->getFieldValue("user_group_name").toString());
        ui->tableUserGroups->setItem(current_row, 0, item);
        combo_roles = buildRolesComboBox();
        ui->tableUserGroups->setCellWidget(current_row, 1, combo_roles);
        inherited_item = new QTableWidgetItem("");
        inherited_item->setForeground(Qt::green);
        ui->tableUserGroups->setItem(current_row, 2, inherited_item);
        m_tableUserGroups_items.insert(id_user_group, item);
    }

    if (combo_roles){
        int index = -1;

        if (inherited_item)
            inherited_item->setText("");

        if (access->hasFieldName("site_access_inherited")){
            if (access->getFieldValue("site_access_inherited").toBool()){
                // Inherited access - disable combobox
                //combo_roles->setDisabled(true);
                if (combo_roles->count() == 3)
                    combo_roles->removeItem(0); // Remove "no role" item
                if (inherited_item)
                    inherited_item->setText("Administrateur / Utilisateur dans au moins un projet du site");
            }
        }
        index = combo_roles->findData(site_role);
        if (index >= 0){
            combo_roles->setCurrentIndex(index);
        }else{
            combo_roles->setCurrentIndex(0);
        }
        combo_roles->setProperty("original_index", index);
        combo_roles->setEnabled(!m_limited);
    }

}

void SiteWidget::updateControlsState()
{
    bool is_super_admin = m_comManager->isCurrentUserSuperAdmin();
    bool is_site_admin = isSiteAdmin() || is_super_admin;

    ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabUsersDetails), is_site_admin);
    ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabDevices), is_site_admin);
    ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabServices), is_site_admin);

    // TODO: Services and devices read only
    /*ui->btnUpdateRoles->setVisible(is_site_admin);
    ui->btnUserGroups->setVisible(is_site_admin);*/

    ui->grpSummary->setVisible(!dataIsNew());
    if (ui->tabNav->count() > 1 && dataIsNew()){
        ui->tabNav->removeTab(1);
    }

}

void SiteWidget::updateFieldsValue()
{
    if (m_data){
        ui->wdgSite->fillFormFromData(m_data->toJson());
        ui->lblTitle->setText(m_data->getName());
    }
}

bool SiteWidget::validateData()
{
    return ui->wdgSite->validateFormData();
}

bool SiteWidget::isSiteAdmin()
{
    if (m_data){
        return m_comManager->isCurrentUserSiteAdmin((m_data->getId()));
    }
    else{
        return false;
    }
}

void SiteWidget::queryUserGroupsSiteAccess()
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getId()));
    //args.addQueryItem(WEB_QUERY_WITH_EMPTY, "1"); // Includes user groups without any access
    queryDataRequest(WEB_SITEACCESS_PATH, args);

}

void SiteWidget::queryServiceSiteAccess()
{
    // Query services for this site
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getId()));
    args.addQueryItem(WEB_QUERY_WITH_SERVICES, "1");
    queryDataRequest(WEB_SERVICESITEINFO_PATH, args);
}

void SiteWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type == WEB_FORMS_QUERY_SITE){
        ui->wdgSite->buildUiFromStructure(data);
        return;
    }
}

void SiteWidget::processSiteAccessReply(QList<TeraData> access, QUrlQuery reply_query)
{
    if (!m_data)
        return;

    // Check if that reply is for us or not.
    if (!reply_query.hasQueryItem(WEB_QUERY_ID_SITE))
        return;

    if (reply_query.queryItemValue(WEB_QUERY_ID_SITE).toInt() != m_data->getId())
        return;

    if (!reply_query.hasQueryItem(WEB_QUERY_BY_USERS)){
        // User groups in reply
        for (int i=0; i<access.count(); i++) {
           updateUserGroupSiteAccess(&access.at(i));
        }
    }

}

void SiteWidget::processServiceSiteAccessReply(QList<TeraData> service_sites, QUrlQuery reply_query)
{
    for(const TeraData &service_site: service_sites){
        updateServiceSite(&service_site);
    }

    // New list received - disable save button
    ui->btnUpdateServices->setEnabled(false);
}

void SiteWidget::processStatsReply(TeraData stats, QUrlQuery reply_query)
{

    // Check if stats are for us
    if (!reply_query.hasQueryItem("id_site"))
        return;

    if (reply_query.queryItemValue("id_site").toInt() != m_data->getId())
        return;

    // Fill stats information
    ui->lblUsers->setText(stats.getFieldValue("users_total_count").toString() + tr(" Utilisateurs") + "\n"
                          + stats.getFieldValue("users_enabled_count").toString() + " Utilisateurs actifs");
    ui->lblProjects->setText(stats.getFieldValue("projects_count").toString() + tr(" Projets"));
    ui->lblGroups->setText(stats.getFieldValue("participants_groups_count").toString() + tr(" Groupes participants"));
    ui->lblParticipant->setText(stats.getFieldValue("participants_total_count").toString() + tr(" Participants") + "\n"
                                + stats.getFieldValue("participants_enabled_count").toString() + tr(" Participants actifs"));
    ui->lblSessions->setText(stats.getFieldValue("sessions_total_count").toString() + tr(" Séances planifiées \nou réalisées"));
    ui->lblDevices->setText(stats.getFieldValue("devices_total_count").toString() + tr(" Appareils"));

}

void SiteWidget::updateServiceSite(const TeraData *service_site)
{
    int id_service = service_site->getFieldValue("id_service").toInt();
    QString service_name = service_site->getFieldValue("service_name").toString();
    QListWidgetItem* item;

    if (m_listServices_items.contains(id_service)){
        item = m_listServices_items[id_service];
    }else{
        item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TeraDataTypes::TERADATA_SERVICE)), service_name);
        ui->lstServices->addItem(item);
        m_listServices_items[id_service] = item;

    }

    item->setText(service_name);
    int id_service_site = service_site->getId();
    if (id_service_site > 0){
        item->setCheckState(Qt::Checked);
        if (!m_listServicesSites_items.contains(id_service_site)){
            m_listServicesSites_items[id_service_site] = item;
        }
    }else{
        item->setCheckState(Qt::Unchecked);
        if (m_listServicesSites_items.contains(id_service_site)){
            m_listServicesSites_items.remove(id_service_site);
        }
    }

}

void SiteWidget::processPostOKReply(QString path)
{
    if (path == WEB_SITEINFO_PATH){
        // Update current user access list for the newly created site
        m_comManager->doUpdateCurrentUser();
    }
    if (path == WEB_SITEACCESS_PATH){
        // Refresh roles
        queryUserGroupsSiteAccess();
    }
}

void SiteWidget::btnUpdateAccess_clicked()
{

    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray roles;

    for (QTableWidgetItem* item: qAsConst(m_tableUserGroups_items)){
        int user_group_id = m_tableUserGroups_items.key(item);
        int row = item->row();
//    }
//    for (int i=0; i<m_tableUserGroups_items.count(); i++){
//        int user_group_id = m_tableUserGroups_items.keys().at(i);
//        int row = m_tableUserGroups_items[user_group_id]->row();
        QComboBox* combo_roles = dynamic_cast<QComboBox*>(ui->tableUserGroups->cellWidget(row,1));
        if (combo_roles->property("original_index").toInt() != combo_roles->currentIndex()){
            QJsonObject data_obj;
            // Ok, value was modified - must add!
            QJsonValue role = combo_roles->currentData().toString();
            data_obj.insert("id_site", m_data->getId());
            data_obj.insert("id_user_group", user_group_id);
            data_obj.insert("site_access_role", role);
            roles.append(data_obj);
        }
    }

    if (!roles.isEmpty()){
        base_obj.insert("site_access", roles);
        document.setObject(base_obj);
        postDataRequest(WEB_SITEACCESS_PATH, document.toJson());
    }



}


void SiteWidget::on_btnUserGroups_clicked()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }

    m_diag_editor = new BaseDialog(this);
    //DataListWidget* list_widget = new DataListWidget(m_comManager, TERADATA_USERGROUP, nullptr);
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getId()));
    DataListWidget* list_widget = new DataListWidget(m_comManager, TERADATA_USERGROUP, WEB_SITEACCESS_PATH, args, QStringList("site_access_role"), nullptr);
    list_widget->setPermissions(isSiteAdmin(), isSiteAdmin());
    list_widget->setFilterText(tr("Seuls les groupes utilisateurs ayant un accès au site sont affichés."));
    m_diag_editor->setCentralWidget(list_widget);

    m_diag_editor->setWindowTitle(tr("Groupes Utilisateurs"));
    m_diag_editor->setMinimumSize(size().width(), 2*size().height()/3);

    connect(m_diag_editor, &BaseDialog::finished, this, &SiteWidget::userGroupsEditor_finished);
    m_diag_editor->open();
}

void SiteWidget::on_icoUsers_clicked()
{
    if (isSiteAdmin()){
        ui->tabNav->setCurrentWidget(ui->tabUsersDetails);
    }
}

void SiteWidget::on_icoProjects_clicked()
{
    //ui->tabSiteInfos->setCurrentWidget(ui->tabProjects);
    //ui->tabNav->setCurrentWidget(ui->tabDetails);
}

void SiteWidget::on_icoDevices_clicked()
{
    if (isSiteAdmin()){
        ui->tabNav->setCurrentWidget(ui->tabDevices);
    }
}

void SiteWidget::userGroupsEditor_finished()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
        m_diag_editor = nullptr;
    }

    // Refresh user groups informations
    ui->tableUserGroups->clearContents();
    ui->tableUserGroups->setRowCount(0);
    m_tableUserGroups_items.clear();
    queryUserGroupsSiteAccess();
}

void SiteWidget::on_tabNav_currentChanged(int index)
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getId()));

    QWidget* current_tab = ui->tabNav->widget(index);

    /*if (current_tab == ui->tabProjects){
        // Projects
        if (m_listProjects_items.isEmpty()){
            // Connect signal to receive updates
            connect(m_comManager, &ComManager::projectsReceived, this, &SiteWidget::processProjectsReply);

            // Query
            args.addQueryItem(WEB_QUERY_LIST, "true");
            queryDataRequest(WEB_PROJECTINFO_PATH, args);
        }
    }*/

    if (current_tab == ui->tabDevices){
        // Devices
        if (!ui->wdgDevices->layout()){
            QHBoxLayout* layout = new QHBoxLayout();
            layout->setMargin(0);
            ui->wdgDevices->setLayout(layout);
        }
        if (ui->wdgDevices->layout()->count() == 0){
            args.addQueryItem(WEB_QUERY_WITH_PARTICIPANTS, "");
            args.addQueryItem(WEB_QUERY_WITH_SITES, "");
            DataListWidget* deviceslist_editor = new DataListWidget(m_comManager, TERADATA_DEVICE, args, QStringList("device_participants.participant_name"), ui->wdgUsers);
            deviceslist_editor->setPermissions(isSiteAdmin(), m_comManager->isCurrentUserSuperAdmin());
            deviceslist_editor->setFilterText(tr("Seuls les appareils associés au site sont affichés."));
            ui->wdgDevices->layout()->addWidget(deviceslist_editor);
        }
    }

    if (current_tab == ui->tabUsersDetails){
        // Users
        if (!ui->wdgUsers->layout()){
            QHBoxLayout* layout = new QHBoxLayout();
            layout->setMargin(0);
            ui->wdgUsers->setLayout(layout);
        }
        if (ui->wdgUsers->layout()->count() == 0){
            args.addQueryItem(WEB_QUERY_BY_USERS, "1");
            args.addQueryItem(WEB_QUERY_WITH_USERGROUPS, "1"); // Includes user groups details
            DataListWidget* userlist_editor = new DataListWidget(m_comManager, TERADATA_USER, WEB_SITEACCESS_PATH, args, QStringList("site_access_role"), ui->wdgUsers);
            userlist_editor->setPermissions(isSiteAdmin(), isSiteAdmin());
            userlist_editor->setFilterText(tr("Seuls les utilisateurs ayant un accès au site sont affichés."));
            ui->wdgUsers->layout()->addWidget(userlist_editor);
        }
    }

    if (current_tab == ui->tabServices){
        if (m_listServices_items.isEmpty()){
            queryServiceSiteAccess();
        }
    }


}


void SiteWidget::on_tabManageUsers_currentChanged(int index)
{
    QWidget* current_tab = ui->tabManageUsers->widget(index);

    if (current_tab == ui->tabUserGroups){
        // User groups
        if (m_tableUserGroups_items.isEmpty()){
            // Query
            queryUserGroupsSiteAccess();
        }
    }
}


void SiteWidget::on_btnUpdateServices_clicked()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray services_sites;
    bool removed_services = false;

    for (int i=0; i<ui->lstServices->count(); i++){
        QListWidgetItem* item = ui->lstServices->item(i);
        int service_id = m_listServices_items.key(item, 0);
        if (item->checkState() == Qt::Checked){
            // New item selected
            QJsonObject item_obj;
            item_obj.insert("id_service", service_id);
            item_obj.insert("id_site", m_data->getId());
            services_sites.append(item_obj);
        }else{
            if (std::find(m_listServicesSites_items.cbegin(), m_listServicesSites_items.cend(), item) == m_listServicesSites_items.cend()){
                removed_services = true;
            }
        }
     }

    if (removed_services){
        GlobalMessageBox msgbox;
        int rval = msgbox.showYesNo(tr("Suppression de service associé"), tr("Au moins un service retiré est associé à un projet.\nSi vous le retirez, il sera désassocié de ce projet et les séances associées à ce service ne seront plus accessibles.\nSouhaitez-vous continuer?"));
        if (rval != GlobalMessageBox::Yes){
            return;
        }
    }

    base_obj.insert("service_site", services_sites);
    document.setObject(base_obj);
    postDataRequest(WEB_SERVICESITEINFO_PATH, document.toJson());
}


void SiteWidget::on_lstServices_itemChanged(QListWidgetItem *item)
{
    // Check for changed items
    bool has_changes = false;
    if (m_listServicesSites_items.key(item) > 0 && item->checkState() == Qt::Unchecked){
        // Item deselected
        has_changes = true;
    }else{
        if (m_listServicesSites_items.key(item) <= 0 && item->checkState() == Qt::Checked){
            // Item selected
            has_changes = true;
        }
    }

    ui->btnUpdateServices->setEnabled(has_changes);
}

