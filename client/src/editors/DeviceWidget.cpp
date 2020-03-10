#include "DeviceWidget.h"
#include "GlobalMessageBox.h"

#include "ui_DeviceWidget.h"

DeviceWidget::DeviceWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::DeviceWidget)
{

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    // Connect signals and slots
    connectSignals();

    // Query forms definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_DEVICE));

    // Query available sites
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_LIST, "true");
    queryDataRequest(WEB_SITEINFO_PATH, args);

    // Query associated participants and session types
    if (!dataIsNew()){
        args.removeAllQueryItems(WEB_QUERY_LIST);
        args.addQueryItem(WEB_QUERY_ID_DEVICE, QString::number(m_data->getId()));
        queryDataRequest(WEB_DEVICEPARTICIPANTINFO_PATH, args);

        args.removeAllQueryItems(WEB_QUERY_ID_DEVICE);
        args.addQueryItem(WEB_QUERY_ID_DEVICE_TYPE, m_data->getFieldValue("device_type").toString());
        queryDataRequest(WEB_SESSIONTYPEDEVICETYPE_PATH, args);
    }

    ui->wdgDevice->setHighlightConditions(false);
    ui->wdgDevice->setComManager(m_comManager);
}

DeviceWidget::~DeviceWidget()
{
    delete ui;
}

void DeviceWidget::saveData(bool signal)
{
    // Site data
    QJsonDocument device_data = ui->wdgDevice->getFormDataJson(m_data->isNew());

    //qDebug() << user_data.toJson();
    postDataRequest(WEB_DEVICEINFO_PATH, device_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgDevice->getFormDataObject(TERADATA_DEVICE);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void DeviceWidget::updateControlsState()
{
   ui->tabSites->setEnabled(!dataIsNew());
}

void DeviceWidget::updateFieldsValue()
{
    if (m_data && !hasPendingDataRequests()){
        if (!ui->wdgDevice->formHasData()){
            ui->wdgDevice->fillFormFromData(m_data->toJson());
        }else {
            ui->wdgDevice->resetFormValues();
        }
    }
}

bool DeviceWidget::validateData()
{
    return ui->wdgDevice->validateFormData();
}

void DeviceWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &DeviceWidget::processFormsReply);
    connect(m_comManager, &ComManager::sitesReceived, this, &DeviceWidget::processSitesReply);
    connect(m_comManager, &ComManager::deviceSitesReceived, this, &DeviceWidget::processDeviceSitesReply);
    connect(m_comManager, &ComManager::deviceProjectsReceived, this, &DeviceWidget::processDeviceProjectsReply);
    connect(m_comManager, &ComManager::deviceParticipantsReceived, this, &DeviceWidget::processDeviceParticipantsReply);
    connect(m_comManager, &ComManager::sessionTypesDeviceTypesReceived, this, &DeviceWidget::processSessionTypesReply);
    connect(m_comManager, &ComManager::projectsReceived, this, &DeviceWidget::processProjectsReply);

    connect(ui->btnSave, &QPushButton::clicked, this, &DeviceWidget::btnSave_clicked);
    connect(ui->btnUndo, &QPushButton::clicked, this, &DeviceWidget::btnUndo_clicked);
    connect(ui->btnSites, &QPushButton::clicked, this, &DeviceWidget::btnSaveSites_clicked);

    connect(ui->lstSites, &QTreeWidget::itemExpanded, this, &DeviceWidget::lstSites_itemExpanded);
    connect(ui->lstSites, &QTreeWidget::itemChanged, this, &DeviceWidget::lstSites_itemChanged);
}

void DeviceWidget::updateSite(TeraData *site)
{
    int id_site = site->getId();
    QTreeWidgetItem* item;
    if (m_listSites_items.contains(id_site)){
        item = m_listSites_items[id_site];

    }else{
        item = new QTreeWidgetItem();
        item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_SITE)));
        item->setCheckState(0, Qt::Unchecked);
        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        ui->lstSites->addTopLevelItem(item);
        m_listSites_items[id_site] = item;
    }
    item->setText(0, site->getName());
}

