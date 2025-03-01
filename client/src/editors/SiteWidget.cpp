#include "SiteWidget.h"
#include "ui_SiteWidget.h"

#include "editors/DataListWidget.h"
#include "services/DashboardsService/DashboardsConfigWidget.h"
#include "services/EmailService/EmailServiceConfigWidget.h"

#include "GlobalMessageBox.h"

SiteWidget::SiteWidget(ComManager *comMan, const TeraData *data, const bool configMode, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::SiteWidget)
{
    m_diag_editor = nullptr;
    m_devicesCount = 0;
    m_configMode = configMode;

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
    connect(m_comManager, &ComManager::deviceSitesReceived, this, &SiteWidget::processDeviceSiteAccessReply);
    connect(m_comManager, &ComManager::servicesSitesReceived, this, &SiteWidget::processServiceSiteAccessReply);
    connect(m_comManager, &ComManager::sessionTypesSitesReceived, this, &SiteWidget::processSessionTypeSiteAccessReply);
    connect(m_comManager, &ComManager::testTypesSitesReceived, this, &SiteWidget::processTestTypeSiteAccessReply);
    connect(m_comManager, &ComManager::statsReceived, this, &SiteWidget::processStatsReply);

    connect(ui->btnUpdateRoles, &QPushButton::clicked, this, &SiteWidget::btnUpdateAccess_clicked);

    connect(ui->wdgSite, &TeraForm::widgetValueHasChanged, this, &SiteWidget::siteValueHasChanged);

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

    if (!is_site_admin)
        ui->wdgSite->hideField("site_2fa_required");

    ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabUsersDetails), is_site_admin);
    ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabDevices), is_site_admin /*(is_site_admin && m_devicesCount>0) || is_super_admin*/);
    ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabServices), is_site_admin);
    ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabSessionTypes), is_site_admin);
    ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabTestTypes), is_site_admin);

    ui->btnUpdateServices->setVisible(is_super_admin);
    ui->btnUpdateDevices->setVisible(is_super_admin);
    ui->btnUpdateSessionTypes->setVisible(is_super_admin);
    ui->btnUpdateTestTypes->setVisible(is_super_admin);
    ui->btnEditDevices->setVisible(is_site_admin && !m_configMode);

    ui->lblAdminSessionTypes->setVisible(!is_super_admin);
    ui->lblAdminTestTypes->setVisible(!is_super_admin);

    ui->btnEditSessionTypes->setVisible(!m_configMode && is_super_admin);
    ui->btnUserGroups->setVisible(!m_configMode);

    ui->grpSummary->setVisible(!dataIsNew());
    if (dataIsNew()){
        while(ui->tabNav->count() > 1){
            ui->tabNav->removeTab(1);
        }
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

void SiteWidget::addServiceTab(const TeraData &service_site)
{
    int id_service = service_site.getFieldValue("id_service").toInt();
    if (m_services_tabs.contains(id_service)) // Already there
        return;

    if (service_site.getFieldValue("id_site").toInt() != m_data->getId())
        return; // Service not enabled for that project

    QString service_key = m_services_keys[id_service];

    // Dashboards Service
    if (service_key == "DashboardsService"){
        if (isSiteAdmin()){
            DashboardsConfigWidget* wdg = new DashboardsConfigWidget(m_comManager, m_data->getId());
            QString service_name = service_key;
            if (m_listServices_items.contains(id_service)){
                service_name = m_listServices_items[id_service]->text();
            }
            //ui->tabManageServices->insertTab(0, wdg, QIcon("://icons/config.png"), service_name);
            ui->tabManageServices->addTab(wdg, QIcon("://icons/config.png"), service_name);
            m_services_tabs.insert(id_service, wdg);
        }
    }

    // Email Service
    if (service_key == "EmailService"){
        if (isSiteAdmin()){
            EmailServiceConfigWidget* wdg = new EmailServiceConfigWidget(m_comManager, m_data->getId());
            QString service_name = service_key;
            if (m_listServices_items.contains(id_service)){
                service_name = m_listServices_items[id_service]->text();
            }
            ui->tabManageServices->addTab(wdg, QIcon("://icons/config.png"), service_name);
            m_services_tabs.insert(id_service, wdg);
        }
    }
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

void SiteWidget::queryDeviceSiteAccess()
{
    // Query devices for this site
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getId()));
    args.addQueryItem(WEB_QUERY_WITH_DEVICES, "1");
    queryDataRequest(WEB_DEVICESITEINFO_PATH, args);
}

