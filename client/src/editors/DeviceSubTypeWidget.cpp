#include "DeviceSubTypeWidget.h"
#include "ui_DeviceSubTypeWidget.h"

DeviceSubTypeWidget::DeviceSubTypeWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::DeviceSubTypeWidget)
{   

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    setLimited(false);

    // Use base class to manage editing
    setEditorControls(ui->wdgDeviceSubType, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

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
   if (dataIsNew()){
       if (ui->tabNav->count() > 1){
           ui->tabNav->removeTab(1);
       }
   }
}

void DeviceSubTypeWidget::updateFieldsValue(){
    if (m_data){
        ui->wdgDeviceSubType->fillFormFromData(m_data->toJson());
        ui->lblTitle->setText(m_data->getName());
    }


}

void DeviceSubTypeWidget::updateDevice(const TeraData *device)
{
    int id_device = device->getId();
    QString device_name = device->getName();
    if (m_listDevices_items.contains(id_device)){
        QListWidgetItem* item = m_listDevices_items[id_device];
        item->setText(device_name);
    }else{
        QListWidgetItem* item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_DEVICE)), device_name);
        ui->lstDevices->addItem(item);
        m_listDevices_items[id_device] = item;
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

void DeviceSubTypeWidget::processDevicesReply(QList<TeraData> devices)
{
    if (!m_data)
        return;

    for (int i=0; i<devices.count(); i++){
        if (devices.at(i).getFieldValue("id_device_subtype").toInt() == m_data->getId()){
            updateDevice(&devices.at(i));
        }
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
    connect(m_comManager, &ComManager::devicesReceived, this, &DeviceSubTypeWidget::processDevicesReply);
}

void DeviceSubTypeWidget::on_tabNav_currentChanged(int index)
{
    QUrlQuery query;
    QWidget* current_tab = ui->tabNav->widget(index);

    if (current_tab == ui->tabDevices){
        // Devices
        if (m_listDevices_items.isEmpty() && m_data){
            query.addQueryItem(WEB_QUERY_ID_DEVICE_SUBTYPE, QString::number(m_data->getId()));
            queryDataRequest(WEB_DEVICEINFO_PATH, query);
        }
    }
}

