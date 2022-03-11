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

}

DeviceWidget::~DeviceWidget()
{
    delete ui;
}

void DeviceWidget::saveData(bool signal)
{
    // Site data
    QJsonDocument device_data = ui->wdgDevice->getFormDataJson(m_data->isNew());

    if (dataIsNew() && ui->treeSites->isEnabled()){
        QJsonObject base_obj = device_data.object();
        QJsonObject base_st = base_obj["device"].toObject();

        // For new device, also sends the projects at the same time
        QJsonArray projects = getSelectedProjectsAsJsonArray();
        if (!projects.isEmpty()){
            base_st.insert("device_projects", projects);
            base_obj.insert("device", base_st);
            device_data.setObject(base_obj);
        }

        // ... and sites
        QJsonArray sites = getSelectedSitesAsJsonArray();
        if (!sites.isEmpty()){
            base_st.insert("device_sites", projects);
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
       ui->tabSites->layout()->removeWidget(ui->treeSites);
       ui->tabDashboard->layout()->removeWidget(ui->frameButtons);

       QLabel* lbl = new QLabel(tr("Sites / Projets associés"));
       QFont labelFont;
       labelFont.setBold(true);
       labelFont.setPointSize(10);
       lbl->setFont(labelFont);

       ui->tabDashboard->layout()->addWidget(lbl);
       ui->tabDashboard->layout()->addWidget(ui->treeSites);
       ui->tabDashboard->layout()->addWidget(ui->frameButtons);
       while (ui->tabNav->count() > 1)
           ui->tabNav->removeTab(1);

       // Query sites and projects if needed
       if (m_treeSites_items.isEmpty() || m_treeProjects_items.isEmpty()){
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
    connect(m_comManager, &ComManager::postResultsOK, this, &DeviceWidget::postResultReply);

    connect(ui->btnSites, &QPushButton::clicked, this, &DeviceWidget::btnSaveSites_clicked);

    connect(ui->treeSites, &QTreeWidget::itemChanged, this, &DeviceWidget::lstSites_itemChanged);
}

void DeviceWidget::updateSite(TeraData *site)
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
        ui->treeSites->addTopLevelItem(item);
        m_treeSites_items[id_site] = item;
    }
    item->setText(0, site->getName());
}

void DeviceWidget::updateProject(TeraData *project)
{
    int id_site = project->getFieldValue("id_site").toInt();
    int id_project = project->getId();
    QTreeWidgetItem* item;
    if (m_treeProjects_items.contains(id_project)){
        item = m_treeProjects_items[id_project];
    }else{
        QTreeWidgetItem* site_item = m_treeSites_items[id_site];
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

void DeviceWidget::updateDeviceProject(TeraData *device_project)
{
    int id_project = device_project->getFieldValue("id_project").toInt();
    QTreeWidgetItem* item;
    QString project_name = device_project->getFieldValue("project_name").toString();

    if (m_treeProjects_items.contains(id_project)){
        item = m_treeProjects_items[id_project];
    }else{
        item = new QTreeWidgetItem();
        item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_PROJECT)));
        int id_site = device_project->getFieldValue("id_site").toInt();
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

    int device_id = device_project->getFieldValue("id_device").toInt();
    if (device_id > 0){
        item->setCheckState(0, Qt::Checked);
    }else{
        item->setCheckState(0, Qt::Unchecked);
    }
    item->setData(0, Qt::UserRole, item->checkState(0));
}

void DeviceWidget::updateDeviceSite(TeraData *device_site)
{
    int id_site = device_site->getFieldValue("id_site").toInt();
    QTreeWidgetItem* item;
    if (m_treeSites_items.contains(id_site)){
        item = m_treeSites_items[id_site];
    }else{
        item = new QTreeWidgetItem();
        item->setIcon(0, QIcon(TeraData::getIconFilenameForDataType(TERADATA_SITE)));
        if (m_comManager->isCurrentUserSuperAdmin())
            item->setCheckState(0, Qt::Unchecked);
        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        ui->treeSites->addTopLevelItem(item);
        m_treeSites_items[id_site] = item;
    }
    QString site_name = device_site->getFieldValue("site_name").toString();
    if (!site_name.isEmpty())
        item->setText(0, site_name);

    if (m_comManager->isCurrentUserSuperAdmin()){
        if (device_site->getId() > 0){
            item->setCheckState(0, Qt::Checked);
        }else{
            item->setCheckState(0, Qt::Unchecked);
        }
        item->setData(0, Qt::UserRole, item->checkState(0));
    }else{
        item->setData(0, Qt::UserRole, Qt::PartiallyChecked);
    }
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

void DeviceWidget::postDeviceSites()
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

    QJsonObject device_obj;
    device_obj.insert("id_device", m_data->getId());
    device_obj.insert("sites", sites);
    base_obj.insert("device", device_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_DEVICESITEINFO_PATH, document.toJson());
}

void DeviceWidget::postDeviceProjects()
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

    QJsonObject device_obj;
    device_obj.insert("id_device", m_data->getId());
    device_obj.insert("projects", projects);
    base_obj.insert("device", device_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_DEVICEPROJECTINFO_PATH, document.toJson());
}