void SiteWidget::querySessionTypeSiteAccess()
{
    // Query session types for this site
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getId()));
    args.addQueryItem(WEB_QUERY_WITH_SESSIONTYPE, "1");
    queryDataRequest(WEB_SESSIONTYPESITE_PATH, args);
}

void SiteWidget::queryTestTypeSiteAccess()
{
    // Query session types for this site
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getId()));
    args.addQueryItem(WEB_QUERY_WITH_TESTTYPES, "1");
    queryDataRequest(WEB_TESTTYPESITE_PATH, args);
}

void SiteWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type == WEB_FORMS_QUERY_SITE){
        if (!ui->wdgSite->formHasStructure())
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
        // Add specific services configuration tabs
        if (!dataIsNew()){ // Don't add anything if a new project
            //ids_service.append(service_project.getFieldValue("id_service").toInt());
            addServiceTab(service_site);
        }
    }

    // Update used list from what is checked right now
    for (int i=0; i<ui->lstServices->count(); i++){
        QListWidgetItem* item = ui->lstServices->item(i);
        if (item->checkState() == Qt::Unchecked){
            int item_key = m_listServicesSites_items.key(item, -1);
            if (item_key > 0){
                m_listServicesSites_items.remove(item_key);
            }
        }
    }


    // New list received - disable save button
    ui->btnUpdateServices->setEnabled(false);
}

void SiteWidget::processDeviceSiteAccessReply(QList<TeraData> device_sites, QUrlQuery reply_query)
{
    for(const TeraData &device_site: device_sites){
        updateDeviceSite(&device_site);
    }

    // Update used list from what is checked right now
    for (int i=0; i<ui->lstDevices->count(); i++){
        QListWidgetItem* item = ui->lstDevices->item(i);
        if (item->checkState() == Qt::Unchecked){
            int item_key = m_listDevicesSites_items.key(item, -1);
            if (item_key > 0){
                m_listDevicesSites_items.remove(item_key);
            }
        }
    }

    // New list received - disable save button
    ui->btnUpdateDevices->setEnabled(false);
}

void SiteWidget::processSessionTypeSiteAccessReply(QList<TeraData> st_sites, QUrlQuery reply_query)
{
    for(const TeraData &st_site: st_sites){
        updateSessionTypeSite(&st_site);
    }

    // Update used list from what is checked right now
    for (int i=0; i<ui->lstSessionTypes->count(); i++){
        QListWidgetItem* item = ui->lstSessionTypes->item(i);
        if (item->checkState() == Qt::Unchecked){
            int item_key = m_listSessionTypeSites_items.key(item, -1);
            if (item_key > 0){
                m_listSessionTypeSites_items.remove(item_key);
            }
        }
    }

    // New list received - disable save button
    ui->btnUpdateSessionTypes->setEnabled(false);
}

void SiteWidget::processTestTypeSiteAccessReply(QList<TeraData> tt_sites, QUrlQuery reply_query)
{
    for(const TeraData &tt_site: tt_sites){
        updateTestTypeSite(&tt_site);
    }

    // Update used list from what is checked right now
    for (int i=0; i<ui->lstTestTypes->count(); i++){
        QListWidgetItem* item = ui->lstTestTypes->item(i);
        if (item->checkState() == Qt::Unchecked){
            int item_key = m_listTestTypeSites_items.key(item, -1);
            if (item_key > 0){
                m_listTestTypeSites_items.remove(item_key);
            }
        }
    }

    // New list received - disable save button
    ui->btnUpdateTestTypes->setEnabled(false);
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

    m_devicesCount = stats.getFieldValue("devices_total_count").toInt();

}

void SiteWidget::siteValueHasChanged(QWidget *widget, QVariant value)
{
    if (widget == ui->wdgSite->getWidgetForField("site_2fa_required")){
        if (value.toBool()){
            GlobalMessageBox msgbox;
            if (msgbox.showYesNo(tr("Authentification multi-facteurs"),
                                 tr("Activer l'authentification multi-facteurs forcera les utilisateurs ayant accès au site à utiliser cette authentification.\n\n"
                                    "Cette action sera difficilement réversible - il faudra désactiver l'authentification pour chacun des utilisateurs individuellement.\n\nVoulez-vous continuer?")) == GlobalMessageBox::No){
                ui->wdgSite->setFieldValue("site_2fa_required", false);
            }
        }
    }
}

