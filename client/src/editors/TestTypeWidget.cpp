#include "TestTypeWidget.h"
#include "ui_TestTypeWidget.h"

TestTypeWidget::TestTypeWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::TestTypeWidget)
{   

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image
    ui->tabNav->setCurrentIndex(0);

    setLimited(false);

    // Use base class to manage editing
    setEditorControls(ui->wdgTestType, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    // Query form definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_TEST_TYPE));

    ui->wdgTestType->setComManager(m_comManager);
    setData(data);

}

TestTypeWidget::~TestTypeWidget()
{
    if (ui)
        delete ui;
}

void TestTypeWidget::saveData(bool signal){

    // If data is new, we request all the fields.
    QJsonDocument st_data = ui->wdgTestType->getFormDataJson(m_data->isNew());

    // New test type - also include projects and sites
    if (m_data->isNew() && ui->treeProjects->isEnabled()){
        QJsonObject base_obj = st_data.object();
        QJsonObject base_st = base_obj["test_type"].toObject();
        QJsonArray projects;
        QJsonArray sites;

        // Sites
        for(QTreeWidgetItem* item:std::as_const(m_treeSites_items)){
            if (item->checkState(0) == Qt::Checked){
                int site_id = m_treeSites_items.key(item);
                QJsonObject data_obj;
                data_obj.insert("id_site", site_id);
                sites.append(data_obj);
            }
        }

        if (!sites.isEmpty()){
            base_st.insert("test_type_sites", sites);
            base_obj.insert("test_type", base_st);
            st_data.setObject(base_obj);
        }

        // Projects
        for(QTreeWidgetItem* item:std::as_const(m_treeProjects_items)){
            if (item->checkState(0) == Qt::Checked){
                int project_id = m_treeProjects_items.key(item);
                QJsonObject data_obj;
                data_obj.insert("id_project", project_id);
                projects.append(data_obj);
            }
        }

        if (!projects.isEmpty()){
            base_st.insert("test_type_projects", projects);
            base_obj.insert("test_type", base_st);
            st_data.setObject(base_obj);
        }
    }

    postDataRequest(WEB_TESTTYPEINFO_PATH, st_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgTestType->getFormDataObject(TERADATA_TESTTYPE);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void TestTypeWidget::updateControlsState(){
    ui->tabProjects->setEnabled(!dataIsNew());

    if (dataIsNew() && ui->tabNav->count()>1){

        // Move projects list to first tab
        ui->tabProjects->layout()->removeWidget(ui->treeProjects);
        ui->tabDashboard->layout()->removeWidget(ui->frameButtons);

        QLabel* lbl = new QLabel(tr("Sites / projets associés"));
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
        if (m_treeSites_items.isEmpty() || m_treeProjects_items.isEmpty()){
            QUrlQuery args;
            args.addQueryItem(WEB_QUERY_LIST, "1");
            queryDataRequest(WEB_SITEINFO_PATH, args);
        }

    }
}

void TestTypeWidget::updateFieldsValue(){
    if (m_data){
        ui->wdgTestType->fillFormFromData(m_data->toJson());
        if (m_data->hasNameField())
            ui->lblTitle->setText(m_data->getName());
        else
            ui->lblTitle->setText(tr("Inconnu"));
    }
}

void TestTypeWidget::updateTestTypeSite(TeraData *tts)
{
    int id_site = tts->getFieldValue("id_site").toInt();
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
    QString site_name = tts->getFieldValue("site_name").toString();
    if (!site_name.isEmpty())
        item->setText(0, site_name);
    if (tts->getId() > 0){
        item->setCheckState(0, Qt::Checked);
    }else{
        item->setCheckState(0, Qt::Unchecked);
    }
}

void TestTypeWidget::updateTestTypeProject(TeraData *ttp)
{
    int id_project = ttp->getFieldValue("id_project").toInt();
    QTreeWidgetItem* item;
    QString project_name = ttp->getFieldValue("project_name").toString();

    if (m_treeProjects_items.contains(id_project)){
        item = m_treeProjects_items[id_project];
    }else{
        item = new QTreeWidgetItem();
        item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_PROJECT)));
        int id_site = ttp->getFieldValue("id_site").toInt();
        QTreeWidgetItem* parent_item = m_treeSites_items[id_site];
        if (!parent_item){
            LOG_WARNING("Site not found for project: " + project_name, "TestTypeWidget::updateTestTypeProject");
            delete item;
            return;
        }
        parent_item->addChild(item);
        m_treeProjects_items[id_project] = item;
    }

    if (!project_name.isEmpty())
        item->setText(0, project_name);

    int st_id = ttp->getFieldValue("id_test_type").toInt();
    if (st_id > 0){
        item->setCheckState(0, Qt::Checked);
    }else{
        item->setCheckState(0, Qt::Unchecked);
    }
}

void TestTypeWidget::updateSite(TeraData *site)
{
    int id_site = site->getId();
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
    item->setText(0, site->getName());
}

void TestTypeWidget::updateProject(TeraData *project)
{
    int id_site = project->getFieldValue("id_site").toInt();
    int id_project = project->getId();
    QTreeWidgetItem* item;
    if (m_treeProjects_items.contains(id_project)){
        item = m_treeProjects_items[id_project];
    }else{
        QTreeWidgetItem* site_item = m_treeSites_items.value(id_site, nullptr);
        if (site_item){
            item = new QTreeWidgetItem();
            item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_PROJECT)));
            //item->setCheckState(0, site_item->checkState(0));
            item->setCheckState(0, Qt::Unchecked);
            site_item->addChild(item);
            m_treeProjects_items[id_project] = item;
        }else{
            // No site in the list for this project...
            return;
        }
    }
    item->setText(0, project->getName());
}

