#include "UserWidget.h"
#include "ui_UserWidget.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QtMultimedia/QCameraInfo>
#include <QtMultimedia/QCamera>
#include <QInputDialog>

#include <QtMultimedia/QAudioDeviceInfo>


UserWidget::UserWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::UserWidget)
{

    if (parent){
        ui->setupUi(parent);
    }else {
        ui->setupUi(this);
    }
    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    setLimited(false);
    ui->tabMain->setCurrentIndex(0);

    // Use base class to manage editing, but manually manage save button
    setEditorControls(ui->wdgUser, ui->btnEdit, ui->frameButtons, nullptr, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    // Query forms definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_USER_PROFILE));
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_USER));

    ui->wdgUser->setComManager(m_comManager);
    setData(data);

}

UserWidget::~UserWidget()
{    
    if (ui)
        delete ui;
}

void UserWidget::setData(const TeraData *data){
   DataEditorWidget::setData(data);

   // Query available user groups
   queryDataRequest(WEB_USERGROUPINFO_PATH);

   /* QString user_uuid = m_data->getFieldValue("user_uuid").toUuid().toString(QUuid::WithoutBraces);
    queryDataRequest(WEB_SITEINFO_PATH, QUrlQuery(QString(WEB_QUERY_USERUUID) + "=" + user_uuid));
    queryDataRequest(WEB_PROJECTINFO_PATH, QUrlQuery(QString(WEB_QUERY_USERUUID) + "=" + user_uuid));*/

}

void UserWidget::saveData(bool signal){

    // User Profile
    QString user_profile = ui->wdgProfile->getFormData(true);

    if (!ui->wdgUser->setFieldValue("user_profile", user_profile)){
        LOG_ERROR(tr("Field user_profile can't be set."), "UserWidget::saveData");
    }

    //QString user_data = ui->wdgUser->getFormData();
    // If data is new, we request all the fields.
    QJsonDocument user_data = ui->wdgUser->getFormDataJson(m_data->isNew());

    // Site access
    /*QJsonArray site_access = getSitesRoles();
    if (!site_access.isEmpty()){
        QJsonObject base_obj = user_data.object();
        QJsonObject base_user = base_obj["user"].toObject();
        base_user.insert("sites",site_access);
        base_obj.insert("user", base_user);
        user_data.setObject(base_obj);
    }

    // Project access
    QJsonArray project_access = getProjectsRoles();
    if (!project_access.isEmpty()){
        QJsonObject base_obj = user_data.object();
        QJsonObject base_user = base_obj["user"].toObject();
        base_user.insert("projects",project_access);
        base_obj.insert("user", base_user);
        user_data.setObject(base_obj);
    }*/

    //qDebug() << user_data.toJson();
    postDataRequest(WEB_USERINFO_PATH, user_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgUser->getFormDataObject(TERADATA_USER);
        new_data->setName(new_data->getFieldValue("user_firstname").toString() + " " + new_data->getFieldValue("user_lastname").toString());
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }

    /*if (parent())
        emit closeRequest(); // Ask to close that editor*/
}


void UserWidget::updateControlsState(){
    ui->wdgProfile->setEnabled(!isWaitingOrLoading());
    ui->tableRoles->setEnabled(!isWaitingOrLoading());
    ui->frameGroups->setEnabled(!isWaitingOrLoading());

    // Buttons update

    // Always show save button if editing current user
    if (m_limited){
        /*ui->btnSave->setVisible(true);
        ui->btnUndo->setVisible(true);*/
        editToggleClicked();
    }

    // Enable access editing
    bool allow_access_edit = !m_limited;
    if (m_data){
        bool user_is_superadmin = m_data->getFieldValue("user_superadmin").toBool();
        // Super admin can't be changed - they have access to everything!
        allow_access_edit &= !user_is_superadmin;
    }
    ui->frameGroups->setEnabled(allow_access_edit);
}

void UserWidget::updateFieldsValue(){
    if (m_data && !hasPendingDataRequests()){
        if (!ui->wdgUser->formHasData())
            ui->wdgUser->fillFormFromData(m_data->toJson());
        else {
            ui->wdgUser->resetFormValues();
        }
        if (!ui->wdgProfile->formHasData())
            ui->wdgProfile->fillFormFromData(m_data->getFieldValue("user_profile").toString());
        else{
            ui->wdgProfile->resetFormValues();
        }

        ui->lblTitle->setText(m_data->getName());

        // Don't allow editing of username if not new data
        if (!m_data->isNew()){
            ui->wdgUser->getWidgetForField("user_username")->setEnabled(false);
        }
    }
}

