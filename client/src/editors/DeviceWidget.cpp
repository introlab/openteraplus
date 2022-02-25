#include "DeviceWidget.h"
#include "GlobalMessageBox.h"

#include "ui_DeviceWidget.h"

DeviceWidget::DeviceWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::DeviceWidget)
{

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    ui->tabNav->setCurrentIndex(0);

    // Use base class to manage editing
    setEditorControls(ui->wdgDevice, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    // Query forms definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_DEVICE));

    ui->wdgDevice->setHighlightConditions(false);
    ui->wdgDevice->setComManager(m_comManager);

    setData(data);

    /*if (!dataIsNew()){
        // Loads first detailled informations tab
        on_tabDeviceInfos_currentChanged(0);

    }*/
}

DeviceWidget::~DeviceWidget()
{
    delete ui;
}

void DeviceWidget::saveData(bool signal)
{
    // Site data
    QJsonDocument device_data = ui->wdgDevice->getFormDataJson(m_data->isNew());

    if (m_data->isNew() && ui->lstSites->isEnabled()){
        QJsonObject base_obj = device_data.object();
        QJsonObject base_st = base_obj["device"].toObject();
        QJsonArray projects = getSelectedProjectsAsJsonArray();
        // For new device, also sends the projects at the same time
        if (!projects.isEmpty()){
            base_st.insert("device_projects", projects);
            base_obj.insert("device", base_st);
            device_data.setObject(base_obj);
        }
    }

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
   //ui->tabDetails->setEnabled(!dataIsNew());

   if (dataIsNew() && ui->tabNav->count()>1){

       // Move sites / projects list to first tab
       ui->tabSites->layout()->removeWidget(ui->lstSites);
       ui->tabDashboard->layout()->removeWidget(ui->frameButtons);

       QLabel* lbl = new QLabel(tr("Sites / Projets associés"));
       QFont labelFont;
       labelFont.setBold(true);
       labelFont.setPointSize(10);
       lbl->setFont(labelFont);

       ui->tabDashboard->layout()->addWidget(lbl);
       ui->tabDashboard->layout()->addWidget(ui->lstSites);
       ui->tabDashboard->layout()->addWidget(ui->frameButtons);
       while (ui->tabNav->count() > 1)
           ui->tabNav->removeTab(1);

       // Query sites and projects if needed
       if (m_listSites_items.isEmpty() || m_listProjects_items.isEmpty()){
           QUrlQuery args;
           args.addQueryItem(WEB_QUERY_LIST, "true");
           if (m_data->hasFieldName("id_site")){
               args.addQueryItem(WEB_QUERY_ID_SITE, m_data->getFieldValue("id_site").toString());
               m_data->removeFieldName("id_site");
           }
           queryDataRequest(WEB_SITEINFO_PATH, args);
       }
   }
}

void DeviceWidget::updateFieldsValue()
{
    if (m_data && !hasPendingDataRequests()){
        if (!ui->wdgDevice->formHasData()){
            ui->wdgDevice->fillFormFromData(m_data->toJson());
        }else {
            ui->wdgDevice->resetFormValues();
        }
        ui->lblTitle->setText(m_data->getName());
    }
}

bool DeviceWidget::validateData()
{
    if (dataIsNew()){
        if (!validateSitesProjects())
            return false;
    }
    return ui->wdgDevice->validateFormData();
}

void DeviceWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &DeviceWidget::processFormsReply);
    connect(m_comManager, &ComManager::sitesReceived, this, &DeviceWidget::processSitesReply);
    connect(m_comManager, &ComManager::deviceSitesReceived, this, &DeviceWidget::processDeviceSitesReply);
    connect(m_comManager, &ComManager::deviceProjectsReceived, this, &DeviceWidget::processDeviceProjectsReply);
    connect(m_comManager, &ComManager::deviceParticipantsReceived, this, &DeviceWidget::processDeviceParticipantsReply);
    connect(m_comManager, &ComManager::projectsReceived, this, &DeviceWidget::processProjectsReply);
    connect(m_comManager, &ComManager::deleteResultsOK, this, &DeviceWidget::deleteDataReply);

    connect(ui->btnSites, &QPushButton::clicked, this, &DeviceWidget::btnSaveSites_clicked);

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
        item->setData(Qt::UserRole, participant->getFieldValue("id_device_participant"));
        item->setText(participant_name);
    }else{
        QListWidgetItem* item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_PARTICIPANT)), participant_name);
        item->setData(Qt::UserRole, participant->getFieldValue("id_device_participant"));
        ui->lstParticipants->addItem(item);
        m_listParticipants_items[id_participant] = item;
    }
}

bool DeviceWidget::validateSitesProjects()
{
    //if (!m_comManager->isCurrentUserSuperAdmin()){
        bool at_least_one_selected = false;
        for(QTreeWidgetItem* item: qAsConst(m_listSites_items)){
            if (item->checkState(0) == Qt::Checked){
                at_least_one_selected = true;
                break;
            }
        }

        if (!at_least_one_selected){
            for(QTreeWidgetItem* item: qAsConst(m_listProjects_items)){
                if (item->checkState(0) == Qt::Checked){
                    at_least_one_selected = true;
                    break;
                }
            }
        }

        if (!at_least_one_selected){
            // Warning: that device not having any site/project meaning that it will be not available to the current user
            GlobalMessageBox msgbox;
            msgbox.showError(tr("Attention"), tr("Aucun site / projet n'a été spécifié.\nVous devez spécifier au moins un site / projet"));
            return false;
        }
    //}
    return true;
}

