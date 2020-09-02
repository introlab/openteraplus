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
    setEditorControls(ui->wdgUser, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    // Query forms definition
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

    //QString user_data = ui->wdgUser->getFormData();
    // If data is new, we request all the fields.
    QJsonDocument user_data = ui->wdgUser->getFormDataJson(m_data->isNew());

    if (m_data->isNew() && ui->lstGroups->isEnabled()){
        QJsonObject base_obj = user_data.object();
        QJsonObject base_user = base_obj["user"].toObject();
        QJsonArray groups = getSelectedGroupsAsJsonArray();
        // For new users, also sends the user groups at the same time
        if (!groups.isEmpty()){
            base_user.insert("user_user_groups", groups);
            base_obj.insert("user", base_user);
            user_data.setObject(base_obj);
        }
    }

    //qDebug() << user_data.toJson();


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

    if (dataIsNew() && ui->tabMain->count()>1){
        // Move user groups list to first tab
        ui->frameGroups->layout()->removeWidget(ui->lstGroups);
        ui->mainWidget->layout()->removeWidget(ui->frameButtons);

        QLabel* lblGroups = new QLabel(tr("Groupes utilisateurs"));
        QFont labelFont;
        labelFont.setBold(true);
        labelFont.setPointSize(10);
        lblGroups->setFont(labelFont);

        ui->mainWidget->layout()->addWidget(lblGroups);
        ui->mainWidget->layout()->addWidget(ui->lstGroups);
        ui->mainWidget->layout()->addWidget(ui->frameButtons);
        while (ui->tabMain->count() > 1)
            ui->tabMain->removeTab(1);
    }
}

void UserWidget::updateFieldsValue(){
    if (m_data && !hasPendingDataRequests()){
        if (!ui->wdgUser->formHasData())
            ui->wdgUser->fillFormFromData(m_data->toJson());
        else {
            ui->wdgUser->resetFormValues();
        }

        ui->lblTitle->setText(m_data->getName());

        // Don't allow editing of username if not new data
        if (!m_data->isNew()){
            ui->wdgUser->getWidgetForField("user_username")->setEnabled(false);
        }else{
            // Require password for new data
            ui->wdgUser->setFieldRequired("user_password", true);
        }
    }
}

bool UserWidget::validateData(){
    if (dataIsNew()){
        // For new data, if not super admin, at least one user group must be specified
        if (!validateUserGroups())
            return false;
    }
    return ui->wdgUser->validateFormData();
}

void UserWidget::refreshUsersUserGroups()
{
    for (int i=0; i<m_listUserGroups_items.count(); i++){
        QListWidgetItem* item = m_listUserGroups_items.values().at(i);
        // Check item if the group is in the users groups list
        if (m_listUserUserGroups_items.values().contains(item)){
            item->setCheckState(Qt::Checked);
        }else{
            item->setCheckState(Qt::Unchecked);
        }
        /*if (m_data->hasFieldName("user_groups")){
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
        }*/
        // Save initial state, to only update required items when saving
        item->setData(Qt::UserRole, item->checkState());
    }
}

QJsonArray UserWidget::getSelectedGroupsAsJsonArray()
{
    QJsonArray user_groups;
    for (int i=0; i<m_listUserGroups_items.count(); i++){
        int user_group_id = m_listUserGroups_items.keys().at(i);
        QListWidgetItem* item = m_listUserGroups_items.values().at(i);
        if (item->checkState() == Qt::Checked){
            QJsonObject data_obj;
            data_obj.insert("id_user_group", user_group_id);
            user_groups.append(data_obj);
        }
    }
    return user_groups;
}

void UserWidget::buildUserPreferencesWidget()
{

    // TODO: Use "generator" to build structure such as what will be needed for "tests".
    QString json_form_structure = "{\"objecttype\": \"user_preference\","
                                  "\"sections\": ["
                                    "{"
                                      "\"_order\": 1,"
                                      "\"id\": \"main_prefs\","
                                      "\"items\": ["
                                        "{"
                                          "\"_order\": 1,"
                                          "\"id\": \"language\","
                                          "\"label\": \"" + tr("Langue de l'interface") + "\","
                                          "\"type\": \"array\","
                                          "\"values\": ["
                                            "{"
                                              "\"id\": \"fr\","
                                              "\"value\": \"" + tr("Français") + "\""
                                            "},"
                                            "{"
                                              "\"id\": \"en\","
                                              "\"value\": \"" + tr("Anglais") + "\""
                                            "}"
                                          "]"
                                        "},"
                                        "{"
                                          "\"_order\": 2,"
                                          "\"id\": \"notify_sounds\","
                                          "\"label\": \"" + tr("Sons lors des notifications") + "\","
                                          "\"type\": \"boolean\""
                                        "}"
                                      "],"
                                      "\"label\": \"" + tr("Préférences") + "\""
                                    "}"
                                  "]"
                                "}";
    ui->wdgPrefs->buildUiFromStructure(json_form_structure);
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
        item->setCheckState(Qt::Unchecked);
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
    item = new QTableWidgetItem(getRoleName(site_access->getFieldValue("site_access_role").toString()));
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
    item = new QTableWidgetItem(getRoleName(project_access->getFieldValue("project_access_role").toString()));
    ui->tableRoles->setItem(current_row,2,item);
}