bool UserWidget::validateData(){
    bool valid = false;

    valid = ui->wdgUser->validateFormData();
    valid &= ui->wdgProfile->validateFormData();

    if (m_data->getId()==0){
        // New user - must check that a password is set
        if (ui->wdgUser->getFieldValue("user_password").toString().isEmpty()){
            valid = false;
        }
    }

    return valid;
}

void UserWidget::refreshUsersUserGroups()
{
    for (int i=0; i<m_listUserGroups_items.count(); i++){
        QListWidgetItem* item = m_listUserGroups_items.values().at(i);
        // Check item if the group is in the users groups list
        item->setCheckState(Qt::Unchecked);
        if (m_data->hasFieldName("user_groups")){
            QVariantList groups_list = m_data->getFieldValue("user_groups").toList();
            for (int j=0; j<groups_list.count(); j++){
                QVariantMap group_info = groups_list.at(j).toMap();
                if (group_info.contains("id_user_group")){
                    if (group_info["id_user_group"].toInt() == m_listUserGroups_items.keys().at(i)){
                        item->setCheckState(Qt::Checked);
                        break;
                    }
                }
            }
        }
        // Save initial state, to only update required items when saving
        item->setData(Qt::UserRole, item->checkState());
    }
}

void UserWidget::updateUserGroup(const TeraData *group)
{
    int id_user_group = group->getId();
    QListWidgetItem* item;
    if (m_listUserGroups_items.contains(id_user_group)){
        item = m_listUserGroups_items[id_user_group];
        item->setText(group->getName());
    }else{
        item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_USERGROUP)), group->getName());
        ui->lstGroups->addItem(item);
        m_listUserGroups_items[id_user_group] = item;
    }

}

void UserWidget::updateSiteAccess(const TeraData *site_access)
{
    // We assume the table is cleared beforehand and that item isn't already present.
    ui->tableRoles->insertRow(0);
    int current_row = 0; // Sites access are always added at the beginning
    QTableWidgetItem* item = new QTableWidgetItem(site_access->getFieldValue("site_name").toString());
    ui->tableRoles->setItem(current_row,0,item);
    item = new QTableWidgetItem(""); // Site access has an empty project name
    ui->tableRoles->setItem(current_row,1,item);
    item = new QTableWidgetItem(getRoleName(site_access->getFieldValue("site_role").toString()));
    ui->tableRoles->setItem(current_row,2,item);
}

void UserWidget::updateProjectAccess(const TeraData *project_access)
{
    // We assume the table is cleared beforehand and that item isn't already present.
    ui->tableRoles->setRowCount(ui->tableRoles->rowCount()+1);
    int current_row = ui->tableRoles->rowCount()-1;
    QTableWidgetItem* item = new QTableWidgetItem(project_access->getFieldValue("site_name").toString());
    ui->tableRoles->setItem(current_row,0,item);
    item = new QTableWidgetItem(project_access->getFieldValue("project_name").toString());
    ui->tableRoles->setItem(current_row,1,item);
    item = new QTableWidgetItem(getRoleName(project_access->getFieldValue("project_role").toString()));
    ui->tableRoles->setItem(current_row,2,item);
}

void UserWidget::processUsersReply(QList<TeraData> users)
{
    for (int i=0; i<users.count(); i++){
        if (users.at(i) == *m_data){
            // We found "ourself" in the list - update data.
            *m_data = users.at(i);
            updateFieldsValue();
            break;
        }
    }
    if (!hasPendingDataRequests())
        updateFieldsValue();
}

void UserWidget::processSitesAccessReply(QList<TeraData> sites)
{
    foreach (TeraData site, sites){
        updateSiteAccess(&site);
    }
}

void UserWidget::processProjectsAccessReply(QList<TeraData> projects)
{
    foreach (TeraData project, projects){
        updateProjectAccess(&project);
    }
}

