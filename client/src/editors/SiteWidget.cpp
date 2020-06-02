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
    setData(data);

    // Set default tabs
    ui->tabNav->setCurrentIndex(0);
    ui->tabSiteInfos->setCurrentIndex(0);
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
        on_tabSiteInfos_currentChanged(0);
    }else{
        ui->tabSiteInfos->setEnabled(false);
    }
}

void SiteWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &SiteWidget::processFormsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &SiteWidget::processPostOKReply);
    connect(m_comManager, &ComManager::siteAccessReceived, this, &SiteWidget::processSiteAccessReply);

    //connect(ui->btnUndo, &QPushButton::clicked, this, &SiteWidget::btnUndo_clicked);
    //connect(ui->btnSave, &QPushButton::clicked, this, &SiteWidget::btnSave_clicked);
    connect(ui->btnUpdateRoles, &QPushButton::clicked, this, &SiteWidget::btnUpdateAccess_clicked);
    //connect(ui->btnProjects, &QPushButton::clicked, this, &SiteWidget::btnProjects_clicked);
    connect(ui->btnDevices, &QPushButton::clicked, this, &SiteWidget::btnDevices_clicked);

}

void SiteWidget::updateUserSiteAccess(const TeraData *access)
{
    QTableWidgetItem* item;
    int id_user = access->getFieldValue("id_user").toInt();
    QString site_role = access->getFieldValue("site_access_role").toString();
    if (m_tableUsers_items.contains(id_user)){
        item = m_tableUsers_items[id_user];
        item->setText(DataEditorWidget::getRoleName(site_role));
    }else{
        // Not there - must add the user and role
        ui->tableUsers->setRowCount(ui->tableUsers->rowCount()+1);
        int current_row = ui->tableUsers->rowCount()-1;
        QTableWidgetItem* item = new QTableWidgetItem(QIcon(access->getIconFilenameForDataType(TERADATA_USER)),
                                                      access->getFieldValue("user_name").toString());
        ui->tableUsers->setItem(current_row, 0, item);
        item = new QTableWidgetItem(DataEditorWidget::getRoleName(site_role));
        ui->tableUsers->setItem(current_row, 1, item);
        m_tableUsers_items.insert(id_user, item); // Store QTableWidget Item that has the role name.
    }
}

void SiteWidget::updateUserGroupSiteAccess(const TeraData *access)
{
    QTableWidgetItem* item;
    int id_user_group = access->getFieldValue("id_user_group").toInt();
    QString site_role = access->getFieldValue("site_access_role").toString();
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
        index = combo_roles->findData(site_role);
        if (index >= 0){
            combo_roles->setCurrentIndex(index);
        }else{
            combo_roles->setCurrentIndex(0);
        }
        combo_roles->setProperty("original_index", index);
        combo_roles->setEnabled(!m_limited);

        if (access->hasFieldName("site_access_inherited")){
            if (access->getFieldValue("site_access_inherited").toBool()){
                // Inherited access - disable combobox
                combo_roles->setDisabled(true);
            }
        }
    }

}

void SiteWidget::updateProject(const TeraData *project)
{
    int id_project = project->getId();
    if (m_listProjects_items.contains(id_project)){
        QListWidgetItem* item = m_listProjects_items[id_project];
        item->setText(project->getName());
    }else{
        QListWidgetItem* item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_PROJECT)), project->getName());
        ui->lstProjects->addItem(item);
        m_listProjects_items[id_project] = item;
    }
}

void SiteWidget::updateDevice(const TeraData *device)
{
    int id_device = device->getId();

    // Create list of projects and participants
    QStringList projects;
    QStringList participants;
    if (device->hasFieldName("device_participants")){
        QVariantList part_list = device->getFieldValue("device_participants").toList();
        for (QVariant part:part_list){
            QVariantMap part_info = part.toMap();
            QString project_name = tr("Project inconnu");
            QString participant_name = "";
            if (part_info.contains("participant_name")){
                participant_name = part_info["participant_name"].toString();
            }
            if (part_info.contains("project_name")){
                project_name = part_info["project_name"].toString();
            }
            projects.append(project_name);
            participants.append(participant_name);
        }
    }

    // Build assignment string
    QString assignment="";
    for (int i=0; i<participants.count(); i++){
        if (i>0)
            assignment += ", ";
        assignment += participants.at(i);
        if (!participants.at(i).isEmpty())
            assignment += " (" + projects.at(i) + ")";
    }

    // Build device name, if empty
    QString device_name = device->getName();
    if (device_name.isEmpty())
        device_name = tr("(Appareil sans nom)");

    // Create / update values in table
    if (m_listDevices_items.contains(id_device)){
       QTableWidgetItem* item = m_listDevices_items[id_device];
       item->setText(device_name);
       ui->lstDevices->item(item->row(), 1)->setText(assignment);
    }else{
        ui->lstDevices->setRowCount(ui->lstDevices->rowCount()+1);
        QTableWidgetItem* item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_DEVICE)), device->getName());
        ui->lstDevices->setItem(ui->lstDevices->rowCount()-1, 0, item);
        m_listDevices_items[id_device] = item;

        item = new QTableWidgetItem(assignment);
        ui->lstDevices->setItem(ui->lstDevices->rowCount()-1, 1, item);
    }  

}