bool UserWidget::validateUserGroups()
{
    if (!m_comManager->isCurrentUserSuperAdmin()){
        bool at_least_one_selected = false;
        for (int i=0; i<m_listUserGroups_items.count(); i++){
            if (m_listUserGroups_items.values().at(i)->checkState() == Qt::Checked){
                at_least_one_selected = true;
                break;
            }
        }
        if (!at_least_one_selected){
            // Warning: that user not having any user group meaning that it will be not available to the current user
            GlobalMessageBox msgbox;
            msgbox.showError(tr("Attention"), tr("Aucun groupe utilisateur n'a été spécifié.\nVous devez spécifier au moins un groupe utilisateur"));
            return false;
        }
    }
    return true;
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
    Q_UNUSED(query)
    foreach(TeraData user_group, user_groups){
        updateUserGroup(&user_group);
    }

}

void UserWidget::processUserUsersGroupsReply(QList<TeraData> user_users_groups, QUrlQuery query)
{
    Q_UNUSED(query)

    //m_listUserUserGroups_items.clear();

    // The reply contains users groups for a user. For us?

    // For this user - complete information in the TeraData object
    QVariantList groups;
    foreach(TeraData user_user_group, user_users_groups){
        if (m_data->getFieldValue("id_user").toInt() == user_user_group.getFieldValue("id_user").toInt()){
            groups.append(user_user_group.getFieldValues());
            // Keep relationship id
            m_listUserUserGroups_items[user_user_group.getId()] = m_listUserGroups_items[user_user_group.getFieldValue("id_user_group").toInt()];
        }
    }

    // Update selected user groups for that user
    refreshUsersUserGroups();

}

void UserWidget::processUserPrefsReply(QList<TeraData> user_prefs, QUrlQuery query)
{
    Q_UNUSED(query)

    foreach(TeraData pref, user_prefs){
        if (pref.getFieldValue("id_user").toInt() == m_data->getId() && pref.getFieldValue("user_preference_app_tag").toString() == APPLICATION_TAG){
            // Got the correct user preference
            ui->wdgPrefs->fillFormFromData(pref.getFieldValue("user_preference_preference").toString());
            break;
        }
    }
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
        }

        // Disable super admin field if not super admin itself!
        QWidget* item = ui->wdgUser->getWidgetForField("user_superadmin");
        if (item){
            item->setVisible(m_comManager->isCurrentUserSuperAdmin());

        }

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
    connect(m_comManager, &ComManager::userUserGroupsReceived, this, &UserWidget::processUserUsersGroupsReply);
    connect(m_comManager, &ComManager::userPreferencesReceived, this, &UserWidget::processUserPrefsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &UserWidget::postResultReply);
    connect(ui->wdgUser, &TeraForm::widgetValueHasChanged, this, &UserWidget::userFormValueChanged);

}

