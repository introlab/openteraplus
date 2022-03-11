#include "SessionTypeWidget.h"
#include "ui_SessionTypeWidget.h"

SessionTypeWidget::SessionTypeWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::SessionTypeWidget)
{   

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image
    ui->tabNav->setCurrentIndex(0);

    setLimited(false);

    // Use base class to manage editing
    setEditorControls(ui->wdgSessionType, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    // Query form definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_SESSION_TYPE));
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_SESSION_TYPE_CONFIG));

    ui->wdgSessionType->setComManager(m_comManager);
    setData(data);

}

SessionTypeWidget::~SessionTypeWidget()
{
    if (ui)
        delete ui;
}

void SessionTypeWidget::saveData(bool signal){

    // If data is new, we request all the fields.
    QJsonDocument st_data = ui->wdgSessionType->getFormDataJson(m_data->isNew());

    // TODO: New session type - also include projects and sites
    /*if (m_data->isNew() && ui->treeProjects->isEnabled()){
        QJsonObject base_obj = st_data.object();
        QJsonObject base_st = base_obj["session_type"].toObject();
        QJsonArray projects = getSelectedProjectsAsJsonArray();
        // For new session types, also sends the projects at the same time
        if (!projects.isEmpty()){
            base_st.insert("session_type_projects", projects);
            base_obj.insert("session_type", base_st);
            st_data.setObject(base_obj);
        }
    }*/

    postDataRequest(WEB_SESSIONTYPE_PATH, st_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgSessionType->getFormDataObject(TERADATA_SESSIONTYPE);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void SessionTypeWidget::updateControlsState(){
    ui->tabProjects->setEnabled(!dataIsNew());
    ui->tabConfig->setEnabled(!dataIsNew());

    if (dataIsNew() && ui->tabNav->count()>1){

        // Move projects list to first tab
        ui->tabProjects->layout()->removeWidget(ui->treeProjects);
        ui->tabDashboard->layout()->removeWidget(ui->frameButtons);

        QLabel* lbl = new QLabel(tr("Projets associés"));
        QFont labelFont;
        labelFont.setBold(true);
        labelFont.setPointSize(10);
        lbl->setFont(labelFont);

        ui->tabDashboard->layout()->addWidget(lbl);
        ui->tabDashboard->layout()->addWidget(ui->treeProjects);
        ui->tabDashboard->layout()->addWidget(ui->frameButtons);
        while (ui->tabNav->count() > 1)
            ui->tabNav->removeTab(1);

        // Query sites and projects if needed
        // TODO - query projects and sites when new session type is created...

        /*if (m_treeProjects_items.isEmpty()){
            QUrlQuery args;
            args.addQueryItem(WEB_QUERY_WITH_SITES, "1");
            queryDataRequest(WEB_SESSIONTYPESITE_PATH, args);
        }*/
    }
}

void SessionTypeWidget::updateFieldsValue(){
    if (m_data){
        ui->wdgSessionType->fillFormFromData(m_data->toJson());
        ui->wdgSessionTypeConfig->fillFormFromData(m_data->getFieldValue("session_type_config").toString());
        ui->lblTitle->setText(m_data->getName());
    }
}

void SessionTypeWidget::updateSessionTypeSite(TeraData *sts)
{
    int id_site = sts->getFieldValue("id_site").toInt();
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
    QString site_name = sts->getFieldValue("site_name").toString();
    if (!site_name.isEmpty())
        item->setText(0, site_name);
    if (sts->getId() > 0){
        item->setCheckState(0, Qt::Checked);
    }else{
        item->setCheckState(0, Qt::Unchecked);
    }
}

void SessionTypeWidget::updateSessionTypeProject(TeraData *stp)
{
    int id_project = stp->getFieldValue("id_project").toInt();
    QTreeWidgetItem* item;
    QString project_name = stp->getFieldValue("project_name").toString();

    if (m_treeProjects_items.contains(id_project)){
        item = m_treeProjects_items[id_project];
    }else{
        item = new QTreeWidgetItem();
        item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_PROJECT)));
        int id_site = stp->getFieldValue("id_site").toInt();
        QTreeWidgetItem* parent_item = m_treeSites_items[id_site];
        if (!parent_item){
            LOG_WARNING(tr("Site not found for project") + " :" + project_name, "SessionTypeWidget::updateSessionTypeProject");
            delete item;
            return;
        }
        parent_item->addChild(item);
        m_treeProjects_items[id_project] = item;
    }

    if (!project_name.isEmpty())
        item->setText(0, project_name);

    int st_id = stp->getFieldValue("id_session_type").toInt();
    if (st_id > 0){
        item->setCheckState(0, Qt::Checked);
    }else{
        item->setCheckState(0, Qt::Unchecked);
    }
}

void SessionTypeWidget::postSessionTypeSites()
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
    service_obj.insert("id_session_type", m_data->getId());
    service_obj.insert("sites", sites);
    base_obj.insert("session_type", service_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_SESSIONTYPESITE_PATH, document.toJson());
}

