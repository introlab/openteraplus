#include "ServiceWidget.h"
#include "ui_ServiceWidget.h"

ServiceWidget::ServiceWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::ServiceWidget)
{   

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    setLimited(false);

    // Use base class to manage editing
    setEditorControls(ui->wdgService, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    // Query form definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_SERVICE));

    ui->wdgService->setComManager(m_comManager);
    ServiceWidget::setData(data);

    ui->tabNav->setCurrentIndex(0);

}

ServiceWidget::~ServiceWidget()
{
    if (ui)
        delete ui;
}

void ServiceWidget::saveData(bool signal){

    // If data is new, we request all the fields.
    QJsonDocument service_data = ui->wdgService->getFormDataJson(m_data->isNew());

    postDataRequest(WEB_SERVICEINFO_PATH, service_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgService->getFormDataObject(TERADATA_SERVICE);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void ServiceWidget::updateControlsState(){

}

void ServiceWidget::updateFieldsValue(){
    if (m_data){
        ui->wdgService->fillFormFromData(m_data->toJson());
        ui->lblTitle->setText(m_data->getName());
    }
}

void ServiceWidget::updateServiceProject(TeraData *project)
{

    int id_project = project->getFieldValue("id_project").toInt();
    QListWidgetItem* item;
    QString project_name = project->getFieldValue("project_name").toString();

    if (m_listProjects_items.contains(id_project)){
        item = m_listProjects_items[id_project];
    }else{
        item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_PROJECT)), project_name);
        item->setCheckState(Qt::Unchecked);
        ui->lstProjects->addItem(item);
        m_listProjects_items[id_project] = item;
        //combo_roles = buildRolesComboBox(m_serviceRoles);
    }

    if (!project_name.isEmpty())
        item->setText(project_name);

    int service_id = project->getFieldValue("id_service").toInt();
    if (service_id > 0){
        item->setCheckState(Qt::Checked);
    }else{
        item->setCheckState(Qt::Unchecked);
    }
}

void ServiceWidget::updateServiceRole(const int& id_role, const QString& role_name)
{
    QListWidgetItem* item;

    if (m_listRoles_items.contains(id_role)){
        item = m_listRoles_items[id_role];
    }else{
        item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_USER)), "");
        item->setData(Qt::UserRole, id_role);
        ui->lstRoles->addItem(item);
        m_listRoles_items[id_role] = item;
    }

    QString role_text = getRoleName(role_name);
    if (role_name != role_text){
        role_text = role_text + " (" + role_name + ")";
    }
    item->setText(role_text);
    m_serviceRoles[id_role] = role_name;
}

void ServiceWidget::postServiceRoles()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray roles;

    for (const QString &role_name:qAsConst(m_serviceRoles)){
        int role_id = m_serviceRoles.key(role_name);
/*    for (int i=0; i<m_serviceRoles.count(); i++){
        int role_id = m_serviceRoles.keys().at(i);
        QString role_name = m_serviceRoles.values().at(i);*/
        QJsonObject data_obj;
        data_obj.insert("id_service_role", role_id);
        data_obj.insert("service_role_name", role_name);
        roles.append(data_obj);
    }

    QJsonObject service_obj;
    service_obj.insert("id_service", m_data->getId());
    service_obj.insert("roles", roles);
    base_obj.insert("service", service_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_SERVICEINFO_PATH, document.toJson());
}

void ServiceWidget::setData(const TeraData *data)
{
    DataEditorWidget::setData(data);

    // Fill service roles
    QVariantList roles = m_data->getFieldValue("service_roles").toList();
    m_serviceRoles.clear();
    foreach(QVariant role, roles){
        QVariantMap role_map = role.toMap();
        int id_role = role_map["id_service_role"].toInt();
        QString role_name = role_map["service_role_name"].toString();
        updateServiceRole(id_role, role_name);
    }

}

bool ServiceWidget::validateData(){
    bool valid = false;

    valid = ui->wdgService->validateFormData();

    return valid;
}

void ServiceWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type == WEB_FORMS_QUERY_SERVICE){
        ui->wdgService->buildUiFromStructure(data);
        return;
    }
}

void ServiceWidget::postResultReply(QString path)
{
    // OK, data was saved!
    if (path==WEB_SERVICEINFO_PATH){
        if (parent())
            emit closeRequest();
    }
}

void ServiceWidget::processServiceProjects(QList<TeraData> projects)
{
    for (TeraData project:projects){
        updateServiceProject(&project);
    }
}

void ServiceWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &ServiceWidget::processFormsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &ServiceWidget::postResultReply);
    connect(m_comManager, &ComManager::servicesProjectsReceived, this, &ServiceWidget::processServiceProjects);
}



void ServiceWidget::on_tabNav_currentChanged(int index)
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SERVICE, QString::number(m_data->getId()));

    QWidget* current_tab = ui->tabNav->widget(index);

    if (current_tab == ui->tabProjects){
        // Projects for that service
        //args.addQueryItem(WEB_QUERY_WITH_PROJECTS, "1"); // Also gets unassociated projects
        //args.addQueryItem(WEB_QUERY_WITH_ROLES, "1");
        queryDataRequest(WEB_SERVICEPROJECTINFO_PATH, args);
    }
}

void ServiceWidget::on_btnUpdateProjects_clicked()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray projects;

    for(QListWidgetItem* item: qAsConst(m_listProjects_items)){
        int project_id = m_listProjects_items.key(item);
    /*for (int i=0; i<m_listProjects_items.count(); i++){
        int project_id = m_listProjects_items.keys().at(i);
        QListWidgetItem* item = m_listProjects_items.values().at(i);*/
        if (item->checkState() == Qt::Checked){
            QJsonObject data_obj;
            data_obj.insert("id_project", project_id);
            projects.append(data_obj);
        }
    }

    QJsonObject service_obj;
    service_obj.insert("id_service", m_data->getId());
    service_obj.insert("projects", projects);
    base_obj.insert("service", service_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_SERVICEPROJECTINFO_PATH, document.toJson());

}

void ServiceWidget::on_lstProjects_itemChanged(QListWidgetItem *item)
{
    if (item->checkState()==Qt::Checked){
        item->setForeground(Qt::green);
    }else{
        item->setForeground(Qt::white);
    }
}

void ServiceWidget::on_lstRoles_currentRowChanged(int currentRow)
{
    ui->btnDeleteRole->setEnabled(currentRow != -1);
    ui->btnEditRole->setEnabled(currentRow != -1);
}

void ServiceWidget::on_btnNewRole_clicked()
{
    bool ok;
    QString role = QInputDialog::getText(this, tr("Nouveau rôle"), tr("Code du rôle:"), QLineEdit::Normal, "", &ok);

    if (ok && !role.isEmpty()){
        // Do query to set role
        updateServiceRole(0, role);
        postServiceRoles();
    }


}

void ServiceWidget::on_btnDeleteRole_clicked()
{
    if (ui->lstRoles->currentRow() < 0)
        return;

    GlobalMessageBox msg_box;

    if (msg_box.showYesNo(tr("Supprimer ce rôle?"), tr("Êtes-vous sûrs de vouloir supprimer le rôle:") +  " \"" + ui->lstRoles->currentItem()->text() + "\"?") == QMessageBox::Yes){
        m_serviceRoles.remove(ui->lstRoles->currentItem()->data(Qt::UserRole).toInt());
        postServiceRoles();
    }
}

void ServiceWidget::on_btnEditRole_clicked()
{
    if (ui->lstRoles->currentRow() < 0)
        return;

    bool ok;
    QString current_role = ui->lstRoles->currentItem()->text();
    int current_role_id = ui->lstRoles->currentItem()->data(Qt::UserRole).toInt();
    QString role = QInputDialog::getText(this, tr("Édition du rôle"), tr("Code du rôle:"), QLineEdit::Normal, current_role, &ok);

    if (ok && !role.isEmpty() && current_role != role){
        // Do query to set role
        updateServiceRole(current_role_id, role);
        postServiceRoles();
    }
}