void SiteWidget::updateServiceSite(const TeraData *service_site)
{
    int id_site = service_site->getFieldValue("id_site").toInt();

    if (id_site != m_data->getId() && id_site>0)
        return; // Not for us

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

    if (!service_name.isEmpty())
        item->setText(service_name);

    int id_service_site = service_site->getId();
    if (id_service_site > 0){
        if (m_comManager->isCurrentUserSuperAdmin())
            item->setCheckState(Qt::Checked);
        if (!m_listServicesSites_items.contains(id_service_site)){
            m_listServicesSites_items[id_service_site] = item;
        }
    }else{
        if (m_comManager->isCurrentUserSuperAdmin())
            item->setCheckState(Qt::Unchecked);
        if (m_listServicesSites_items.contains(id_service_site)){
            m_listServicesSites_items.remove(id_service_site);
        }
    }

    if (service_site->hasFieldName("service_key")){
        m_services_keys[id_service] = service_site->getFieldValue("service_key").toString();
    }

}

void SiteWidget::updateDeviceSite(const TeraData *device_site)
{
    int id_site = device_site->getFieldValue("id_site").toInt();

    if (id_site != m_data->getId() && id_site>0)
        return; // Not for us

    int id_device = device_site->getFieldValue("id_device").toInt();
    QString device_name = device_site->getFieldValue("device_name").toString();
    QListWidgetItem* item;

    if (m_listDevices_items.contains(id_device)){
        item = m_listDevices_items[id_device];
    }else{
        item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TeraDataTypes::TERADATA_DEVICE)), device_name);
        ui->lstDevices->addItem(item);
        m_listDevices_items[id_device] = item;

    }

    if (!device_name.isEmpty())
        item->setText(device_name);

    int id_device_site = device_site->getId();
    if (id_device_site > 0){
        if (m_comManager->isCurrentUserSuperAdmin())
            item->setCheckState(Qt::Checked);
        if (!m_listDevicesSites_items.contains(id_device_site)){
            m_listDevicesSites_items[id_device_site] = item;
        }
    }else{
        if (m_comManager->isCurrentUserSuperAdmin())
            item->setCheckState(Qt::Unchecked);
        if (m_listDevicesSites_items.contains(id_device_site)){
            m_listDevicesSites_items.remove(id_device_site);
        }
    }
}

void SiteWidget::updateSessionTypeSite(const TeraData *st_site)
{
    int id_site = st_site->getFieldValue("id_site").toInt();

    if (id_site != m_data->getId() && id_site>0)
        return; // Not for us

    int id_session_type = st_site->getFieldValue("id_session_type").toInt();
    QString st_name = st_site->getFieldValue("session_type_name").toString();
    QListWidgetItem* item;

    if (m_listSessionTypes_items.contains(id_session_type)){
        item = m_listSessionTypes_items[id_session_type];
    }else{
        item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TeraDataTypes::TERADATA_SESSIONTYPE)), st_name);
        ui->lstSessionTypes->addItem(item);
        m_listSessionTypes_items[id_session_type] = item;

    }

    if (!st_name.isEmpty())
        item->setText(st_name);

    int id_session_type_site = st_site->getId();
    if (id_session_type_site > 0){
        if (m_comManager->isCurrentUserSuperAdmin())
            item->setCheckState(Qt::Checked);
        if (!m_listSessionTypeSites_items.contains(id_session_type_site)){
            m_listSessionTypeSites_items[id_session_type_site] = item;
        }
    }else{
        if (m_comManager->isCurrentUserSuperAdmin())
            item->setCheckState(Qt::Unchecked);
        if (m_listSessionTypeSites_items.contains(id_session_type_site)){
            m_listSessionTypeSites_items.remove(id_session_type_site);
        }
    }
}

