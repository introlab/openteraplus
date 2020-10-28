#include "UserGroupWidget.h"
#include "ui_UserGroupWidget.h"

#include "editors/DataListWidget.h"

UserGroupWidget::UserGroupWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::UserGroupWidget)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    // Use base class to manage editing
    setEditorControls(ui->wdgUserGroup, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    // Query forms definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_USER_GROUP));

    ui->wdgUserGroup->setComManager(m_comManager);
    setData(data);

    // Set default tabs
    ui->tabNav->setCurrentIndex(0);
}

UserGroupWidget::~UserGroupWidget()
{
    delete ui;

}

void UserGroupWidget::saveData(bool signal)
{
    // UserGroup data
    QJsonDocument user_group_data = ui->wdgUserGroup->getFormDataJson(m_data->isNew());

    if (m_data->isNew()){
        // For new user group, also send the project and site access at the same time
        QJsonObject base_obj = user_group_data.object();
        QJsonObject base_ug = base_obj["user_group"].toObject();

        // Sites
        QJsonArray sites;
        for (int i=0; i<m_tableSites_items.count(); i++){
            int site_id = m_tableSites_items.keys().at(i);
            int row = m_tableSites_items[site_id]->row();
            QComboBox* combo_roles = dynamic_cast<QComboBox*>(ui->tableSites->cellWidget(row,1));
            if (combo_roles->currentIndex()>0){
                QJsonObject data_obj;
                QJsonValue role = combo_roles->currentData().toString();
                data_obj.insert("id_site", site_id);
                data_obj.insert("site_access_role", role);
                sites.append(data_obj);
            }
        }

        // Projects
        QJsonArray projects;
        for (int i=0; i<m_tableProjects_items.count(); i++){
            int project_id = m_tableProjects_items.keys().at(i);
            int row = m_tableProjects_items[project_id]->row();
            QComboBox* combo_roles = dynamic_cast<QComboBox*>(ui->tableProjects->cellWidget(row,1));
            if (combo_roles->currentIndex()>0){
                QJsonObject data_obj;
                QJsonValue role = combo_roles->currentData().toString();
                data_obj.insert("id_project", project_id);
                data_obj.insert("project_access_role", role);
                projects.append(data_obj);
            }
        }

        if (!projects.isEmpty() || !sites.isEmpty()){
            if (!projects.isEmpty())
                base_ug.insert("user_group_projects_access", projects);
            if (!sites.isEmpty())
                base_ug.insert("user_group_sites_access", sites);
            base_obj.insert("user_group", base_ug);
            user_group_data.setObject(base_obj);
        }
    }

    //qDebug() << user_data.toJson();
    postDataRequest(WEB_USERGROUPINFO_PATH, user_group_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgUserGroup->getFormDataObject(TERADATA_USERGROUP);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void UserGroupWidget::setData(const TeraData *data)
{
    DataEditorWidget::setData(data);

}

void UserGroupWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &UserGroupWidget::processFormsReply);
    connect(m_comManager, &ComManager::siteAccessReceived, this, &UserGroupWidget::processSiteAccessReply);
    connect(m_comManager, &ComManager::projectAccessReceived, this, &UserGroupWidget::processProjectAccessReply);
    connect(m_comManager, &ComManager::sitesReceived, this, &UserGroupWidget::processSitesReply);
    connect(m_comManager, &ComManager::projectsReceived, this, &UserGroupWidget::processProjectsReply);
    connect(m_comManager, &ComManager::userUserGroupsReceived, this, &UserGroupWidget::processUserUserGroupsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &UserGroupWidget::processPostOKReply);

    connect(ui->btnUpdateSitesRoles, &QPushButton::clicked, this, &UserGroupWidget::btnUpdateSiteAccess_clicked);
    connect(ui->btnUpdateProjectsRoles, &QPushButton::clicked, this, &UserGroupWidget::btnUpdateProjectAccess_clicked);

}

