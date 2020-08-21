#include "ServiceConfigWidget.h"
#include "ui_ServiceConfigWidget.h"

ServiceConfigWidget::ServiceConfigWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::ServiceConfigWidget)
{   

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    setLimited(false);
    m_gotFormStructure = false;

    // Use base class to manage editing
    setEditorControls(ui->wdgServiceConfig, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    ui->wdgServiceConfig->setComManager(m_comManager);
    setData(data);

}

ServiceConfigWidget::~ServiceConfigWidget()
{
    if (ui)
        delete ui;
}

void ServiceConfigWidget::saveData(bool signal){

    QJsonDocument st_data = ui->wdgServiceConfig->getFormDataJson(m_data->isNew());
    postDataRequest(WEB_SERVICECONFIGINFO_PATH, st_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgServiceConfig->getFormDataObject(TERADATA_SERVICE_CONFIG);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void ServiceConfigWidget::updateControlsState(){

}

void ServiceConfigWidget::updateFieldsValue(){
    if (m_data && m_gotFormStructure){
        //ui->wdgServiceConfig->fillFormFromData(m_data->toJson());
        ui->lblTitle->setText(m_data->getName());
    }


}

bool ServiceConfigWidget::validateData(){
    bool valid = false;

    valid = ui->wdgServiceConfig->validateFormData();

    return valid;
}

void ServiceConfigWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type.contains(WEB_FORMS_QUERY_SERVICE_CONFIG)){
        ui->wdgServiceConfig->buildUiFromStructure(data);
        m_gotFormStructure = true;
        updateFieldsValue();
        return;
    }
}

void ServiceConfigWidget::postResultReply(QString path)
{
    // OK, data was saved!
    if (path==WEB_SERVICECONFIGINFO_PATH){
        if (parent())
            emit closeRequest();
    }
}

void ServiceConfigWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &ServiceConfigWidget::processFormsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &ServiceConfigWidget::postResultReply);
}

void ServiceConfigWidget::setData(const TeraData *data)
{
     DataEditorWidget::setData(data);

     // Query form definition
     if (m_data->hasFieldName("id_service")){
         QUrlQuery args(WEB_FORMS_QUERY_SERVICE_CONFIG);
         args.addQueryItem(WEB_QUERY_ID, m_data->getFieldValue("id_service").toString());
         queryDataRequest(WEB_FORMS_PATH, args);
     }
}