void SiteWidget::updateTestTypeSite(const TeraData *tt_site)
{
    int id_site = tt_site->getFieldValue("id_site").toInt();

    if (id_site != m_data->getId() && id_site>0)
        return; // Not for us

    int id_test_type = tt_site->getFieldValue("id_test_type").toInt();
    QString tt_name = tt_site->getFieldValue("test_type_name").toString();
    QListWidgetItem* item;

    if (m_listTestTypes_items.contains(id_test_type)){
        item = m_listTestTypes_items[id_test_type];
    }else{
        item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TeraDataTypes::TERADATA_TESTTYPE)), tt_name);
        ui->lstTestTypes->addItem(item);
        m_listTestTypes_items[id_test_type] = item;
    }

    if (!tt_name.isEmpty())
        item->setText(tt_name);

    int id_test_type_site = tt_site->getId();
    if (id_test_type_site > 0){
        if (m_comManager->isCurrentUserSuperAdmin())
            item->setCheckState(Qt::Checked);
        if (!m_listTestTypeSites_items.contains(id_test_type_site)){
            m_listTestTypeSites_items[id_test_type_site] = item;
        }
    }else{
        if (m_comManager->isCurrentUserSuperAdmin())
            item->setCheckState(Qt::Unchecked);
        if (m_listTestTypeSites_items.contains(id_test_type_site)){
            m_listTestTypeSites_items.remove(id_test_type_site);
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
    if (path == WEB_SESSIONTYPEPROJECT_PATH || path == WEB_TESTTYPEPROJECT_PATH){
        // Update associated services
        queryServiceSiteAccess();
    }
}

void SiteWidget::btnUpdateAccess_clicked()
{

    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray roles;

    for (QTableWidgetItem* item: std::as_const(m_tableUserGroups_items)){
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

void SiteWidget::devicesEditor_finished()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
        m_diag_editor = nullptr;
    }

    // Refresh device site informations
    m_listDevicesSites_items.clear();
    m_listDevices_items.clear();
    ui->lstDevices->clear();
    queryDeviceSiteAccess();
}

void SiteWidget::sessionTypesEditor_finished()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
        m_diag_editor = nullptr;
    }

    // Refresh session type site informations
    m_listSessionTypeSites_items.clear();
    m_listSessionTypes_items.clear();
    ui->lstSessionTypes->clear();
    querySessionTypeSiteAccess();
}

void SiteWidget::on_tabNav_currentChanged(int index)
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getId()));

    QWidget* current_tab = ui->tabNav->widget(index);

    if (current_tab == ui->tabDevices){
        if (m_listDevices_items.isEmpty()){
            queryDeviceSiteAccess();
        }
    }

    if (current_tab == ui->tabUsersDetails){
        // Users
        if (!ui->wdgUsers->layout()){
            QHBoxLayout* layout = new QHBoxLayout();
            layout->setContentsMargins(0,0,0,0);
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
        ui->tabManageServices->setCurrentIndex(0);
    }

    if (current_tab == ui->tabSessionTypes){
        // Session types
        querySessionTypeSiteAccess();
    }

    if (current_tab == ui->tabTestTypes){
        queryTestTypeSiteAccess();
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
    QJsonObject site_obj;
    QJsonArray services_sites;
    bool removed_services = false;

    site_obj.insert("id_site", m_data->getId());
    for (int i=0; i<ui->lstServices->count(); i++){
        QListWidgetItem* item = ui->lstServices->item(i);
        int service_id = m_listServices_items.key(item, 0);
        if (item->checkState() == Qt::Checked){
            // New item selected
            QJsonObject item_obj;
            item_obj.insert("id_service", service_id);
            services_sites.append(item_obj);
        }else{
            if (!removed_services){
                if (m_listServicesSites_items.key(item, 0) > 0){
                    removed_services = true;
                }
            }
        }
     }

    if (removed_services){
        GlobalMessageBox msgbox;
        int rval = msgbox.showYesNo(tr("Suppression de service associé"), tr("Au moins un service a été retiré de ce site. S'il y a des projets qui utilisent ce service, ils ne pourront plus l'utiliser.\nSouhaitez-vous continuer?"));
        if (rval != GlobalMessageBox::Yes){
            return;
        }
    }

    site_obj.insert("services", services_sites);
    base_obj.insert("site", site_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_SERVICESITEINFO_PATH, document.toJson());
}


void SiteWidget::on_lstServices_itemChanged(QListWidgetItem *item)
{
    if (!m_comManager->isCurrentUserSuperAdmin())
        return;

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

    if (item->checkState() == Qt::Checked){
        item->setForeground(Qt::green);
    }else{
        item->setForeground(Qt::red);
    }

    ui->btnUpdateServices->setEnabled(has_changes);
}


void SiteWidget::on_lstDevices_itemChanged(QListWidgetItem *item)
{
    if (!m_comManager->isCurrentUserSuperAdmin())
        return;

    // Check for changed items
    bool has_changes = false;
    if (m_listDevicesSites_items.key(item) > 0 && item->checkState() == Qt::Unchecked){
        // Item deselected
        has_changes = true;
    }else{
        if (m_listDevicesSites_items.key(item) <= 0 && item->checkState() == Qt::Checked){
            // Item selected
            has_changes = true;
        }
    }

    if (item->checkState() == Qt::Checked){
        item->setForeground(Qt::green);
    }else{
        item->setForeground(Qt::red);
    }

    ui->btnUpdateDevices->setEnabled(has_changes);
}


void SiteWidget::on_btnUpdateDevices_clicked()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonObject site_obj;
    QJsonArray devices_sites;
    bool removed_devices = false;

    site_obj.insert("id_site", m_data->getId());
    for (int i=0; i<ui->lstDevices->count(); i++){
        QListWidgetItem* item = ui->lstDevices->item(i);
        int device_id = m_listDevices_items.key(item, 0);
        if (item->checkState() == Qt::Checked){
            // New item selected
            QJsonObject item_obj;
            item_obj.insert("id_device", device_id);
            devices_sites.append(item_obj);
        }else{
            if (!removed_devices){
                if (m_listDevicesSites_items.key(item, 0) > 0){
                    removed_devices = true;
                }
            }
        }
     }

    if (removed_devices){
        GlobalMessageBox msgbox;
        int rval = msgbox.showYesNo(tr("Suppression d'appareil associé"), tr("Au moins un appareil a été retiré de ce site. S'il y a des projets qui utilisent cet appareil, ils ne pourront plus l'utiliser.\nSouhaitez-vous continuer?"));
        if (rval != GlobalMessageBox::Yes){
            return;
        }
    }

    site_obj.insert("devices", devices_sites);
    base_obj.insert("site", site_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_DEVICESITEINFO_PATH, document.toJson());
}