QJsonArray DeviceWidget::getSelectedProjectsAsJsonArray()
{
    QJsonArray projects;
    for(QTreeWidgetItem* item: qAsConst(m_listProjects_items)){
    //for (int i=0; i<m_listProjects_items.count(); i++){
        int project_id = m_listProjects_items.key(item);
        if (item->checkState(0) == Qt::Checked){
            QJsonObject data_obj;
            data_obj.insert("id_project", project_id);
            projects.append(data_obj);
        }
    }
    return projects;
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
    for (QTreeWidgetItem* item:qAsConst(m_listSites_items)){
        item->setCheckState(0, Qt::Unchecked);
    }
    // Check required sites
    for(const TeraData &device_site:device_sites){
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
    for (QTreeWidgetItem* item:qAsConst(m_listProjects_items)){
        item->setCheckState(0, Qt::Unchecked);
    }
//    bool first_load = m_listSites_items.isEmpty();
    m_listDeviceProjects_items.clear();

    // Check required projects
    for(const TeraData &device_project:device_projects){
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

void DeviceWidget::btnSaveSites_clicked()
{

    if (!validateSitesProjects())
        return;

    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray devices;
    QList<int> projects_to_delete;

    //for (int i=0; i<m_listProjects_items.count(); i++){
    for (QTreeWidgetItem* item: qAsConst(m_listProjects_items)){
        // Build json list of devices and projects
        if (item->checkState(0)==Qt::Checked){
            QJsonObject item_obj;
            item_obj.insert("id_device", m_data->getId());
            item_obj.insert("id_project", m_listProjects_items.key(item));
            devices.append(item_obj);
        }else{
            int id_device_project = m_listDeviceProjects_items.key(item,-1);
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
    if (!devices.isEmpty()){
        base_obj.insert("device_project", devices);
        document.setObject(base_obj);
        postDataRequest(WEB_DEVICEPROJECTINFO_PATH, document.toJson());
    }

}

void DeviceWidget::deleteDataReply(QString path, int id)
{
    if (path == WEB_DEVICEPARTICIPANTINFO_PATH){
        // A participant device association was deleted
        for (int i=0; i<ui->lstParticipants->count(); i++){
            if (ui->lstParticipants->item(i)->data(Qt::UserRole).toInt() == id){
                int id_participant = m_listParticipants_items.key(ui->lstParticipants->item(i));
                m_listParticipants_items.remove(id_participant);
                delete ui->lstParticipants->takeItem(i);
                return;
            }
        }
    }
}

void DeviceWidget::lstSites_itemChanged(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    static bool updating = false;

    // Lock to prevent recursive updates
    if (updating)
        return;

    updating = true;
    if (std::find(m_listSites_items.cbegin(), m_listSites_items.cend(), item) != m_listSites_items.cend()){
        if (item->childCount() == 0 && !isLoading()){
            item->setExpanded(true);
        }else{
            // Check or uncheck all childs (projects)
            for (int i=0; i<item->childCount(); i++){
                item->child(i)->setCheckState(0, item->checkState(0));
            }
        }
    }

    if (std::find(m_listProjects_items.cbegin(), m_listProjects_items.cend(), item) != m_listProjects_items.cend()){
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

void DeviceWidget::on_tabNav_currentChanged(int index)
{
    // Load data depending on selected tab
    QUrlQuery args;
    //args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getFieldValue("id_site").toInt()));

    QWidget* current_tab = ui->tabNav->widget(index);


    if (current_tab == ui->tabSites){
        // Sites / Projects
        if (m_listSites_items.isEmpty() || m_listProjects_items.isEmpty()){
            args.addQueryItem(WEB_QUERY_LIST, "true");
            queryDataRequest(WEB_SITEINFO_PATH, args);
        }
    }

    if (current_tab == ui->tabParticipants){
        // Participants
        if (m_listParticipants_items.isEmpty()){
            args.addQueryItem(WEB_QUERY_ID_DEVICE, QString::number(m_data->getId()));
            queryDataRequest(WEB_DEVICEPARTICIPANTINFO_PATH, args);
        }
    }

    if (current_tab == ui->tabServiceConfig){
        // Service configuration
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


}

void DeviceWidget::on_lstParticipants_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
   Q_UNUSED(previous)
   ui->btnRemoveParticipant->setEnabled(current);
}

void DeviceWidget::on_btnRemoveParticipant_clicked()
{
    if (!ui->lstParticipants->currentItem()){
        ui->btnRemoveParticipant->setEnabled(false);
        return;
    }

    GlobalMessageBox msg;
    int rval = msg.showYesNo(tr("Retirer l'appareil"), tr("Êtes-vous sûrs de vouloir retirer cet appareil de ce participant?\nSi l'appareil est présentement déployé, les données ne seront plus collectés et l'appareil ne sera plus utilisable pendant les séances."));
    if (rval == QMessageBox::Yes){
        int id_device_participant = ui->lstParticipants->currentItem()->data(Qt::UserRole).toInt();
        deleteDataRequest(WEB_DEVICEPARTICIPANTINFO_PATH, id_device_participant);
    }
}