void DeviceWidget::updateProject(TeraData *project)
{
    int id_site = project->getFieldValue("id_site").toInt();
    int id_project = project->getId();
    QTreeWidgetItem* item;
    if (m_listProjects_items.contains(id_project)){
        item = m_listProjects_items[id_project];
    }else{
        QTreeWidgetItem* site_item = m_listSites_items[id_site];
        if (site_item){
            item = new QTreeWidgetItem();
            item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_PROJECT)));
            //item->setCheckState(0, site_item->checkState(0));
            item->setCheckState(0, Qt::Unchecked);
            site_item->addChild(item);
            m_listProjects_items[id_project] = item;
        }else{
            // No site in the list for this project...
            return;
        }
    }
    item->setText(0, project->getName());
}

void DeviceWidget::updateParticipant(TeraData *participant)
{
    int id_participant = participant->getFieldValue("id_participant").toInt();
    QString participant_name = participant->getFieldValue("participant_name").toString();
    if (m_listParticipants_items.contains(id_participant)){
        QListWidgetItem* item = m_listParticipants_items[id_participant];
        item->setText(participant_name);
    }else{
        QListWidgetItem* item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_PARTICIPANT)), participant_name);
        ui->lstParticipants->addItem(item);
        m_listParticipants_items[id_participant] = item;
    }
}

void DeviceWidget::updateSessionType(TeraData *session_type)
{
    int id_session_type = session_type->getFieldValue("id_session_type").toInt();
    QString session_type_name = session_type->getFieldValue("session_type_name").toString();
    if (m_listSessionTypes_items.contains(id_session_type)){
        QListWidgetItem* item = m_listSessionTypes_items[id_session_type];
        item->setText(session_type_name);
    }else{
        QListWidgetItem* item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_SESSIONTYPE)), session_type_name);
        ui->lstSessionTypes->addItem(item);
        m_listSessionTypes_items[id_session_type] = item;
    }
}

void DeviceWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type == WEB_FORMS_QUERY_DEVICE){
        ui->wdgDevice->buildUiFromStructure(data);
        return;
    }

}

void DeviceWidget::processSitesReply(QList<TeraData> sites)
{
    // If our site list is empty, already query sites for that device
    bool need_to_load_device_sites = m_listSites_items.isEmpty();

    for(TeraData site:sites){
        updateSite(&site);
    }

    if (need_to_load_device_sites && !dataIsNew()){
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_DEVICE, QString::number(m_data->getId()));
        args.addQueryItem(WEB_QUERY_LIST, "true");
        queryDataRequest(WEB_DEVICESITEINFO_PATH, args);
    }

    // Query projects for sites
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_LIST, "true");
    queryDataRequest(WEB_PROJECTINFO_PATH, args);
}

void DeviceWidget::processDeviceSitesReply(QList<TeraData> device_sites)
{
    if (device_sites.isEmpty())
        return;

    // Check if it is for us
    if (device_sites.first().getFieldValue("id_device").toInt() != m_data->getId())
        return;

    // Uncheck all sites
    for (QTreeWidgetItem* item:m_listSites_items){
        item->setCheckState(0, Qt::Unchecked);
    }
    // Check required sites
    for(TeraData device_site:device_sites){
        int site_id = device_site.getFieldValue("id_site").toInt();
        if (m_listSites_items.contains(site_id)){
            m_listSites_items[site_id]->setCheckState(0, Qt::Checked);
        }
    }
}

void DeviceWidget::processDeviceProjectsReply(QList<TeraData> device_projects)
{
    if (device_projects.isEmpty())
        return;

    // Check if it is for us
    if (device_projects.first().getFieldValue("id_device").toInt() != m_data->getId())
        return;

    // Uncheck all projects
    for (QTreeWidgetItem* item:m_listProjects_items){
        item->setCheckState(0, Qt::Unchecked);
    }
//    bool first_load = m_listSites_items.isEmpty();
    m_listDeviceProjects_items.clear();

    // Check required projects
    for(TeraData device_project:device_projects){
        int project_id = device_project.getFieldValue("id_project").toInt();
        int device_project_id = device_project.getId();
        if (m_listProjects_items.contains(project_id)){
            //if (first_load) // Only change state if it is not user forced
                m_listProjects_items[project_id]->setCheckState(0, Qt::Checked);
            m_listDeviceProjects_items[device_project_id] = m_listProjects_items[project_id];
        }
    }
}