void SiteWidget::on_txtSearchDevices_textChanged(const QString &search_text)
{
    for(QListWidgetItem* item: std::as_const(m_listDevices_items)){
        item->setHidden(!item->text().contains(search_text, Qt::CaseInsensitive));
    }
}


void SiteWidget::on_btnEditDevices_clicked()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }

    m_diag_editor = new BaseDialog(this);
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getId()));
    args.addQueryItem(WEB_QUERY_WITH_PARTICIPANTS, "1");
    args.addQueryItem(WEB_QUERY_WITH_SITES, "1");
    DataListWidget* list_widget = new DataListWidget(m_comManager, TERADATA_DEVICE, args, QStringList(), m_diag_editor);
    list_widget->setPermissions(isSiteAdmin(), isSiteAdmin());
    list_widget->setFilterText(tr("Seuls les appareils associés au site sont affichés."));
    m_diag_editor->setCentralWidget(list_widget);

    m_diag_editor->setWindowTitle(tr("Appareils"));
    m_diag_editor->setMinimumSize(size().width(), 2*size().height()/3);

    connect(m_diag_editor, &BaseDialog::finished, this, &SiteWidget::devicesEditor_finished);
    m_diag_editor->open();
    /*args.addQueryItem(WEB_QUERY_WITH_PARTICIPANTS, "");
    args.addQueryItem(WEB_QUERY_WITH_SITES, "");
    DataListWidget* deviceslist_editor = new DataListWidget(m_comManager, TERADATA_DEVICE, args, QStringList("device_participants.participant_name"), ui->wdgUsers);
    deviceslist_editor->setPermissions(isSiteAdmin(), m_comManager->isCurrentUserSuperAdmin());
    deviceslist_editor->setFilterText(tr("Seuls les appareils associés au site sont affichés."));*/
}


void SiteWidget::on_lstSessionTypes_itemChanged(QListWidgetItem *item)
{
    if (!m_comManager->isCurrentUserSuperAdmin())
        return;

    // Check for changed items
    bool has_changes = false;
    if (m_listSessionTypeSites_items.key(item) > 0 && item->checkState() == Qt::Unchecked){
        // Item deselected
        has_changes = true;
    }else{
        if (m_listSessionTypeSites_items.key(item) <= 0 && item->checkState() == Qt::Checked){
            // Item selected
            has_changes = true;
        }
    }

    if (item->checkState() == Qt::Checked){
        item->setForeground(Qt::green);
    }else{
        item->setForeground(Qt::red);
    }

    ui->btnUpdateSessionTypes->setEnabled(has_changes);
}


