#include "ServiceConfigWidget.h"
#include "ui_ServiceConfigWidget.h"

ServiceConfigWidget::ServiceConfigWidget(ComManager *comMan, const QString id_field_name, const int id_field_value, QWidget *parent) :
    DataEditorWidget(comMan, nullptr, parent),
    ui(new Ui::ServiceConfigWidget)
{   

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    setLimited(false);
    ui->frameEditor->setVisible(false);
    ui->wdgServiceConfig->hide();
    ui->frameSpecific->hide();

    // Use base class to manage editing
    //setEditorControls(ui->wdgServiceConfig, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    ui->wdgServiceConfig->setComManager(m_comManager);
    ui->wdgServiceConfigConfig->setComManager(m_comManager);

    m_idFieldName = id_field_name;
    m_idFieldValue = id_field_value;

    // Check if we are editing someone else configuration (Global config) or our own (Specific config)
    if (m_idFieldName == "id_user"){
        // For now, only users can have specific service configs in that software.
        if (m_idFieldValue == m_comManager->getCurrentUser().getId()){
            // We are editing ourself, so we are having a specific config
            m_specificId = Utils::getMachineUniqueId();
        }
    }

    // Query services
    QUrlQuery args;
    //args.addQueryItem(id_field_name, QString::number(id_field_value));
    args.addQueryItem(WEB_QUERY_WITH_CONFIG, "1"); // Get list of all services with a config
    args.addQueryItem(WEB_QUERY_LIST, "1"); // List only, not all infos
    /*if (!m_specificId.isEmpty()){
        args.addQueryItem(WEB_QUERY_ID_SPECIFIC, m_specificId);
    }*/
    queryDataRequest(WEB_SERVICEINFO_PATH, args);

    // Query generic service config form
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_SERVICE_CONFIG));

}

ServiceConfigWidget::~ServiceConfigWidget()
{
    if (ui)
        delete ui;
}

void ServiceConfigWidget::saveData(bool signal){

    // Config itself
    QString service_config = ui->wdgServiceConfigConfig->getFormData(true);

    // Remove structure name
    service_config = service_config.replace("\"service_config_config\": {", "");
    service_config = service_config.left(service_config.length()-2);
    if (!ui->wdgServiceConfig->setFieldValue("service_config_config", service_config)){
        LOG_ERROR(tr("Field service_config_config can't be set."), "ServiceConfigWidget::saveData");
    }
    //qDebug() << service_config;

    QJsonDocument service_config_data = ui->wdgServiceConfig->getFormDataJson(true);
    QJsonObject service_config_obj = service_config_data.object();
    QJsonObject service_config_base = service_config_obj.value("service_config").toObject();

    if (!m_specificId.isEmpty()){
        // We must add the specific id value to the document
        service_config_base.insert(WEB_QUERY_ID_SPECIFIC, m_specificId);
    }

    service_config_obj.insert("service_config", service_config_base);
    service_config_data.setObject(service_config_obj);

    postDataRequest(WEB_SERVICECONFIGINFO_PATH, service_config_data.toJson());

    if (signal){
        /*TeraData* new_data = ui->wdgServiceConfig->getFormDataObject(TERADATA_SERVICE_CONFIG);
        *m_data = *new_data;
        delete new_data;*/
        emit dataWasChanged();
    }

}

void ServiceConfigWidget::updateControlsState(){
    ui->wdgServiceConfig->setDisabled(m_limited);
    ui->wdgServiceConfigConfig->setDisabled(m_limited);
    ui->frameButtons->setVisible(!m_limited && ui->wdgServiceConfigConfig->formHasStructure());
}

void ServiceConfigWidget::updateFieldsValue(){
    if (!m_data)
        return;

    if (m_data->hasFieldName("id_service_config")){
        ui->wdgServiceConfig->setFieldValue("id_service_config", m_data->getFieldValue("id_service_config"));
    }

    if (m_data->hasFieldName("service_config_config")){
        ui->wdgServiceConfigConfig->fillFormFromData(m_data->toJson("service_config_config"));
    }

    if (m_data->hasFieldName("service_config_specifics") && m_specificId.isEmpty() && ui->cmbSpecific->count() == 0){
        setLoading();
        ui->cmbSpecific->clear();
        QVariantList specifics = m_data->getFieldValue("service_config_specifics").toList();
        if (!specifics.isEmpty()){
            ui->cmbSpecific->addItem(tr("Globale"));
            for (QVariant specific_id:specifics){
                ui->cmbSpecific->addItem(specific_id.toString());
            }
            ui->frameSpecific->show();
        }
        setReady();
    }
}

void ServiceConfigWidget::updateServiceConfig(const TeraData *config)
{
    setData(config);
}

void ServiceConfigWidget::updateService(const TeraData *service)
{
    int id_service = service->getFieldValue("id_service").toInt();
    QString service_name = service->getFieldValue("service_name").toString();
    QListWidgetItem* item;

    if (m_listServices_items.contains(id_service)){
        // Service already existing
        item = m_listServices_items[id_service];
    }else{
        // New service with a config
        item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_SERVICE_CONFIG)), service_name);
        ui->lstServiceConfig->addItem(item);
        m_listServices_items[id_service] = item;
    }
    item->setText(service_name);
}

bool ServiceConfigWidget::validateData(){
    bool valid = false;

    valid = ui->wdgServiceConfig->validateFormData() & ui->wdgServiceConfigConfig->validateFormData() ;


    return valid;
}

void ServiceConfigWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type.contains(WEB_FORMS_QUERY_SERVICE_CONFIG)){
        int id_service = m_listServices_items.key(ui->lstServiceConfig->currentItem());
        if (!form_type.contains(WEB_QUERY_ID)){
            // Generic service config form
            ui->wdgServiceConfig->buildUiFromStructure(data);

            // Set values
            ui->wdgServiceConfig->setFieldValue(m_idFieldName, m_idFieldValue);
        }else{
            // Specific config form for a service
            ui->wdgServiceConfig->setFieldValue("id_service", id_service);
            ui->wdgServiceConfigConfig->buildUiFromStructure(data);
            updateFieldsValue();
            ui->frameEditor->setVisible(true);
        }
        m_gotServiceForm = true;
        return;
    }
}

void ServiceConfigWidget::processServiceConfigsReply(QList<TeraData> configs, QUrlQuery query)
{
    Q_UNUSED(query)

    foreach (TeraData config, configs){
        if (config.hasFieldName(m_idFieldName)){
            if (config.getFieldValue(m_idFieldName).toInt() == m_idFieldValue){
                // Service config is for us!
                updateServiceConfig(&config);
            }
        }
    }

    // Request form for that specific config
    if (!m_gotServiceForm){
        int id_service = m_listServices_items.key(ui->lstServiceConfig->currentItem());
        QUrlQuery args(WEB_FORMS_QUERY_SERVICE_CONFIG);
        args.addQueryItem(WEB_QUERY_ID, QString::number(id_service));
        queryDataRequest(WEB_FORMS_PATH, args);
    }
}

void ServiceConfigWidget::processServicesReply(QList<TeraData> services, QUrlQuery query)
{
    Q_UNUSED(query)

    foreach (TeraData service, services){
        if (service.hasFieldName("service_editable_config")){
            if (service.getFieldValue("service_editable_config").toBool())
                updateService(&service);
        }

    }
}

void ServiceConfigWidget::postResultReply(QString path)
{
    // OK, data was saved!
    if (path==WEB_SERVICECONFIGINFO_PATH){
        // Refresh value
        on_lstServiceConfig_itemClicked(ui->lstServiceConfig->currentItem());
        m_gotServiceForm = true;
        if (parent())
            emit closeRequest();
    }
}

void ServiceConfigWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &ServiceConfigWidget::processFormsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &ServiceConfigWidget::postResultReply);
    connect(m_comManager, &ComManager::servicesConfigReceived, this, &ServiceConfigWidget::processServiceConfigsReply);
    connect(m_comManager, &ComManager::servicesReceived, this, &ServiceConfigWidget::processServicesReply);
}


void ServiceConfigWidget::on_lstServiceConfig_itemClicked(QListWidgetItem *item)
{
    ui->cmbSpecific->clear();
    ui->frameSpecific->hide();

    int id_service = m_listServices_items.key(item);
    // Query config for that service
    QUrlQuery args;
    args.addQueryItem(m_idFieldName, QString::number(m_idFieldValue));
    args.addQueryItem(WEB_QUERY_LIST, "1"); // Also lists specific ids
    args.addQueryItem(WEB_QUERY_ID_SERVICE, QString::number(id_service));
    if (!m_specificId.isEmpty()){
        args.addQueryItem(WEB_QUERY_ID_SPECIFIC, m_specificId);
    }
    queryDataRequest(WEB_SERVICECONFIGINFO_PATH, args);

    m_gotServiceForm = false;

    // Query form for that service
    /*

    QUrlQuery args(WEB_FORMS_QUERY_SERVICE_CONFIG);
    args.addQueryItem(WEB_QUERY_ID, QString::number(id_service));
    queryDataRequest(WEB_FORMS_PATH, args);*/

    ui->wdgServiceConfig->setFieldValue("id_service", id_service);

    ui->lblTitle->setText(item->text());

}

void ServiceConfigWidget::on_btnSave_clicked()
{
    if (validateData())
        saveData();
    else{
        QStringList invalids = ui->wdgServiceConfigConfig->getInvalidFormDataLabels();

        if (!invalids.isEmpty()){
            QString msg = tr("Les champs suivants doivent être complétés:") +" <ul>";
            for (QString field:invalids){
                msg += "<li>" + field + "</li>";
            }
            msg += "</ul>";
            GlobalMessageBox msgbox(this);
            msgbox.showError(tr("Champs invalides"), msg);
        }
    }

}

void ServiceConfigWidget::on_btnUndo_clicked()
{
    ui->wdgServiceConfigConfig->resetFormValues();
    undoData();
}

void ServiceConfigWidget::on_cmbSpecific_currentIndexChanged(const QString &current_id)
{
    if (isLoading())
        return;

    // Reload data for requested config
    int id_service = m_listServices_items.key(ui->lstServiceConfig->currentItem());
    QUrlQuery args;
    args.addQueryItem(m_idFieldName, QString::number(m_idFieldValue));
    args.addQueryItem(WEB_QUERY_ID_SERVICE, QString::number(id_service));
    if (ui->cmbSpecific->currentIndex()>0){
        args.addQueryItem(WEB_QUERY_ID_SPECIFIC, current_id);
        m_specificId = current_id;
    }else{
        m_specificId.clear();
    }
    queryDataRequest(WEB_SERVICECONFIGINFO_PATH, args);

}
