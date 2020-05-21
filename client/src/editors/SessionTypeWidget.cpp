#include "SessionTypeWidget.h"
#include "ui_SessionTypeWidget.h"

SessionTypeWidget::SessionTypeWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::SessionTypeWidget)
{   

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    setLimited(false);

    // Use base class to manage editing
    setEditorControls(ui->wdgSessionType, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    // Query form definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_SESSION_TYPE));

    ui->wdgSessionType->setComManager(m_comManager);
    setData(data);

    if (!dataIsNew()){
        // Loads first detailled informations tab
        on_tabSessionTypeInfos_currentChanged(0);

    }

}

SessionTypeWidget::~SessionTypeWidget()
{
    if (ui)
        delete ui;
}

void SessionTypeWidget::saveData(bool signal){

    // If data is new, we request all the fields.
    QJsonDocument st_data = ui->wdgSessionType->getFormDataJson(m_data->isNew());

    postDataRequest(WEB_SESSIONTYPE_PATH, st_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgSessionType->getFormDataObject(TERADATA_SESSIONTYPE);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void SessionTypeWidget::updateControlsState(){
    ui->tabDetails->setEnabled(!dataIsNew());
}

void SessionTypeWidget::updateFieldsValue(){
    if (m_data){
        ui->wdgSessionType->fillFormFromData(m_data->toJson());
        ui->lblTitle->setText(m_data->getName());
    }
}

void SessionTypeWidget::updateProject(TeraData *project)
{
    int id_project = project->getId();
    if (m_listProjects_items.contains(id_project)){
        QListWidgetItem* item = m_listProjects_items[id_project];
        item->setText(project->getName());
    }else{
        QListWidgetItem* item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_PROJECT)), project->getName());
        item->setCheckState(Qt::Unchecked);
        ui->lstProjects->addItem(item);
        m_listProjects_items[id_project] = item;
    }
}

bool SessionTypeWidget::validateData(){
    bool valid = false;

    valid = ui->wdgSessionType->validateFormData();

    return valid;
}

void SessionTypeWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type == WEB_FORMS_QUERY_SESSION_TYPE){
        ui->wdgSessionType->buildUiFromStructure(data);
        return;
    }
}

void SessionTypeWidget::processSessionTypesProjectsReply(QList<TeraData> stp_list)
{
    if (stp_list.isEmpty())
        return;

    // Check if it is for us
    if (stp_list.first().hasFieldName(WEB_QUERY_ID_SESSION_TYPE)){
        if (stp_list.first().getFieldValue(WEB_QUERY_ID_SESSION_TYPE).toInt() != m_data->getId())
            return;
    }else{
        return;
    }

    // Uncheck all projects
    for (QListWidgetItem* item:m_listProjects_items){
        item->setCheckState(Qt::Unchecked);
    }
    m_listTypesProjects_items.clear();

    // Check required projects
    for(TeraData type_project:stp_list){
        int project_id = type_project.getFieldValue("id_project").toInt();
        int stp_id = type_project.getId();
        if (m_listProjects_items.contains(project_id)){
            m_listProjects_items[project_id]->setCheckState(Qt::Checked);
            m_listTypesProjects_items[stp_id] = m_listProjects_items[project_id];
        }
    }
}

void SessionTypeWidget::processProjectsReply(QList<TeraData> projects)
{
    // If our site list is empty, already query sites for that device
    bool need_to_load_sessions_type_projects = m_listProjects_items.isEmpty();

    for(TeraData project:projects){
        updateProject(&project);
    }

    if (need_to_load_sessions_type_projects && !dataIsNew()){
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_SESSION_TYPE, QString::number(m_data->getId()));
        args.addQueryItem(WEB_QUERY_LIST, "true");
        queryDataRequest(WEB_SESSIONTYPEPROJECT_PATH, args);
    }
}

void SessionTypeWidget::postResultReply(QString path)
{
    // OK, data was saved!
    if (path==WEB_SESSIONTYPE_PATH){
        if (parent())
            emit closeRequest();
    }
}

void SessionTypeWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &SessionTypeWidget::processFormsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &SessionTypeWidget::postResultReply);
    connect(m_comManager, &ComManager::sessionTypesProjectsReceived, this, &SessionTypeWidget::processSessionTypesProjectsReply);
    connect(m_comManager, &ComManager::projectsReceived, this, &SessionTypeWidget::processProjectsReply);

    connect(ui->btnProjects, & QPushButton::clicked, this, &SessionTypeWidget::btnSaveProjects_clicked);
}

void SessionTypeWidget::btnSaveProjects_clicked()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray projects;
    QList<int> projects_to_delete;

    for (int i=0; i<ui->lstProjects->count(); i++){
        // Build json list of projects
        if (ui->lstProjects->item(i)->checkState()==Qt::Checked){
            QJsonObject item_obj;
            item_obj.insert("id_session_type", m_data->getId());
            item_obj.insert("id_project", m_listProjects_items.key(ui->lstProjects->item(i)));
            projects.append(item_obj);
        }else{
            int id_type_project = m_listTypesProjects_items.key(ui->lstProjects->item(i),-1);
            if (id_type_project>=0){
                projects_to_delete.append(id_type_project);
            }
        }
    }

    // Delete queries
    for (int id_type_project:projects_to_delete){
        deleteDataRequest(WEB_SESSIONTYPEPROJECT_PATH, id_type_project);
    }

    // Update query
    base_obj.insert("session_type_project", projects);
    document.setObject(base_obj);
    postDataRequest(WEB_SESSIONTYPEPROJECT_PATH, document.toJson());
}

void SessionTypeWidget::on_tabSessionTypeInfos_currentChanged(int index)
{

    QUrlQuery args;

    if (index == 0){
        // Query available projects
        if (m_listProjects_items.isEmpty()){
            args.addQueryItem(WEB_QUERY_LIST, "true");
            queryDataRequest(WEB_PROJECTINFO_PATH, args);
        }
    }

}
