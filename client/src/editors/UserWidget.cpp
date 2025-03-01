#include "UserWidget.h"
#include "ui_UserWidget.h"

#include <QtMultimedia/QCameraDevice>
#include <QtMultimedia/QCamera>
#include <QInputDialog>

#include <QtMultimedia/QAudioDevice>

#include "dialogs/PasswordStrengthDialog.h"
#include "ServiceConfigWidget.h"
#include "GlobalMessageBox.h"


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

    m_passwordJustGenerated = false;

    setLimited(false);

    // Use base class to manage editing, but manually manage save button
    setEditorControls(ui->wdgUser, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    // Query forms definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_USER));

    // Query roles (needed to check what is visible to the current user)
    queryUserAccess();

    ui->wdgUser->setComManager(m_comManager);
    UserWidget::setData(data);

    // Init UI
    initUI();
}

UserWidget::~UserWidget()
{    
    if (ui)
        delete ui;
}

void UserWidget::setData(const TeraData *data){
   DataEditorWidget::setData(data);

   // Query available user groups
   if (!m_data->hasFieldName("id_site"))
        queryDataRequest(WEB_USERGROUPINFO_PATH);
   else{
       // We have a new user with a specific id_site - we must query the correct user groups then
       QUrlQuery args;
       args.addQueryItem(WEB_QUERY_ID_SITE, m_data->getFieldValue("id_site").toString());
       m_data->removeFieldName("id_site");
       queryDataRequest(WEB_USERGROUPINFO_PATH, args);
   }


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

    if (*m_data == m_comManager->getCurrentUser()){
        m_currentUserPasswordChanged = ui->wdgUser->getFieldDirty("user_password");
    }else{
        m_currentUserPasswordChanged = false;
    }

    //qDebug() << user_data.toJson();
    postDataRequest(WEB_USERINFO_PATH, user_data.toJson());


    if (signal){
        TeraData* new_data = ui->wdgUser->getFormDataObject(TERADATA_USER);
        new_data->setName(new_data->getFieldValue("user_firstname").toString() + " " + new_data->getFieldValue("user_lastname").toString());
        *m_data = *new_data;
        delete new_data;
        if (!m_currentUserPasswordChanged)
            // If current password was changed, we will process it in the user replies
            emit dataWasChanged();
    }

    /*if (parent())
        emit closeRequest(); // Ask to close that editor*/
}


void UserWidget::updateControlsState(){
    ui->tableProjectsRoles->setEnabled(!isWaitingOrLoading());
    ui->tableSitesRoles->setEnabled(!isWaitingOrLoading());
    ui->frameGroups->setEnabled(!isWaitingOrLoading());

    // Buttons update

    // Always show save button if editing current user
    if (m_limited){
        editToggleClicked();
    }

    // Enable access editing
    bool allow_access_edit = !m_limited;
    bool is_editing_current_user = m_data->getId() == m_comManager->getCurrentUser().getId();
    if (m_data){
        bool user_is_superadmin = m_data->getFieldValue("user_superadmin").toBool();
        // Super admin can't be changed - they have access to everything!
        allow_access_edit &= !user_is_superadmin;

        if (is_editing_current_user){
            ui->wdgUser->hideFields(QStringList() << "user_force_password_change" << "user_2fa_otp_enabled" << "user_2fa_email_enabled");
            ui->wdgUser->setFieldEnabled("user_2fa_enabled", !m_data->getFieldValue("user_2fa_enabled").toBool());
        }

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
    }else{
        if (ui->tabMain->indexOf(ui->tabLogins) < 0){
            // Check if current user is site admin in at least a site of that user... or is self!
            bool has_site_admin_access = is_editing_current_user || m_comManager->isCurrentUserSuperAdmin();
            if (!has_site_admin_access){
                // Check if we are admin in a list one site
                QList<int> id_sites = m_userSitesRole.keys();
                for(int id_site:std::as_const(id_sites)){
                    if (m_comManager->isCurrentUserSiteAdmin(id_site)){
                        has_site_admin_access = true;
                        break;
                    }
                }
            }

            if (has_site_admin_access){
                ui->tabMain->addTab(ui->tabLogins, QIcon(":/icons/password.png"), tr("Journal d'accès"));
            }else{
                ui->wdgUser->hideFields(QStringList() << "user_force_password_change" << "user_2fa_otp_enabled" << "user_2fa_email_enabled");
                ui->wdgUser->setFieldEnabled("user_2fa_enabled", false);
            }

            if (m_data){
                ui->btnReset2FA->setVisible(m_data->getFieldValue("user_2fa_enabled").toBool());
            }
        }

    }
}