void SiteWidget::updateControlsState()
{
    ui->btnDevices->setVisible(!m_limited);
    //ui->btnProjects->setVisible(!m_limited);

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

    if (reply_query.hasQueryItem(WEB_QUERY_BY_USERS)){
        for (int i=0; i<access.count(); i++) {
           updateUserSiteAccess(&access.at(i));
        }
    }else{
        // User groups in reply
        for (int i=0; i<access.count(); i++) {
           updateUserGroupSiteAccess(&access.at(i));
        }
    }
}

void SiteWidget::processProjectsReply(QList<TeraData> projects)
{
    if (!m_data)
        return;

    for (int i=0; i<projects.count(); i++){
        if (projects.at(i).getFieldValue("id_site") == m_data->getFieldValue("id_site")){
            updateProject(&projects.at(i));
        }
    }
}

void SiteWidget::processDevicesReply(QList<TeraData> devices)
{
    if (!m_data)
        return;

    for (int i=0; i<devices.count(); i++){
        QVariantList sites_list = devices.at(i).getFieldValue("device_sites").toList();
        for (int j=0; j<sites_list.count(); j++){
            QVariantMap site_info = sites_list.at(j).toMap();
            if (site_info.contains("id_site")){
                if (site_info["id_site"].toInt() == m_data->getId()){
                    updateDevice(&devices.at(i));
                    break;
                }
            }
        }
    }

}

void SiteWidget::processPostOKReply(QString path)
{
    if (path == WEB_SITEINFO_PATH){
        // Update current user access list for the newly created site
        m_comManager->doUpdateCurrentUser();
    }
}

void SiteWidget::btnUpdateAccess_clicked()
{

    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray roles;

    for (int i=0; i<m_tableUserGroups_items.count(); i++){
        int user_group_id = m_tableUserGroups_items.keys().at(i);
        int row = m_tableUserGroups_items[user_group_id]->row();
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

void SiteWidget::btnProjects_clicked()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }

    m_diag_editor = new BaseDialog(this);
    DataListWidget* list_widget = new DataListWidget(m_comManager, TERADATA_PROJECT, nullptr);
    m_diag_editor->setCentralWidget(list_widget);

    m_diag_editor->setWindowTitle(tr("Projets"));

    m_diag_editor->open();
}

void SiteWidget::btnDevices_clicked()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }
    m_diag_editor = new BaseDialog(this);
    DataListWidget* list_widget = new DataListWidget(m_comManager, TERADATA_DEVICE, nullptr);
    m_diag_editor->setCentralWidget(list_widget);

    m_diag_editor->setWindowTitle(tr("Appareils"));
    m_diag_editor->setMinimumHeight(700);

    m_diag_editor->open();
}

void SiteWidget::on_tabSiteInfos_currentChanged(int index)
{

    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getFieldValue("id_site").toInt()));

    QString tab_name = ui->tabSiteInfos->widget(index)->objectName();

    if (tab_name == "tabProjects"){
        // Projects
        if (m_listProjects_items.isEmpty()){
            // Connect signal to receive updates
            connect(m_comManager, &ComManager::projectsReceived, this, &SiteWidget::processProjectsReply);

            // Query
            args.addQueryItem(WEB_QUERY_LIST, "true");
            queryDataRequest(WEB_PROJECTINFO_PATH, args);
        }
    }

    if (tab_name == "tabDevices"){
        // Devices
        if (m_listDevices_items.isEmpty()){
            // Connect signal to receive updates
            connect(m_comManager, &ComManager::devicesReceived, this, &SiteWidget::processDevicesReply);

            // Query
            args.addQueryItem(WEB_QUERY_PARTICIPANTS, "");
            args.addQueryItem(WEB_QUERY_SITES, "");
            queryDataRequest(WEB_DEVICEINFO_PATH, args);
        }
    }

    if (tab_name == "tabUsers"){
        // Users
        if (m_tableUsers_items.isEmpty()){
            // Query
            args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getId()));
            args.addQueryItem(WEB_QUERY_BY_USERS, "1");
            queryDataRequest(WEB_SITEACCESS_PATH, args);
        }
    }

    if (tab_name == "tabUserGroups"){
        // User groups
        if (m_tableUserGroups_items.isEmpty()){
            // Query
            args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getId()));
            args.addQueryItem(WEB_QUERY_WITH_USERGROUPS, "1"); // Includes user groups without any access
            queryDataRequest(WEB_SITEACCESS_PATH, args);
        }
    }

}