void UserGroupWidget::updateSiteAccess(const TeraData *access)
{
    QTableWidgetItem* item;
    int id_site = access->getFieldValue("id_site").toInt();
    QString site_role;
    if (access->hasFieldName("site_access_role"))
        site_role = access->getFieldValue("site_access_role").toString();
    QComboBox* combo_roles;
    if (m_tableSites_items.contains(id_site)){
        item = m_tableSites_items[id_site];
        combo_roles = dynamic_cast<QComboBox*>(ui->tableSites->cellWidget(item->row(),1));
    }else{
        // Not there - must add the site and role
        ui->tableSites->setRowCount(ui->tableSites->rowCount()+1);
        int current_row = ui->tableSites->rowCount()-1;
        QTableWidgetItem* item = new QTableWidgetItem(QIcon(access->getIconFilenameForDataType(TERADATA_SITE)),
                                                      access->getFieldValue("site_name").toString());
        ui->tableSites->setItem(current_row, 0, item);
        combo_roles = buildRolesComboBox();
        ui->tableSites->setCellWidget(current_row, 1, combo_roles);
        m_tableSites_items.insert(id_site, item);
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
        combo_roles->setDisabled(false);

        if (access->hasFieldName("site_access_inherited")){
            if (access->getFieldValue("site_access_inherited").toBool()){
                // Inherited access - disable combobox
                //combo_roles->setDisabled(true);
                if (combo_roles->count() == 3)
                    combo_roles->removeItem(0); // Remove "no role" item
            }
        }

        // If not admin for that site, disable combo box
        if (!m_comManager->isCurrentUserSiteAdmin(id_site))
            combo_roles->setDisabled(true);
    }
}

void UserGroupWidget::updateProjectAccess(const TeraData *access)
{
    QTableWidgetItem* item;
    int id_project = access->getFieldValue("id_project").toInt();
    QString project_role;
    if (access->hasFieldName("project_access_role"))
        project_role = access->getFieldValue("project_access_role").toString();
    QComboBox* combo_roles;
    if (m_tableProjects_items.contains(id_project)){
        item = m_tableProjects_items[id_project];
        combo_roles = dynamic_cast<QComboBox*>(ui->tableProjects->cellWidget(item->row(),1));
    }else{
        // Not there - must add the site and role
        ui->tableProjects->setRowCount(ui->tableProjects->rowCount()+1);
        int current_row = ui->tableProjects->rowCount()-1;
        QTableWidgetItem* item = new QTableWidgetItem(QIcon(access->getIconFilenameForDataType(TERADATA_PROJECT)),
                                                      access->getFieldValue("project_name").toString());
        ui->tableProjects->setItem(current_row, 0, item);
        combo_roles = buildRolesComboBox();
        ui->tableProjects->setCellWidget(current_row, 1, combo_roles);
        m_tableProjects_items.insert(id_project, item);
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
        combo_roles->setDisabled(false);

        if (access->hasFieldName("project_access_inherited")){
            if (access->getFieldValue("project_access_inherited").toBool()){
                // Inherited access - disable combobox
                combo_roles->setDisabled(true);
            }
        }

        // If not admin for that project, disable combo box
        if (!m_comManager->isCurrentUserProjectAdmin(id_project))
            combo_roles->setDisabled(true);
    }

}

void UserGroupWidget::updateUserUserGroup(const TeraData *uug)
{
    int id_user_user_group = uug->getId();
    int id_user = uug->getFieldValue("id_user").toInt();
    QString user_name;
    if (uug->hasFieldName("user_fullname"))
        user_name = uug->getFieldValue("user_fullname").toString();

    QListWidgetItem* item;
    if (m_listUsers_items.contains(id_user)){
        item = m_listUsers_items[id_user];
    }else{
        // Must create a new item
        item = new QListWidgetItem(user_name);
        item->setIcon(QIcon(TeraData::getIconFilenameForDataType(TERADATA_USER)));
        ui->lstUsers->addItem(item);

        m_listUsers_items[id_user] = item;
    }

    if (id_user_user_group > 0){
        if (!m_listUsersUserGroups_items.contains(id_user_user_group)){
            m_listUsersUserGroups_items[id_user_user_group] = item;
        }
    }
    if (!user_name.isEmpty())
        item->setText(user_name);

    if (uug->getFieldValue("id_user_group").toInt() == m_data->getId()){
        item->setCheckState(Qt::Checked);
    }else{
        item->setCheckState(Qt::Unchecked);
    }
}

void UserGroupWidget::updateControlsState()
{

}

