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
    if (dataIsNew()){
        // Hide useless tabs when creating a new service
        while (ui->tabNav->count() > 1)
            ui->tabNav->removeTab(1);
    }
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
    QTreeWidgetItem* item;
    QString project_name = project->getFieldValue("project_name").toString();

    if (m_treeProjects_items.contains(id_project)){
        item = m_treeProjects_items[id_project];
    }else{
        item = new QTreeWidgetItem();
        item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_PROJECT)));
        int id_site = project->getFieldValue("id_site").toInt();
        QTreeWidgetItem* parent_item = m_treeSites_items[id_site];
        if (!parent_item){
            LOG_WARNING(tr("Site not found for project") + " :" + project_name, "ServiceWidget::updateServiceProject");
            delete item;
            return;
        }
        parent_item->addChild(item);
        m_treeProjects_items[id_project] = item;
    }

    if (!project_name.isEmpty())
        item->setText(0, project_name);

    int service_id = project->getFieldValue("id_service").toInt();
    if (service_id > 0){
        item->setCheckState(0, Qt::Checked);
    }else{
        item->setCheckState(0, Qt::Unchecked);
    }
}

void ServiceWidget::updateServiceSite(TeraData *service_site)
{
    int id_site = service_site->getFieldValue("id_site").toInt();
    QTreeWidgetItem* item;
    if (m_treeSites_items.contains(id_site)){
        item = m_treeSites_items[id_site];

    }else{
        item = new QTreeWidgetItem();
        item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_SITE)));
        item->setCheckState(0, Qt::Unchecked);
        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        ui->treeProjects->addTopLevelItem(item);
        m_treeSites_items[id_site] = item;
    }
    item->setText(0, service_site->getFieldValue("site_name").toString());
    if (service_site->getId() > 0){
        item->setCheckState(0, Qt::Checked);
    }else{
        item->setCheckState(0, Qt::Unchecked);
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

void ServiceWidget::postServiceSites()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray sites;

    for(QTreeWidgetItem* item: qAsConst(m_treeSites_items)){
        int site_id = m_treeSites_items.key(item);
        if (item->checkState(0) == Qt::Checked){
            QJsonObject data_obj;
            data_obj.insert("id_site", site_id);
            sites.append(data_obj);
        }
    }

    QJsonObject service_obj;
    service_obj.insert("id_service", m_data->getId());
    service_obj.insert("sites", sites);
    base_obj.insert("service", service_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_SERVICESITEINFO_PATH, document.toJson());
}

void ServiceWidget::postServiceProjects()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray projects;

    for(QTreeWidgetItem* item: qAsConst(m_treeProjects_items)){
        int project_id = m_treeProjects_items.key(item);
        if (item->checkState(0) == Qt::Checked){
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

    if (path==WEB_SERVICESITEINFO_PATH){
        // We saved the service-site association, now saving the service-project association
        postServiceProjects();
    }
}

void ServiceWidget::processServiceProjects(QList<TeraData> projects)
{
    for (TeraData project:projects){
        updateServiceProject(&project);
    }

    ui->treeProjects->expandAll();
}

void ServiceWidget::processServiceSites(QList<TeraData> sites, QUrlQuery reply_query)
{
    for (TeraData site:sites){
        updateServiceSite(&site);
    }

    // Query projects for service
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SERVICE, QString::number(m_data->getId()));
    args.addQueryItem(WEB_QUERY_WITH_PROJECTS, "1");
    args.addQueryItem(WEB_QUERY_WITH_SITES, "1");
    queryDataRequest(WEB_SERVICEPROJECTINFO_PATH, args);
}

void ServiceWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &ServiceWidget::processFormsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &ServiceWidget::postResultReply);
    connect(m_comManager, &ComManager::servicesProjectsReceived, this, &ServiceWidget::processServiceProjects);
    connect(m_comManager, &ComManager::servicesSitesReceived, this, &ServiceWidget::processServiceSites);
}



void ServiceWidget::on_tabNav_currentChanged(int index)
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SERVICE, QString::number(m_data->getId()));

    QWidget* current_tab = ui->tabNav->widget(index);

    if (current_tab == ui->tabProjects){
        // Sites / Projects
        if (m_treeSites_items.isEmpty() || m_treeSites_items.isEmpty()){
            args.addQueryItem(WEB_QUERY_WITH_SITES, "1");
            queryDataRequest(WEB_SERVICESITEINFO_PATH, args);
        }
        // Projects for that service
        /*args.addQueryItem(WEB_QUERY_WITH_PROJECTS, "1"); // Also gets unassociated projects
        args.addQueryItem(WEB_QUERY_WITH_ROLES, "1");
        queryDataRequest(WEB_SERVICEPROJECTINFO_PATH, args);*/
    }
}

void ServiceWidget::on_btnUpdateProjects_clicked()
{
    // We post service-site association first
    postServiceSites();

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
    int current_role_id = ui->lstRoles->currentItem()->data(Qt::UserRole).toInt();
    QString current_role = m_serviceRoles[current_role_id]; //ui->lstRoles->currentItem()->text();
    QString role = QInputDialog::getText(this, tr("Édition du rôle"), tr("Code du rôle:"), QLineEdit::Normal, current_role, &ok);

    if (ok && !role.isEmpty() && current_role != role){
        // Do query to set role
        updateServiceRole(current_role_id, role);
        postServiceRoles();
    }
}

void ServiceWidget::on_lstRoles_itemDoubleClicked(QListWidgetItem *item)
{
    on_btnEditRole_clicked();
}


void ServiceWidget::on_treeProjects_itemChanged(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    static bool updating = false;

    // Lock to prevent recursive updates
    if (updating)
        return;

    updating = true;
    if (std::find(m_treeSites_items.cbegin(), m_treeSites_items.cend(), item) != m_treeSites_items.cend()){
        if (item->childCount() == 0 && !isLoading()){
            item->setExpanded(true);
        }else{
            // Check or uncheck all childs (projects)
            for (int i=0; i<item->childCount(); i++){
                item->child(i)->setCheckState(0, item->checkState(0));
            }
        }
    }

    if (std::find(m_treeProjects_items.cbegin(), m_treeProjects_items.cend(), item) != m_treeProjects_items.cend()){
        // We have a project - check if we need to check the parent (site)
        QTreeWidgetItem* site = item->parent();
        if (site){
            for(int i=0; i<site->childCount(); i++){
                if (site->child(i)->checkState(0) == Qt::Checked){
                    site->setCheckState(0, Qt::Checked);
                    updating = false;
                    return;
                }
            }
            // No projects selected for that site
            site->setCheckState(0, Qt::Unchecked);
        }
    }

    updating = false;
}