void UserWidget::updateFieldsValue(){
    if (m_data && !hasPendingDataRequests()){
        //if (!ui->wdgUser->formHasData())
            ui->wdgUser->fillFormFromData(m_data->toJson());
        /*else {
            ui->wdgUser->resetFormValues();
        }*/

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
    for(QListWidgetItem* item: std::as_const(m_listUserGroups_items)){
        if (std::find(m_listUserUserGroups_items.cbegin(), m_listUserUserGroups_items.cend(), item) != m_listUserUserGroups_items.cend()){
        //if (m_listUserUserGroups_items.contains(item)){
            item->setCheckState(Qt::Checked);
            item->setHidden(false);
        }else{
            item->setCheckState(Qt::Unchecked);
            if (m_limited){
                item->setHidden(true);
            }
        }
        // Save initial state, to only update required items when saving
        item->setData(Qt::UserRole, item->checkState());


    }

}

QJsonArray UserWidget::getSelectedGroupsAsJsonArray()
{
    QJsonArray user_groups;
    for(QListWidgetItem* item: std::as_const(m_listUserGroups_items)){
        int user_group_id = m_listUserGroups_items.key(item);
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
    ui->wdgPrefs->setEnabled(true);

    // Set preferences with current ones, for now
    ui->wdgPrefs->setFieldValue("notify_sounds", m_comManager->getCurrentPreferences().isNotifySounds());
    ui->wdgPrefs->setFieldValue("language", m_comManager->getCurrentPreferences().getLanguage());

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
        if (m_limited){
            item->setHidden(true);
        }
    }

}

void UserWidget::updateSiteAccess(const TeraData *site_access)
{
    // We assume the table is cleared beforehand and that item isn't already present.
    ui->tableSitesRoles->setRowCount(ui->tableSitesRoles->rowCount()+1);
    int current_row = ui->tableSitesRoles->rowCount()-1;
    QTableWidgetItem* item = new QTableWidgetItem(site_access->getFieldValue("site_name").toString());
    item->setIcon(QIcon(TeraData::getIconFilenameForDataType(TERADATA_SITE)));
    ui->tableSitesRoles->setItem(current_row,0,item);
    item = new QTableWidgetItem(getRoleName(site_access->getFieldValue("site_access_role").toString()));
    ui->tableSitesRoles->setItem(current_row,1,item);

    m_userSitesRole[site_access->getFieldValue("id_site").toInt()] = site_access->getFieldValue("site_access_role").toString();
}

void UserWidget::updateProjectAccess(const TeraData *project_access)
{
    // We assume the table is cleared beforehand and that item isn't already present.
    ui->tableProjectsRoles->setRowCount(ui->tableProjectsRoles->rowCount()+1);
    int current_row = ui->tableProjectsRoles->rowCount()-1;
    QTableWidgetItem* item = new QTableWidgetItem(project_access->getFieldValue("site_name").toString());
    ui->tableProjectsRoles->setItem(current_row,1,item);
    item = new QTableWidgetItem(project_access->getFieldValue("project_name").toString());
    item->setIcon(QIcon(TeraData::getIconFilenameForDataType(TERADATA_PROJECT)));
    ui->tableProjectsRoles->setItem(current_row,0,item);
    item = new QTableWidgetItem(getRoleName(project_access->getFieldValue("project_access_role").toString()));
    ui->tableProjectsRoles->setItem(current_row,2,item);
}

void UserWidget::updateServiceAccess(const TeraData *service_access)
{
    // We assume the table is cleared beforehand and that item isn't already present.
    int current_row = ui->tableServicesRoles->rowCount();
    ui->tableServicesRoles->setRowCount(ui->tableServicesRoles->rowCount()+1);
    QTableWidgetItem* item = new QTableWidgetItem(service_access->getFieldValue("service_name").toString());
    item->setIcon(QIcon(TeraData::getIconFilenameForDataType(TERADATA_SERVICE)));
    ui->tableServicesRoles->setItem(current_row,0,item);
    item = new QTableWidgetItem(getRoleName(service_access->getFieldValue("service_access_role_name").toString()));
    ui->tableServicesRoles->setItem(current_row,1,item);

}

void UserWidget::queryUserAccess()
{
    // Roles
    m_userSitesRole.clear();
    ui->tableProjectsRoles->clearContents(); // Resets all elements in the tables
    ui->tableProjectsRoles->setRowCount(0);
    ui->tableProjectsRoles->sortItems(-1);
    ui->tableSitesRoles->clearContents();
    ui->tableSitesRoles->setRowCount(0);
    ui->tableSitesRoles->sortItems(-1);
    ui->tableServicesRoles->clearContents();
    ui->tableServicesRoles->setRowCount(0);
    ui->tableServicesRoles->sortItems(-1);

    // Query sites and projects roles
    if (!m_data->isNew()){
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_USER, QString::number(m_data->getId()));

        queryDataRequest(WEB_SITEACCESS_PATH, args);

        queryDataRequest(WEB_SERVICEACCESSINFO_PATH, args);

        args.addQueryItem(WEB_QUERY_WITH_SITES, "1");
        queryDataRequest(WEB_PROJECTACCESS_PATH, args);

    }
}