void UserGroupWidget::updateFieldsValue()
{

    if (dataIsNew() && ui->tabNav->count()>1){

        // Move sites / projects access list to first tab
        ui->tabSites->layout()->removeWidget(ui->tableSites);
        ui->tabProjects->layout()->removeWidget(ui->tableProjects);
        ui->tabDashboard->layout()->removeWidget(ui->frameButtons);

        QLabel* lbl = new QLabel(tr("Accès - Sites"));
        QFont labelFont;
        labelFont.setBold(true);
        labelFont.setPointSize(10);
        lbl->setFont(labelFont);

        ui->tabDashboard->layout()->addWidget(lbl);
        ui->tabDashboard->layout()->addWidget(ui->tableSites);

        lbl = new QLabel(tr("Accès - Projets"));
        lbl->setFont(labelFont);

        ui->tabDashboard->layout()->addWidget(lbl);
        ui->tabDashboard->layout()->addWidget(ui->tableProjects);
        ui->tabDashboard->layout()->addWidget(ui->frameButtons);
        while (ui->tabNav->count() > 1)
            ui->tabNav->removeTab(1);

        // Query sites and projects if needed
        if (m_tableSites_items.isEmpty()){
            queryDataRequest(WEB_SITEINFO_PATH);
        }
        if (m_tableProjects_items.isEmpty()){
            queryDataRequest(WEB_PROJECTINFO_PATH);
        }
    }

    if (m_data){
        ui->wdgUserGroup->fillFormFromData(m_data->toJson());
        ui->lblTitle->setText(m_data->getName());
    }


}

bool UserGroupWidget::validateData()
{
    return ui->wdgUserGroup->validateFormData();
}

void UserGroupWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type == WEB_FORMS_QUERY_USER_GROUP){
        ui->wdgUserGroup->buildUiFromStructure(data);
        return;
    }
}

void UserGroupWidget::processSiteAccessReply(QList<TeraData> access, QUrlQuery reply_query)
{
    if (!m_data)
        return;

    // Check if that reply is for us or not.
    if (!reply_query.hasQueryItem(WEB_QUERY_ID_USER_GROUP))
        return;

    if (reply_query.queryItemValue(WEB_QUERY_ID_USER_GROUP).toInt() != m_data->getId())
        return;

    for (int i=0; i<access.count(); i++) {
       updateSiteAccess(&access.at(i));
    }

}

void UserGroupWidget::processProjectAccessReply(QList<TeraData> access, QUrlQuery reply_query)
{
    if (!m_data)
        return;

    // Check if that reply is for us or not.
    if (!reply_query.hasQueryItem(WEB_QUERY_ID_USER_GROUP))
        return;

    if (reply_query.queryItemValue(WEB_QUERY_ID_USER_GROUP).toInt() != m_data->getId())
        return;

    for (int i=0; i<access.count(); i++) {
       updateProjectAccess(&access.at(i));
    }

}

void UserGroupWidget::processSitesReply(QList<TeraData> sites)
{
    for (int i=0; i<sites.count(); i++) {
       updateSiteAccess(&sites.at(i));
    }
}

void UserGroupWidget::processProjectsReply(QList<TeraData> projects)
{
    for (int i=0; i<projects.count(); i++) {
       updateProjectAccess(&projects.at(i));
    }
}

void UserGroupWidget::processUserUserGroupsReply(QList<TeraData> users_user_groups)
{
    for (int i=0; i<users_user_groups.count(); i++){
        updateUserUserGroup(&users_user_groups.at(i));
    }
}

void UserGroupWidget::processPostOKReply(QString path)
{
    if (path == TeraData::getPathForDataType(TERADATA_SITEACCESS)){
        // Reset "dirty" flag on each combo box role
        for (int i=0; i<m_tableSites_items.count(); i++){
           int site_id = m_tableSites_items.keys().at(i);
           int row = m_tableSites_items[site_id]->row();
           QComboBox* combo_roles = dynamic_cast<QComboBox*>(ui->tableSites->cellWidget(row,1));
           if (combo_roles){
               combo_roles->setProperty("original_index", combo_roles->currentIndex());
           }
        }
    }

    if (path == TeraData::getPathForDataType(TERADATA_PROJECTACCESS)){
        // Reset "dirty" flag on each combo box role
        for (int i=0; i<m_tableProjects_items.count(); i++){
           int project_id = m_tableProjects_items.keys().at(i);
           int row = m_tableProjects_items[project_id]->row();
           QComboBox* combo_roles = dynamic_cast<QComboBox*>(ui->tableProjects->cellWidget(row,1));
           if (combo_roles){
               combo_roles->setProperty("original_index", combo_roles->currentIndex());
           }
        }
    }
}

