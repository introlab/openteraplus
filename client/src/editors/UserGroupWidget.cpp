#include "UserGroupWidget.h"
#include "ui_UserGroupWidget.h"

#include "editors/DataListWidget.h"

UserGroupWidget::UserGroupWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::UserGroupWidget)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    // Limited by default
    m_limited = true;

    // Use base class to manage editing
    setEditorControls(ui->wdgUserGroup, ui->btnEdit, ui->frameSave, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    // Query forms definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_USER_GROUP));

    ui->wdgUserGroup->setComManager(m_comManager);
    setData(data);

    // Set default tabs
    ui->tabNav->setCurrentIndex(0);
    ui->tabUserGroupInfos->setCurrentIndex(0);
}

UserGroupWidget::~UserGroupWidget()
{
    delete ui;

}

void UserGroupWidget::saveData(bool signal)
{
    // UserGroup data
    QJsonDocument user_group_data = ui->wdgUserGroup->getFormDataJson(m_data->isNew());

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

    if (!dataIsNew()){
        // Loads first detailled informations tab
        on_tabUserGroupInfos_currentChanged(0);
    }else{
        ui->tabUserGroupInfos->setEnabled(false);
    }
}

void UserGroupWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &UserGroupWidget::processFormsReply);
    connect(m_comManager, &ComManager::siteAccessReceived, this, &UserGroupWidget::processSiteAccessReply);
    connect(m_comManager, &ComManager::projectAccessReceived, this, &UserGroupWidget::processProjectAccessReply);
    connect(m_comManager, &ComManager::usersReceived, this, &UserGroupWidget::processUsersReply);

    connect(ui->btnUpdateSitesRoles, &QPushButton::clicked, this, &UserGroupWidget::btnUpdateSiteAccess_clicked);
    connect(ui->btnUpdateProjectsRoles, &QPushButton::clicked, this, &UserGroupWidget::btnUpdateProjectAccess_clicked);

}

void UserGroupWidget::updateSiteAccess(const TeraData *access)
{
    QTableWidgetItem* item;
    int id_site = access->getFieldValue("id_site").toInt();
    QString site_role = access->getFieldValue("site_access_role").toString();
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
        combo_roles->setEnabled(!m_limited);
    }
}

void UserGroupWidget::updateProjectAccess(const TeraData *access)
{
    QTableWidgetItem* item;
    int id_project = access->getFieldValue("id_project").toInt();
    QString project_role = access->getFieldValue("project_access_role").toString();
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
        combo_roles->setEnabled(!m_limited);
    }

}

void UserGroupWidget::updateUser(const TeraData *user)
{
    QListWidgetItem* item;

    if (m_lstUsers_items.contains(user->getId())){
        item = m_lstUsers_items[user->getId()];
    }else{
        // New item
        item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_USER)), user->getName());
        ui->lstUsers->addItem(item);
        m_lstUsers_items[user->getId()] = item;
    }
    item->setText(user->getName());

}

void UserGroupWidget::updateControlsState()
{

}

void UserGroupWidget::updateFieldsValue()
{
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

void UserGroupWidget::processUsersReply(QList<TeraData> users, QUrlQuery reply_query)
{
    if (!m_data)
        return;

    // Check if that reply is for us or not.
    if (!reply_query.hasQueryItem(WEB_QUERY_ID_USER_GROUP))
        return;

    if (reply_query.queryItemValue(WEB_QUERY_ID_USER_GROUP).toInt() != m_data->getId())
        return;

    for (int i=0; i<users.count(); i++){
        updateUser(&users.at(i));
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

void UserGroupWidget::on_tabUserGroupInfos_currentChanged(int index)
{

    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_USER_GROUP, QString::number(m_data->getId()));

    QString tab_name = ui->tabUserGroupInfos->widget(index)->objectName();

    if (tab_name == "tabProjects"){
        // Projects
        if (m_tableProjects_items.isEmpty()){
            args.addQueryItem(WEB_QUERY_WITH_PROJECTS, "1");
            queryDataRequest(WEB_PROJECTACCESS_PATH, args);
        }
    }

    if (tab_name == "tabSites"){
        // Sites
        if (m_tableSites_items.isEmpty()){
            args.addQueryItem(WEB_QUERY_WITH_SITES, "1");
            queryDataRequest(WEB_SITEACCESS_PATH, args);
        }
    }

    if (tab_name == "tabUsers"){
        // Users
        if (m_lstUsers_items.isEmpty()){
            args.addQueryItem(WEB_QUERY_LIST, "1");
            queryDataRequest(WEB_USERINFO_PATH, args);
        }
    }

}