void TestTypeWidget::postTestTypeSites()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray sites;

    for(QTreeWidgetItem* item: std::as_const(m_treeSites_items)){
        int site_id = m_treeSites_items.key(item);
        if (item->checkState(0) == Qt::Checked){
            QJsonObject data_obj;
            data_obj.insert("id_site", site_id);
            sites.append(data_obj);
        }
    }

    QJsonObject service_obj;
    service_obj.insert("id_test_type", m_data->getId());
    service_obj.insert("sites", sites);
    base_obj.insert("test_type", service_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_TESTTYPESITE_PATH, document.toJson());
}

void TestTypeWidget::postTestTypeProjects()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray projects;

    for(QTreeWidgetItem* item: std::as_const(m_treeProjects_items)){
        int proj_id = m_treeProjects_items.key(item);
        if (item->checkState(0) == Qt::Checked){
            QJsonObject data_obj;
            data_obj.insert("id_project", proj_id);
            projects.append(data_obj);
        }
    }

    QJsonObject service_obj;
    service_obj.insert("id_test_type", m_data->getId());
    service_obj.insert("projects", projects);
    base_obj.insert("test_type", service_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_TESTTYPEPROJECT_PATH, document.toJson());
}

bool TestTypeWidget::validateData(){
    bool valid = false;

    valid = ui->wdgTestType->validateFormData();

    if (dataIsNew() && valid){
        valid = validateProjects();
    }
    return valid;
}

bool TestTypeWidget::validateProjects()
{
    if (!m_comManager->isCurrentUserSuperAdmin()){
        bool at_least_one_selected = false;
        for(QTreeWidgetItem* site_item:std::as_const(m_treeSites_items)){
            if (site_item->checkState(0) == Qt::Checked){
                at_least_one_selected = true;
                break;
            }
        }
        if (!at_least_one_selected){
            // Warning: that type not having any project meaning that it will be not available to the current user
            GlobalMessageBox msgbox;
            msgbox.showError(tr("Attention"), tr("Aucun site n'a été associé.\nVous devez associer au moins un site."));
            return false;
        }
    }
    return true;
}

void TestTypeWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type.startsWith(WEB_FORMS_QUERY_TEST_TYPE)){
        ui->wdgTestType->buildUiFromStructure(data);
        return;
    }
}

void TestTypeWidget::processTestTypesProjectsReply(QList<TeraData> ttp_list)
{
    for (TeraData project:ttp_list){
        updateTestTypeProject(&project);
    }

    ui->treeProjects->expandAll();
}

void TestTypeWidget::processTestTypesSitesReply(QList<TeraData> tts_list)
{
    for (TeraData tts:tts_list){
        updateTestTypeSite(&tts);
    }

    // Query projects for test type
    if (m_treeProjects_items.isEmpty()){
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_TEST_TYPE, QString::number(m_data->getId()));
        args.addQueryItem(WEB_QUERY_WITH_PROJECTS, "1");
        args.addQueryItem(WEB_QUERY_WITH_SITES, "1");
        queryDataRequest(WEB_TESTTYPEPROJECT_PATH, args);
    }
}

void TestTypeWidget::processSitesReply(QList<TeraData> sites)
{
    for(TeraData site:sites){
        updateSite(&site);
    }

    // Query projects for sites
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_LIST, "true");
    queryDataRequest(WEB_PROJECTINFO_PATH, args);
}

void TestTypeWidget::processProjectsReply(QList<TeraData> projects)
{
    for(TeraData project:projects){
        updateProject(&project);
    }

    ui->treeProjects->expandAll();
}


void TestTypeWidget::postResultReply(QString path)
{
    // OK, data was saved!
    if (path==WEB_TESTTYPEINFO_PATH){
        if (parent())
            emit closeRequest();
    }

    if (path==WEB_TESTTYPESITE_PATH){
        postTestTypeProjects();
    }
}

void TestTypeWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &TestTypeWidget::processFormsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &TestTypeWidget::postResultReply);
    connect(m_comManager, &ComManager::testTypesProjectsReceived, this, &TestTypeWidget::processTestTypesProjectsReply);
    connect(m_comManager, &ComManager::testTypesSitesReceived, this, &TestTypeWidget::processTestTypesSitesReply);

    connect(m_comManager, &ComManager::sitesReceived, this, &TestTypeWidget::processSitesReply);
    connect(m_comManager, &ComManager::projectsReceived, this, &TestTypeWidget::processProjectsReply);

    connect(ui->btnProjects, & QPushButton::clicked, this, &TestTypeWidget::btnSaveProjects_clicked);
}

void TestTypeWidget::btnSaveProjects_clicked()
{
    if (!validateProjects())
        return;

    postTestTypeSites();
}

void TestTypeWidget::on_treeProjects_itemChanged(QTreeWidgetItem *item, int column)
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

void TestTypeWidget::on_tabNav_currentChanged(int index)
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_TEST_TYPE, QString::number(m_data->getId()));

    QWidget* current_tab = ui->tabNav->widget(index);

    if (current_tab == ui->tabProjects){
        // Sites / Projects
        if (m_treeSites_items.isEmpty() || m_treeSites_items.isEmpty()){
            args.addQueryItem(WEB_QUERY_WITH_SITES, "1");
            queryDataRequest(WEB_TESTTYPESITE_PATH, args);
        }
    }

}
