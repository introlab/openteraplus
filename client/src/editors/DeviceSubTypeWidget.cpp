#include "DeviceSubTypeWidget.h"
#include "ui_DeviceSubTypeWidget.h"

DeviceSubTypeWidget::DeviceSubTypeWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::DeviceSubTypeWidget)
{   

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    setLimited(false);

    // Connect signals and slots
    connectSignals();

    // Query form definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_DEVICE_SUBTYPE));

    ui->wdgDeviceSubType->setComManager(m_comManager);
    setData(data);

}

DeviceSubTypeWidget::~DeviceSubTypeWidget()
{
    if (ui)
        delete ui;
}

void DeviceSubTypeWidget::saveData(bool signal){

    // If data is new, we request all the fields.
    QJsonDocument st_data = ui->wdgDeviceSubType->getFormDataJson(m_data->isNew());

    postDataRequest(WEB_DEVICESUBTYPE_PATH, st_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgDeviceSubType->getFormDataObject(TERADATA_DEVICESUBTYPE);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void DeviceSubTypeWidget::updateControlsState(){

    ui->wdgDeviceSubType->setEnabled(!isWaitingOrLoading() && !m_limited);

    // Buttons update
    ui->btnSave->setEnabled(!isWaitingOrLoading());
    ui->btnUndo->setEnabled(!isWaitingOrLoading());

    ui->frameButtons->setVisible(!m_limited);

}

void DeviceSubTypeWidget::updateFieldsValue(){
    if (m_data){
        ui->wdgDeviceSubType->fillFormFromData(m_data->toJson());
    }
}

bool DeviceSubTypeWidget::validateData(){
    bool valid = false;

    valid = ui->wdgDeviceSubType->validateFormData();

    return valid;
}

void DeviceSubTypeWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type == WEB_FORMS_QUERY_DEVICE_SUBTYPE){
        ui->wdgDeviceSubType->buildUiFromStructure(data);
        return;
    }
}

void DeviceSubTypeWidget::postResultReply(QString path)
{
    // OK, data was saved!
    if (path==WEB_DEVICESUBTYPE_PATH){
        if (parent())
            emit closeRequest();
    }
}

void DeviceSubTypeWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &DeviceSubTypeWidget::processFormsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &DeviceSubTypeWidget::postResultReply);

    connect(ui->btnUndo, &QPushButton::clicked, this, &DeviceSubTypeWidget::btnUndo_clicked);
    connect(ui->btnSave, &QPushButton::clicked, this, &DeviceSubTypeWidget::btnSave_clicked);
}

void DeviceSubTypeWidget::btnSave_clicked()
{
    if (!validateData()){
        QStringList invalids = ui->wdgDeviceSubType->getInvalidFormDataLabels();

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

void DeviceSubTypeWidget::btnUndo_clicked()
{
    undoOrDeleteData();

    if (parent())
        emit closeRequest();

}