void UserGroupWidget::btnUpdateSiteAccess_clicked()
{

    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray roles;

    for (int i=0; i<m_tableSites_items.count(); i++){
        int site_id = m_tableSites_items.keys().at(i);
        int row = m_tableSites_items[site_id]->row();
        QComboBox* combo_roles = dynamic_cast<QComboBox*>(ui->tableSites->cellWidget(row,1));
        if (combo_roles->property("original_index").toInt() != combo_roles->currentIndex()){
            QJsonObject data_obj;
            // Ok, value was modified - must add!
            QJsonValue role = combo_roles->currentData().toString();
            data_obj.insert("id_site", site_id);
            data_obj.insert("id_user_group", m_data->getId());
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

void UserGroupWidget::btnUpdateProjectAccess_clicked()
{

    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray roles;

    for (int i=0; i<m_tableProjects_items.count(); i++){
        int project_id = m_tableProjects_items.keys().at(i);
        int row = m_tableProjects_items[project_id]->row();
        QComboBox* combo_roles = dynamic_cast<QComboBox*>(ui->tableProjects->cellWidget(row,1));
        if (combo_roles->property("original_index").toInt() != combo_roles->currentIndex()){
            QJsonObject data_obj;
            // Ok, value was modified - must add!
            QJsonValue role = combo_roles->currentData().toString();
            data_obj.insert("id_project", project_id);
            data_obj.insert("id_user_group", m_data->getId());
            data_obj.insert("project_access_role", role);
            roles.append(data_obj);
        }
    }

    if (!roles.isEmpty()){
        base_obj.insert("project_access", roles);
        document.setObject(base_obj);
        postDataRequest(WEB_PROJECTACCESS_PATH, document.toJson());
    }

}

void UserGroupWidget::on_tabNav_currentChanged(int index)
{
    QUrlQuery args;

    QWidget* current_tab = ui->tabNav->widget(index);

    args.addQueryItem(WEB_QUERY_ID_USER_GROUP, QString::number(m_data->getId()));
    if (current_tab == ui->tabProjects){
        // Projects
        args.addQueryItem(WEB_QUERY_WITH_EMPTY, "1");
        queryDataRequest(WEB_PROJECTACCESS_PATH, args);
    }

    if (current_tab == ui->tabSites){
        args.addQueryItem(WEB_QUERY_WITH_EMPTY, "1");
        queryDataRequest(WEB_SITEACCESS_PATH, args);
    }

    if (current_tab == ui->tabUsers){
        // Users
        if (m_listUsers_items.isEmpty()){
            args.addQueryItem(WEB_QUERY_WITH_EMPTY, "1");
            queryDataRequest(WEB_USERUSERGROUPINFO_PATH, args);
        }
    }
}

void UserGroupWidget::on_btnUpdateUsers_clicked()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray user_usergroups;
    QList<int> todel_ids;

    for (int i=0; i<ui->lstUsers->count(); i++){
        QListWidgetItem* item = ui->lstUsers->item(i);
        if (m_listUsersUserGroups_items.values().contains(item)){
            if (item->checkState() == Qt::Unchecked){
                // Group was unselected
                todel_ids.append(m_listUsersUserGroups_items.key(item));
            }
        }else{
            if (item->checkState() == Qt::Checked){
                // New item selected
                QJsonObject item_obj;
                item_obj.insert("id_user", m_listUsers_items.key(item));
                item_obj.insert("id_user_group", m_data->getId());
                user_usergroups.append(item_obj);
            }
        }
     }

    // Delete queries
    for (int id_user_user_group:todel_ids){
        deleteDataRequest(WEB_USERUSERGROUPINFO_PATH, id_user_user_group);
    }

    // Update query
    if (!user_usergroups.isEmpty()){
        base_obj.insert("user_user_group", user_usergroups);
        document.setObject(base_obj);
        postDataRequest(WEB_USERUSERGROUPINFO_PATH, document.toJson());
    }
}