void UserWidget::on_tabMain_currentChanged(int index)
{
    QUrlQuery args;

    if (!ui->tabMain->currentWidget()->isEnabled())
        return;

    QWidget* current_tab = ui->tabMain->widget(index);

    if (current_tab == ui->tabGroups){
        // User groups
        //if (!m_data->hasFieldName("user_groups")){
            // Update groups for user
            args.addQueryItem(WEB_QUERY_ID_USER, QString::number(m_data->getId()));
            //queryDataRequest(WEB_USERGROUPINFO_PATH, args);
            queryDataRequest(WEB_USERUSERGROUPINFO_PATH, args);
        //}

        // If user is super admin, disable groups
        bool super_admin = m_data->getFieldValue("user_superadmin").toBool();
        ui->lblWarning->setVisible(super_admin);
        ui->frameGroups->setVisible(!super_admin);
    }
    if (current_tab == ui->tabRoles){
        // Roles
        ui->tableRoles->clearContents(); // Resets all elements in the table
        ui->tableRoles->setRowCount(0);
        ui->tableRoles->sortItems(-1);

        // Query sites and projects roles
        if (!m_data->isNew()){
            args.addQueryItem(WEB_QUERY_ID_USER, QString::number(m_data->getId()));

            queryDataRequest(WEB_SITEACCESS_PATH, args);
            args.addQueryItem(WEB_QUERY_WITH_SITES, "1");
            queryDataRequest(WEB_PROJECTACCESS_PATH, args);

        }
    }

    if (current_tab == ui->tabConfig){
        // Service config
        if (!ui->wdgServiceConfig->layout()){
            QHBoxLayout* layout = new QHBoxLayout();
            layout->setMargin(0);
            ui->wdgServiceConfig->setLayout(layout);
        }
        if (ui->wdgServiceConfig->layout()->count() == 0){
            ServiceConfigWidget* service_config_widget = new ServiceConfigWidget(m_comManager, m_data->getIdFieldName(), m_data->getId(), ui->wdgServiceConfig);
            ui->wdgServiceConfig->layout()->addWidget(service_config_widget);
        }
    }

    if (current_tab == ui->tabProfile){
        // User Preferences
        if (!ui->wdgPrefs->formHasStructure()){
            buildUserPreferencesWidget();

            // Query user preferences
            QUrlQuery args;
            args.addQueryItem(WEB_QUERY_ID_USER, QString::number(m_data->getId()));
            args.addQueryItem(WEB_QUERY_APPTAG, APPLICATION_TAG);
            queryDataRequest(WEB_USERPREFSINFO_PATH, args);
        }
    }
}

void UserWidget::on_btnUpdateGroups_clicked()
{

    if (!validateUserGroups())
        return;

    /*QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray user_groups = getSelectedGroupsAsJsonArray();


    QJsonObject user_obj;
    user_obj.insert("id_user", m_data->getId());
    user_obj.insert("user_groups", user_groups);
    base_obj.insert("user", user_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_USERINFO_PATH, document.toJson());*/
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray groups;
    QList<int> user_user_group_to_delete;

    for (int i=0; i<m_listUserGroups_items.count(); i++){
        // Build json list of user and groups
        if (m_listUserGroups_items.values().at(i)->checkState()==Qt::Checked){
            QJsonObject item_obj;
            item_obj.insert("id_user", m_data->getId());
            item_obj.insert("id_user_group", m_listUserGroups_items.keys().at(i));
            groups.append(item_obj);
        }else{
            int id_user_user_group = m_listUserUserGroups_items.key(m_listUserGroups_items.values().at(i),-1);
            if (id_user_user_group>=0){
                user_user_group_to_delete.append(id_user_user_group);
            }
        }
    }

    // Delete queries
    for (int id_user_user_group:user_user_group_to_delete){
        deleteDataRequest(WEB_USERUSERGROUPINFO_PATH, id_user_user_group);
        m_listUserUserGroups_items.remove(id_user_user_group);
    }

    // Update query
    if (!groups.isEmpty()){
        base_obj.insert("user_user_group", groups);
        document.setObject(base_obj);
        postDataRequest(WEB_USERUSERGROUPINFO_PATH, document.toJson());
    }

}

void UserWidget::userFormValueChanged(QWidget *widget, QVariant value)
{
    if (widget == ui->wdgUser->getWidgetForField("user_superadmin")){
        ui->lstGroups->setEnabled(!value.toBool());
    }
}

void UserWidget::on_btnUpdatePrefs_clicked()
{
    if (!m_data)
        return;

    QJsonDocument user_pref_data = ui->wdgPrefs->getFormDataJson(true);

    // Add base fields
    QJsonObject base_obj;
    QJsonObject base_user_pref = user_pref_data.object()["user_preference"].toObject();
    QJsonDocument user_pref_doc;
    QJsonObject user_pref_obj;
    user_pref_obj.insert("id_user", m_data->getId());
    user_pref_obj.insert("user_preference_app_tag", APPLICATION_TAG);
    user_pref_obj.insert("user_preference_preference", base_user_pref);
    base_obj.insert("user_preference", user_pref_obj);
    user_pref_doc.setObject(base_obj);

    postDataRequest(WEB_USERPREFSINFO_PATH, user_pref_doc.toJson());

}