void SessionTypeWidget::postSessionTypeProjects()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray projects;

    for(QTreeWidgetItem* item: qAsConst(m_treeProjects_items)){
        int proj_id = m_treeProjects_items.key(item);
        if (item->checkState(0) == Qt::Checked){
            QJsonObject data_obj;
            data_obj.insert("id_project", proj_id);
            projects.append(data_obj);
        }
    }

    QJsonObject service_obj;
    service_obj.insert("id_session_type", m_data->getId());
    service_obj.insert("projects", projects);
    base_obj.insert("session_type", service_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_SESSIONTYPEPROJECT_PATH, document.toJson());
}

bool SessionTypeWidget::validateData(){
    bool valid = false;

    valid = ui->wdgSessionType->validateFormData();

    if (dataIsNew() && valid){
        valid = validateProjects();
    }
    return valid;
}

bool SessionTypeWidget::validateProjects()
{
    if (!m_comManager->isCurrentUserSuperAdmin()){
        bool at_least_one_selected = false;
        for (int i=0; i<m_treeSites_items.count(); i++){
            if (m_treeSites_items.values().at(i)->checkState(0) == Qt::Checked){
                at_least_one_selected = true;
                break;
            }
        }
        if (!at_least_one_selected){
            // Warning: that session type not having any project meaning that it will be not available to the current user
            GlobalMessageBox msgbox;
            msgbox.showError(tr("Attention"), tr("Aucun site n'a été associé.\nVous devez associer au moins un site."));
            return false;
        }
    }
    return true;
}

void SessionTypeWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type == WEB_FORMS_QUERY_SESSION_TYPE){
        ui->wdgSessionType->buildUiFromStructure(data);
        ui->wdgSessionType->hideField("session_type_config");
        return;
    }

    if (form_type == WEB_FORMS_QUERY_SESSION_TYPE_CONFIG){
        ui->wdgSessionTypeConfig->buildUiFromStructure(data);
        return;
    }
}

void SessionTypeWidget::processSessionTypesProjectsReply(QList<TeraData> stp_list)
{
    for (TeraData project:stp_list){
        updateSessionTypeProject(&project);
    }

    ui->treeProjects->expandAll();
}

void SessionTypeWidget::processSessionTypesSitesReply(QList<TeraData> sts_list)
{
    for (TeraData sts:sts_list){
        updateSessionTypeSite(&sts);
    }

    // Query projects for session type
    if (m_treeProjects_items.isEmpty()){
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_SESSION_TYPE, QString::number(m_data->getId()));
        args.addQueryItem(WEB_QUERY_WITH_PROJECTS, "1");
        args.addQueryItem(WEB_QUERY_WITH_SITES, "1");
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

    if (path==WEB_SESSIONTYPESITE_PATH){
        postSessionTypeProjects();
    }
}

void SessionTypeWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &SessionTypeWidget::processFormsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &SessionTypeWidget::postResultReply);
    connect(m_comManager, &ComManager::sessionTypesProjectsReceived, this, &SessionTypeWidget::processSessionTypesProjectsReply);
    connect(m_comManager, &ComManager::sessionTypesSitesReceived, this, &SessionTypeWidget::processSessionTypesSitesReply);

    connect(ui->btnProjects, & QPushButton::clicked, this, &SessionTypeWidget::btnSaveProjects_clicked);
}

void SessionTypeWidget::btnSaveProjects_clicked()
{
    if (!validateProjects())
        return;

    postSessionTypeSites();
}

void SessionTypeWidget::on_treeProjects_itemChanged(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    static bool updating = false;

    // Lock to prevent recursive updates
    if (updating)
        return;

    updating = true;
    // Uncheck all projects if site was unselected
    if (std::find(m_treeSites_items.cbegin(), m_treeSites_items.cend(), item) != m_treeSites_items.cend()){
        if (item->checkState(0) == Qt::Unchecked){
            for (int i=0; i<item->childCount(); i++){
                item->child(i)->setCheckState(0, Qt::Unchecked);
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
        }
    }
    updating = false;
}

void SessionTypeWidget::on_tabNav_currentChanged(int index)
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SESSION_TYPE, QString::number(m_data->getId()));

    QWidget* current_tab = ui->tabNav->widget(index);

    if (current_tab == ui->tabProjects){
        // Sites / Projects
        if (m_treeSites_items.isEmpty() || m_treeSites_items.isEmpty()){
            args.addQueryItem(WEB_QUERY_WITH_SITES, "1");
            queryDataRequest(WEB_SESSIONTYPESITE_PATH, args);
        }
    }

}

void SessionTypeWidget::on_btnUpdateConfig_clicked()
{
    QJsonObject session_type_config = ui->wdgSessionTypeConfig->getFormDataJson(true).object()["session_type_config"].toObject();
    QString config_str = QString::fromUtf8(QJsonDocument(session_type_config).toJson(QJsonDocument::Compact));
    ui->wdgSessionType->setFieldValue("session_type_config", config_str);
    saveData();
}