void DeviceWidget::processDeviceParticipantsReply(QList<TeraData> device_parts)
{
    if (device_parts.isEmpty())
        return;

    // Check if it is for us
    if (device_parts.first().getFieldValue("id_device").toInt() != m_data->getId())
        return;

    for (TeraData device_part:device_parts){
        updateParticipant(&device_part);
    }
}

void DeviceWidget::processSessionTypesReply(QList<TeraData> session_types)
{
    if (session_types.isEmpty())
        return;

    // Check if it is for us
    if (session_types.first().getFieldValue("id_device_type").toInt() != m_data->getFieldValue("device_type").toInt())
        return;

    for (TeraData session_type:session_types){
        updateSessionType(&session_type);
    }
}

void DeviceWidget::processProjectsReply(QList<TeraData> projects)
{
    // If our project list is empty, already query projects for that device

    for(TeraData project:projects){
        updateProject(&project);
    }

    if (!dataIsNew()){
        //if (m_listSites_items.isEmpty()){
            QUrlQuery args;
            args.addQueryItem(WEB_QUERY_ID_DEVICE, QString::number(m_data->getId()));
            args.addQueryItem(WEB_QUERY_LIST, "true");
            queryDataRequest(WEB_DEVICEPROJECTINFO_PATH, args);
        //}
    }
}

void DeviceWidget::btnSave_clicked()
{
    if (!validateData()){
        QStringList invalids = ui->wdgDevice->getInvalidFormDataLabels();

        QString msg = tr("Les champs suivants doivent être complétés:") +" <ul>";
        for (QString field:invalids){
            msg += "<li>" + field + "</li>";
        }
        msg += "</ul>";
        GlobalMessageBox msgbox(this);
        msgbox.showError(tr("Champs invalides"), msg);
        return;
    }

     saveData();
}

void DeviceWidget::btnUndo_clicked()
{
    undoOrDeleteData();

    if (parent())
        emit closeRequest();
}

void DeviceWidget::btnSaveSites_clicked()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray devices;
    QList<int> projects_to_delete;

    for (int i=0; i<m_listProjects_items.count(); i++){
        // Build json list of devices and projects
        if (m_listProjects_items.values().at(i)->checkState(0)==Qt::Checked){
            QJsonObject item_obj;
            item_obj.insert("id_device", m_data->getId());
            item_obj.insert("id_project", m_listProjects_items.keys().at(i));
            devices.append(item_obj);
        }else{
            int id_device_project = m_listDeviceProjects_items.key(m_listProjects_items.values().at(i),-1);
            if (id_device_project>=0){
                projects_to_delete.append(id_device_project);
            }
        }
    }

    // Delete queries
    for (int id_device_project:projects_to_delete){
        deleteDataRequest(WEB_DEVICEPROJECTINFO_PATH, id_device_project);
    }

    if (!projects_to_delete.isEmpty()){
        // Also refresh participants
        ui->lstParticipants->clear();
        m_listParticipants_items.clear();
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_DEVICE, QString::number(m_data->getId()));
        queryDataRequest(WEB_DEVICEPARTICIPANTINFO_PATH, args);
    }

    // Update query
    base_obj.insert("device_project", devices);
    document.setObject(base_obj);
    postDataRequest(WEB_DEVICEPROJECTINFO_PATH, document.toJson());

}

void DeviceWidget::lstSites_itemExpanded(QTreeWidgetItem *item)
{
    /*if (item->childCount() == 0){
        // Query projects for that site
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_listSites_items.key(item)));
        queryDataRequest(WEB_PROJECTINFO_PATH, args);
    }*/
}

void DeviceWidget::lstSites_itemChanged(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    static bool updating = false;

    // Lock to prevent recursive updates
    if (updating)
        return;

    updating = true;
    if (m_listSites_items.values().contains(item)){
        if (item->childCount() == 0 && !isLoading()){
            // Query projects for that site if needed
            item->setExpanded(true);
        }else{
            // Check or uncheck all childs (projects)
            for (int i=0; i<item->childCount(); i++){
                item->child(i)->setCheckState(0, item->checkState(0));
            }
        }
    }

    if (m_listProjects_items.values().contains(item)){
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