void UserWidget::processUserGroupsReply(QList<TeraData> user_groups, QUrlQuery query)
{
    if (query.hasQueryItem(WEB_QUERY_ID_USER)){
        // The reply contains users groups for a user. For us?
        if (m_data->getId() == query.queryItemValue(WEB_QUERY_ID_USER).toInt()){
            // For this user - complete information in the TeraData object
            QVariantList groups;
            foreach(TeraData user_group, user_groups){
                groups.append(user_group.getFieldValues());
            }
            m_data->setFieldValue("user_groups", groups);
        }

    }
    foreach(TeraData user_group, user_groups){
        updateUserGroup(&user_group);
    }

    // Update selected user groups for that user
    refreshUsersUserGroups();
}

void UserWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type == WEB_FORMS_QUERY_USER){
        ui->wdgUser->buildUiFromStructure(data);
        // Disable some widgets if we are in limited mode (editing self profile)
        if (m_limited){
            // Disable some widgets
            QWidget* item = ui->wdgUser->getWidgetForField("user_username");
            if (item) item->setEnabled(false);
            item = ui->wdgUser->getWidgetForField("user_enabled");
            if (item) item->setEnabled(false);
            item = ui->wdgUser->getWidgetForField("user_superadmin");
            if (item) item->setEnabled(false);
            item = ui->wdgUser->getWidgetForField("user_notes");
            if (item) item->setEnabled(false);
            item = ui->wdgUser->getWidgetForField("id_user_group");
            if (item) item->setEnabled(false);
        }
        return;
    }

    if (form_type == WEB_FORMS_QUERY_USER_PROFILE){
        ui->wdgProfile->buildUiFromStructure(data);
        return;
    }
}

void UserWidget::postResultReply(QString path)
{
    // OK, data was saved!
    if (path==WEB_USERINFO_PATH){
        if (parent())
            emit closeRequest();
    }
}

void UserWidget::connectSignals()
{
    connect(m_comManager, &ComManager::usersReceived, this, &UserWidget::processUsersReply);
    connect(m_comManager, &ComManager::siteAccessReceived, this, &UserWidget::processSitesAccessReply);
    connect(m_comManager, &ComManager::projectAccessReceived, this, &UserWidget::processProjectsAccessReply);
    connect(m_comManager, &ComManager::formReceived, this, &UserWidget::processFormsReply);
    connect(m_comManager, &ComManager::userGroupsReceived, this, &UserWidget::processUserGroupsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &UserWidget::postResultReply);

}

void UserWidget::on_tabMain_currentChanged(int index)
{
    QUrlQuery args;
    if (index == 1){
        // User groups
        //if (!m_data->hasFieldName("user_groups")){
            // Update groups for user
            args.addQueryItem(WEB_QUERY_ID_USER, QString::number(m_data->getId()));
            queryDataRequest(WEB_USERGROUPINFO_PATH, args);
        //}

        // If user is super admin, disable groups
        bool super_admin = m_data->getFieldValue("user_superadmin").toBool();
        ui->lblWarning->setVisible(super_admin);
        ui->frameGroups->setVisible(!super_admin);
    }
    if (index == 2){
        // Roles
        ui->tableRoles->clearContents(); // Resets all elements in the table
        ui->tableRoles->setRowCount(0);
        ui->tableRoles->sortItems(-1);

        // Query sites and projects roles
        args.addQueryItem(WEB_QUERY_ID_USER, QString::number(m_data->getId()));

        queryDataRequest(WEB_SITEACCESS_PATH, args);
        args.addQueryItem(WEB_QUERY_WITH_SITES, "1");
        queryDataRequest(WEB_PROJECTACCESS_PATH, args);
    }
}

void UserWidget::on_btnUpdateGroups_clicked()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray roles;

    for (int i=0; i<m_listUserGroups_items.count(); i++){
        int user_group_id = m_listUserGroups_items.keys().at(i);
        QListWidgetItem* item = m_listUserGroups_items.values().at(i);
        if (item->data(Qt::UserRole).toInt() != item->checkState()){
            QJsonObject data_obj;
            // Ok, value was modified - must add!
            /*QJsonValue role = combo_roles->currentData().toString();
            data_obj.insert("id_site", m_data->getId());
            data_obj.insert("id_user", user_id);
            data_obj.insert("site_access_role", role);
            roles.append(data_obj);*/
        }
    }

    /*if (!roles.isEmpty()){
        base_obj.insert("site_access", roles);
        document.setObject(base_obj);
        postDataRequest(WEB_SITEACCESS_PATH, document.toJson());
    }*/
}