void SiteWidget::on_btnUpdateSessionTypes_clicked()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonObject site_obj;
    QJsonArray st_sites;
    bool removed_sts = false;

    site_obj.insert("id_site", m_data->getId());
    for (int i=0; i<ui->lstSessionTypes->count(); i++){
        QListWidgetItem* item = ui->lstSessionTypes->item(i);
        int st_id = m_listSessionTypes_items.key(item, 0);
        if (item->checkState() == Qt::Checked){
            // New item selected
            QJsonObject item_obj;
            item_obj.insert("id_session_type", st_id);
            st_sites.append(item_obj);
        }else{
            if (!removed_sts){
                if (m_listSessionTypes_items.key(item, 0) > 0){
                    removed_sts = true;
                }
            }
        }
     }

    if (removed_sts){
        GlobalMessageBox msgbox;
        int rval = msgbox.showYesNo(tr("Suppression de types de séances associés"), tr("Au moins un type de séance a été retiré de ce site. S'il y a des projets qui utilisent ce type, ils ne pourront plus l'utiliser.\nSouhaitez-vous continuer?"));
        if (rval != GlobalMessageBox::Yes){
            return;
        }
    }

    site_obj.insert("sessiontypes", st_sites);
    base_obj.insert("site", site_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_SESSIONTYPESITE_PATH, document.toJson());
}


void SiteWidget::on_btnEditSessionTypes_clicked()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }

    m_diag_editor = new BaseDialog(this);
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getId()));
    DataListWidget* list_widget = new DataListWidget(m_comManager, TERADATA_SESSIONTYPE, WEB_SESSIONTYPESITE_PATH, args, QStringList(), m_diag_editor);
    list_widget->setPermissions(isSiteAdmin(), isSiteAdmin());
    list_widget->setFilterText(tr("Seuls les types de séances associés au site sont affichés."));
    m_diag_editor->setCentralWidget(list_widget);

    m_diag_editor->setWindowTitle(tr("Types de séances"));
    m_diag_editor->setMinimumSize(size().width(), 2*size().height()/3);

    connect(m_diag_editor, &BaseDialog::finished, this, &SiteWidget::sessionTypesEditor_finished);
    m_diag_editor->open();
}


void SiteWidget::on_lstTestTypes_itemChanged(QListWidgetItem *item)
{
    if (!m_comManager->isCurrentUserSuperAdmin())
        return;

    // Check for changed items
    bool has_changes = false;
    if (m_listTestTypeSites_items.key(item) > 0 && item->checkState() == Qt::Unchecked){
        // Item deselected
        has_changes = true;
    }else{
        if (m_listTestTypeSites_items.key(item) <= 0 && item->checkState() == Qt::Checked){
            // Item selected
            has_changes = true;
        }
    }

    if (item->checkState() == Qt::Checked){
        item->setForeground(Qt::green);
    }else{
        item->setForeground(Qt::red);
    }

    ui->btnUpdateTestTypes->setEnabled(has_changes);
}


void SiteWidget::on_btnUpdateTestTypes_clicked()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonObject site_obj;
    QJsonArray tt_sites;
    bool removed_tts = false;

    site_obj.insert("id_site", m_data->getId());
    for (int i=0; i<ui->lstTestTypes->count(); i++){
        QListWidgetItem* item = ui->lstTestTypes->item(i);
        int tt_id = m_listTestTypes_items.key(item, 0);
        if (item->checkState() == Qt::Checked){
            // New item selected
            QJsonObject item_obj;
            item_obj.insert("id_test_type", tt_id);
            tt_sites.append(item_obj);
        }else{
            if (!removed_tts){
                if (m_listTestTypeSites_items.key(item, 0) > 0){
                    removed_tts = true;
                }
            }
        }
    }

    if (removed_tts){
        GlobalMessageBox msgbox;
        int rval = msgbox.showYesNo(tr("Suppression de types de tests associés"), tr("Au moins un type de test a été retiré de ce site. S'il y a des projets qui utilisent ce type, ils ne pourront plus l'utiliser.\nSouhaitez-vous continuer?"));
        if (rval != GlobalMessageBox::Yes){
            return;
        }
    }

    site_obj.insert("testtypes", tt_sites);
    base_obj.insert("site", site_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_TESTTYPESITE_PATH, document.toJson());
}


void SiteWidget::on_tabManageServices_currentChanged(int index)
{
    QWidget* current_tab = ui->tabManageServices->widget(index);

    if (dynamic_cast<DashboardsConfigWidget*>(current_tab)){
        dynamic_cast<DashboardsConfigWidget*>(current_tab)->refresh();
    }

    if (dynamic_cast<EmailServiceConfigWidget*>(current_tab)){
        dynamic_cast<EmailServiceConfigWidget*>(current_tab)->refresh();
    }
}