QJsonArray DeviceWidget::getSelectedProjectsAsJsonArray()
{
    QJsonArray projects;
    for(QTreeWidgetItem* item: qAsConst(m_treeProjects_items)){
    //for (int i=0; i<m_listProjects_items.count(); i++){
        int project_id = m_treeProjects_items.key(item);
        if (item->checkState(0) == Qt::Checked){
            QJsonObject data_obj;
            data_obj.insert("id_project", project_id);
            projects.append(data_obj);
        }
    }
    return projects;
}

QJsonArray DeviceWidget::getSelectedSitesAsJsonArray()
{
    QJsonArray sites;
    for(QTreeWidgetItem* item: qAsConst(m_treeSites_items)){
        int site_id = m_treeSites_items.key(item);
        if (item->checkState(0) == Qt::Checked){
            QJsonObject data_obj;
            data_obj.insert("id_site", site_id);
            sites.append(data_obj);
        }
    }
    return sites;
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
    for(TeraData site:sites){
        updateSite(&site);
    }

    // Query projects for sites
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_LIST, "true");
    queryDataRequest(WEB_PROJECTINFO_PATH, args);
}

void DeviceWidget::processDeviceSitesReply(QList<TeraData> device_sites)
{
    for (TeraData site:device_sites){
        updateDeviceSite(&site);
    }

    // Query projects for device
    if (m_treeProjects_items.isEmpty()){
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_DEVICE, QString::number(m_data->getId()));
        args.addQueryItem(WEB_QUERY_WITH_PROJECTS, "1");
        args.addQueryItem(WEB_QUERY_WITH_SITES, "1");
        queryDataRequest(WEB_DEVICEPROJECTINFO_PATH, args);
    }
}

void DeviceWidget::processDeviceProjectsReply(QList<TeraData> device_projects)
{
    for (TeraData project:device_projects){
        updateDeviceProject(&project);
    }

    ui->treeSites->expandAll();

    ui->btnSites->setEnabled(false);
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
}

void DeviceWidget::btnSaveSites_clicked()
{
    bool has_removed = false;

    for(QTreeWidgetItem* item: qAsConst(m_treeSites_items)){
        if (item->checkState(0) == Qt::Unchecked && item->data(0, Qt::UserRole) == Qt::Checked){
            has_removed = true;
            break;
        }
    }

    if (!has_removed){
        for(QTreeWidgetItem* item: qAsConst(m_treeProjects_items)){
            if (item->checkState(0) == Qt::Unchecked && item->data(0, Qt::UserRole) == Qt::Checked){
                has_removed = true;
                break;
            }
        }
    }

    if (has_removed){
        GlobalMessageBox msgbox;
        int rval = msgbox.showYesNo(tr("Suppression de site/projet associé"), tr("Au moins un site ou un projet a été retiré de cet appareil. S'il y a des projets qui utilisent cet appareil, ils ne pourront plus l'utiliser.\nSouhaitez-vous continuer?"));
        if (rval != GlobalMessageBox::Yes){
            return;
        }

    }

    if (m_comManager->isCurrentUserSuperAdmin()){
        // Save device sites
        postDeviceSites();
    }else{
        // Not super admin - save device projects
        postDeviceProjects();
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

void DeviceWidget::postResultReply(QString path)
{
    if (path==WEB_DEVICESITEINFO_PATH){
        // We saved the device-site association, now saving the device-project association
        postDeviceProjects();
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

    bool has_changes = false;
    if (m_comManager->isCurrentUserSuperAdmin()){
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
                        break;
                    }
                }
                // No projects selected for that site
                //site->setCheckState(0, Qt::Unchecked);
            }
        }
    }

    updating = false;

    for(QTreeWidgetItem* site: qAsConst(m_treeSites_items)){
        if (site->checkState(0) != site->data(0, Qt::UserRole) && site->data(0, Qt::UserRole) != Qt::PartiallyChecked){
            has_changes = true;
            break;
        }
    }
    if (!has_changes){
        for(QTreeWidgetItem* proj: qAsConst(m_treeProjects_items)){
            if (proj->checkState(0) != proj->data(0, Qt::UserRole)){
                has_changes = true;
                break;
            }
        }
    }
    ui->btnSites->setEnabled(has_changes);

}

void DeviceWidget::on_tabNav_currentChanged(int index)
{
    // Load data depending on selected tab
    QUrlQuery args;
    //args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getFieldValue("id_site").toInt()));

    QWidget* current_tab = ui->tabNav->widget(index);


    if (current_tab == ui->tabSites){
        // Sites / Projects
        if (m_treeSites_items.isEmpty() || m_treeSites_items.isEmpty()){
            args.addQueryItem(WEB_QUERY_ID_DEVICE, QString::number(m_data->getId()));
            args.addQueryItem(WEB_QUERY_WITH_SITES, "1");
            queryDataRequest(WEB_DEVICESITEINFO_PATH, args);
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
