#include "ProjectWidget.h"
#include "ui_ProjectWidget.h"

#include "editors/DataListWidget.h"

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

    // Query forms definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_PROJECT));

    ui->wdgProject->setComManager(m_comManager);
    setData(data);

    // Default display
    ui->tabNav->setCurrentIndex(0);
    ui->tabProjectInfos->setCurrentIndex(0);
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

    }else{
        ui->tabProjectInfos->setEnabled(false);
    }
}

void ProjectWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &ProjectWidget::processFormsReply);
    connect(m_comManager, &ComManager::projectAccessReceived, this, &ProjectWidget::processProjectAccessReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &ProjectWidget::processPostOKReply);

    connect(ui->btnUpdateRoles, &QPushButton::clicked, this, &ProjectWidget::btnUpdateAccess_clicked);
    //connect(ui->btnDevices, &QPushButton::clicked, this, &ProjectWidget::btnDevices_clicked);

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

void ProjectWidget::updateSessionType(const TeraData *st)
{
    int id_session_type = st->getId();
    if (m_listSessionTypes_items.contains(id_session_type)){
        QListWidgetItem* item = m_listSessionTypes_items[id_session_type];
        item->setText(st->getFieldValue("session_type_name").toString());
    }else{
        QListWidgetItem* item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_SESSION)), st->getFieldValue("session_type_name").toString());
        ui->lstSessions->addItem(item);
        m_listSessionTypes_items[id_session_type] = item;
    }
}

void ProjectWidget::updateControlsState()
{
    //ui->btnDevices->setVisible(!m_limited);
    ui->btnUpdateRoles->setVisible(!m_limited);

}

void ProjectWidget::updateFieldsValue()
{
    if (m_data){
        ui->wdgProject->fillFormFromData(m_data->toJson());
        ui->lblTitle->setText(m_data->getName());
    }
}

bool ProjectWidget::validateData()
{
    return ui->wdgProject->validateFormData();
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

void ProjectWidget::processSessionTypesProjectReply(QList<TeraData> stps)
{
    if (!m_data)
        return;

    for (int i=0; i<stps.count(); i++){
        if (stps.at(i).getFieldValue("id_project") == m_data->getId()){
            updateSessionType(&stps.at(i));
        }
    }
}

void ProjectWidget::processPostOKReply(QString path)
{
    if (path == WEB_PROJECTINFO_PATH){
        // Update current user access list for the newly created project
        m_comManager->doUpdateCurrentUser();
    }
}

void ProjectWidget::btnUpdateAccess_clicked()
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
            data_obj.insert("id_project", m_data->getId());
            data_obj.insert("id_user_group", user_group_id);
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

void ProjectWidget::btnDevices_clicked()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }
    m_diag_editor = new QDialog(this);
    DataListWidget* list_widget = new DataListWidget(m_comManager, TERADATA_DEVICE, m_diag_editor);
    Q_UNUSED(list_widget)

    m_diag_editor->setWindowTitle(tr("Appareils"));

    m_diag_editor->open();
}

void ProjectWidget::on_tabProjectInfos_currentChanged(int index)
{
    // Load data depending on selected tab
    QUrlQuery args;
    QString tab_name = ui->tabProjectInfos->widget(index)->objectName();

    if (tab_name == "tabGroups"){
        // Groups
        if (m_listGroups_items.isEmpty()){
            connect(m_comManager, &ComManager::groupsReceived, this, &ProjectWidget::processGroupsReply);

            args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getFieldValue("id_project").toInt()));
            args.addQueryItem(WEB_QUERY_LIST, "true");
            queryDataRequest(WEB_GROUPINFO_PATH, args);
        }
    }

    if (tab_name == "tabDevices"){
        // Devices
        if (m_listDevices_items.isEmpty()){
            connect(m_comManager, &ComManager::devicesReceived, this, &ProjectWidget::processDevicesReply);

            args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getFieldValue("id_site").toInt()));
            args.addQueryItem(WEB_QUERY_PARTICIPANTS, "");
            queryDataRequest(WEB_DEVICEINFO_PATH, args);
        }
    }

    if (tab_name == "tabUsers"){
        // Users
        if (m_tableUsers_items.isEmpty()){
            args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
            args.addQueryItem(WEB_QUERY_BY_USERS, "1");
            queryDataRequest(WEB_PROJECTACCESS_PATH, args);
        }
    }

    if (tab_name == "tabSessionTypes"){
        // Session types
        if (m_listSessionTypes_items.isEmpty()){
            connect(m_comManager, &ComManager::sessionTypesProjectsReceived, this, &ProjectWidget::processSessionTypesProjectReply);

            args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
            queryDataRequest(WEB_SESSIONTYPEPROJECT_PATH, args);
        }
    }

    if (tab_name == "tabUserGroups"){
        // User groups
        if (m_tableUserGroups_items.isEmpty()){
            // Query
            args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getId()));
            args.addQueryItem(WEB_QUERY_WITH_USERGROUPS, "1"); // Includes user groups without any access
            queryDataRequest(WEB_PROJECTACCESS_PATH, args);
        }
    }

}