bool UserWidget::validateUserGroups()
{
    //if (!m_comManager->isCurrentUserSuperAdmin()){
        bool at_least_one_selected = false;
        //for (int i=0; i<m_listUserGroups_items.count(); i++){
        for (QListWidgetItem* item: std::as_const(m_listUserGroups_items)){
            if (item->checkState() == Qt::Checked){
                at_least_one_selected = true;
                break;
            }
        }
        if (!at_least_one_selected && !ui->wdgUser->getFieldValue("user_superadmin").toBool()){
            // Warning: that user not having any user group meaning that it will be not available to the current user
            GlobalMessageBox msgbox;
            msgbox.showError(tr("Attention"), tr("Aucun groupe utilisateur n'a été spécifié.\nVous devez spécifier au moins un groupe utilisateur"));
            return false;
        }
    //}
    return true;
}

void UserWidget::processUsersReply(QList<TeraData> users)
{
    for (int i=0; i<users.count(); i++){
        if (users.at(i) == *m_data){
            // Update password if it was changed
            if (m_currentUserPasswordChanged){
                m_comManager->setCredentials(m_data->getFieldValue("user_username").toString(),
                                             ui->wdgUser->getFieldValue("user_password").toString());
                m_currentUserPasswordChanged = false;
                emit dataWasChanged();
            }
            // We found "ourself" in the list - update data.
            //*m_data = users.at(i);
            m_data->updateFrom(users.at(i));
            //updateFieldsValue();
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
    //ui->tableSitesRoles->resizeColumnsToContents();
}

void UserWidget::processProjectsAccessReply(QList<TeraData> projects)
{
    foreach (TeraData project, projects){
        updateProjectAccess(&project);
    }
    //ui->tableProjectsRoles->resizeColumnsToContents();
}

void UserWidget::processServicesAccessReply(QList<TeraData> services_access)
{
    foreach (TeraData access, services_access){
        // Ignore "OpenTera" service
        if (access.getFieldValue("service_key").toString() == "OpenTeraServer")
            continue;
        updateServiceAccess(&access);
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
        if (!ui->wdgUser->formHasStructure()){
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
            if (!m_comManager->isCurrentUserSuperAdmin())
                ui->wdgUser->hideField("user_superadmin");
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
    connect(m_comManager, &ComManager::servicesAccessReceived, this, &UserWidget::processServicesAccessReply);
    connect(m_comManager, &ComManager::formReceived, this, &UserWidget::processFormsReply);
    connect(m_comManager, &ComManager::userGroupsReceived, this, &UserWidget::processUserGroupsReply);
    connect(m_comManager, &ComManager::userUserGroupsReceived, this, &UserWidget::processUserUsersGroupsReply);
    connect(m_comManager, &ComManager::userPreferencesReceived, this, &UserWidget::processUserPrefsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &UserWidget::postResultReply);
    connect(ui->wdgUser,  &TeraForm::widgetValueHasChanged, this, &UserWidget::userFormValueChanged);
    connect(ui->wdgUser,  &TeraForm::widgetValueHasFocus, this, &UserWidget::userFormValueHasFocus);

}

void UserWidget::initUI()
{
    ui->tabMain->setCurrentIndex(0);

    ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabLogins)); // Remove logs tab for now, will be readded if access is sufficient

    // Configure log view
    ui->wdgLogins->setComManager(m_comManager);
    ui->wdgLogins->setViewMode(LogViewWidget::VIEW_LOGINS_USER, m_data->getUuid());
    ui->wdgLogins->setUuidName(m_data->getUuid(), m_data->getName());
}

void UserWidget::on_tabMain_currentChanged(int index)
{
    QUrlQuery args;
    bool super_admin = m_data->getFieldValue("user_superadmin").toBool();

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
        ui->lblWarning->setVisible(super_admin);
        ui->frameGroups->setVisible(!super_admin);
    }
    if (current_tab == ui->tabRoles){
        queryUserAccess();
        // If user is super admin, disable services
        ui->lblWarning2->setVisible(super_admin);
        ui->tableServicesRoles->setVisible(!super_admin);
    }

    if (current_tab == ui->tabConfig){
        // Service config
        if (!ui->wdgServiceConfig->layout()){
            QHBoxLayout* layout = new QHBoxLayout();
            layout->setContentsMargins(0,0,0,0);
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

    if (current_tab == ui->tabLogins){
        ui->wdgLogins->refreshData();
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

    //for (int i=0; i<m_listUserGroups_items.count(); i++){
    for (QListWidgetItem* item: std::as_const(m_listUserGroups_items)){
        // Build json list of user and groups
        if (item->checkState()==Qt::Checked){
            QJsonObject item_obj;
            item_obj.insert("id_user", m_data->getId());
            item_obj.insert("id_user_group", m_listUserGroups_items.key(item));
            groups.append(item_obj);
        }else{
            int id_user_user_group = m_listUserUserGroups_items.key(item,-1);
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

    if (widget == ui->wdgUser->getWidgetForField("user_password")){
        QString current_pass = value.toString();
        if (!current_pass.isEmpty() && !m_passwordJustGenerated){
            // Show password dialog
            PasswordStrengthDialog dlg(current_pass);
            //QLineEdit* wdg_editor = dynamic_cast<QLineEdit*>(ui->wdgUser->getWidgetForField("user_password"));
            //dlg.setCursorPosition(wdg_editor->cursorPosition());

            if (dlg.exec() == QDialog::Accepted){
                m_passwordJustGenerated = true;
                ui->wdgUser->setFieldValue("user_password", dlg.getCurrentPassword());
            }else{
                ui->wdgUser->setFieldValue("user_password", "");
                //wdg_editor->undo();
            }

        }else{
            if (m_passwordJustGenerated)
                m_passwordJustGenerated = false;
        }
    }

    if (widget == ui->wdgUser->getWidgetForField("user_2fa_enabled")){
        if (m_data){
            if (value.toBool() && !m_data->getFieldValue("user_2fa_enabled").toBool()){
                GlobalMessageBox msgbox;
                if (msgbox.showYesNo(tr("Authentification multi-facteurs"),
                                     tr("Activer la double authentification forcera une configuration de l'utilisateur lors de la prochaine connexion.\n\nVoulez-vous continuer?")) == GlobalMessageBox::No){
                    ui->wdgUser->setFieldValue("user_2fa_enabled", false);
                }
            }
        }
    }
    if (widget == ui->wdgUser->getWidgetForField("user_2fa_otp_enabled")){
        if (m_data){
            if (!value.toBool() && m_data->getFieldValue("user_2fa_otp_enabled").toBool()){
                GlobalMessageBox msgbox;
                if (msgbox.showYesNo(tr("Authentification multi-facteurs"),
                                     tr("Désactiver l'authentification par OTP forcera une reconfiguration de l'utilisateur lors de la prochaine connexion.\n\nVoulez-vous continuer?")) == GlobalMessageBox::No){
                    ui->wdgUser->setFieldValue("user_2fa_otp_enabled", true);
                }
            }
        }
    }
}

void UserWidget::userFormValueHasFocus(QWidget *widget)
{
    if (widget == ui->wdgUser->getWidgetForField("user_password")){
        if (!m_passwordJustGenerated){
            // Show password dialog
            QString current_pass = ui->wdgUser->getFieldValue("user_password").toString();
            PasswordStrengthDialog dlg(current_pass);
            //QLineEdit* wdg_editor = dynamic_cast<QLineEdit*>(ui->wdgUser->getWidgetForField("user_password"));
            //dlg.setCursorPosition(wdg_editor->cursorPosition());

            if (dlg.exec() == QDialog::Accepted){
                m_passwordJustGenerated = true;
                ui->wdgUser->setFieldValue("user_password", dlg.getCurrentPassword());
            }else{
                ui->wdgUser->setFieldValue("user_password", "");
                //wdg_editor->undo();
            }

        }else{
            if (m_passwordJustGenerated)
                m_passwordJustGenerated = false;
        }
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

/*void UserWidget::on_btnGeneratePassword_clicked()
{
    // Show random password dialog
    GeneratePasswordDialog dlg;

    if (dlg.exec() == QDialog::Accepted){
        QString password = dlg.getPassword();
        m_passwordJustGenerated = true;
        ui->wdgUser->setFieldValue("user_password", password);
    }
}*/

void UserWidget::editToggleClicked()
{
    DataEditorWidget::editToggleClicked();
}

void UserWidget::saveButtonClicked()
{
    DataEditorWidget::saveButtonClicked();

}

void UserWidget::undoButtonClicked()
{
    DataEditorWidget::undoButtonClicked();

}

void UserWidget::on_btnReset2FA_clicked()
{
    GlobalMessageBox msg;
    if (msg.showYesNo(tr("Réinitialiser code authentification"), tr("Cette action forcera l'utilisateur à reconfigurer ses paramètres d'authentification.\n\nVoulez-vous continuer?")) == GlobalMessageBox::Yes){
        QJsonDocument document;
        QJsonObject data_obj;
        QJsonObject base_obj;

        data_obj.insert("id_user", QJsonValue::fromVariant(ui->wdgUser->getFieldValue("id_user")));
        data_obj.insert("user_2fa_otp_secret", QJsonValue::fromVariant(""));

        base_obj.insert(TeraData::getDataTypeName(TERADATA_USER), data_obj);
        document.setObject(base_obj);

        postDataRequest(WEB_USERINFO_PATH, document.toJson());

    }
}

