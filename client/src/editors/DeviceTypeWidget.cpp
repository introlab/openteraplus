#include "DeviceTypeWidget.h"
#include "ui_DeviceTypeWidget.h"

DeviceTypeWidget::DeviceTypeWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::DeviceTypeWidget)
{   

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    setLimited(false);

    // Use base class to manage editing
    setEditorControls(ui->wdgDeviceType, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    // Query form definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_DEVICE_TYPE));

    ui->wdgDeviceType->setComManager(m_comManager);
    setData(data);

}

DeviceTypeWidget::~DeviceTypeWidget()
{
    if (ui)
        delete ui;
}

void DeviceTypeWidget::saveData(bool signal){

    // If data is new, we request all the fields.
    QJsonDocument st_data = ui->wdgDeviceType->getFormDataJson(m_data->isNew());

    postDataRequest(WEB_DEVICETYPE_PATH, st_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgDeviceType->getFormDataObject(TERADATA_DEVICETYPE);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void DeviceTypeWidget::updateControlsState(){
    if (dataIsNew()){
        if (ui->tabNav->count() > 1){
            ui->tabNav->removeTab(1);
        }
    }

}

void DeviceTypeWidget::updateFieldsValue(){
    if (m_data){
        ui->wdgDeviceType->fillFormFromData(m_data->toJson());
        ui->lblTitle->setText(m_data->getName());
    }


}

void DeviceTypeWidget::updateDevice(const TeraData *device)
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

bool DeviceTypeWidget::validateData(){
    bool valid = false;

    valid = ui->wdgDeviceType->validateFormData();

    return valid;
}

void DeviceTypeWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type == WEB_FORMS_QUERY_DEVICE_TYPE){
        ui->wdgDeviceType->buildUiFromStructure(data);
        return;
    }
}

void DeviceTypeWidget::processDevicesReply(QList<TeraData> devices)
{
    if (!m_data)
        return;

    for (int i=0; i<devices.count(); i++){
        if (devices.at(i).getFieldValue("id_device_type").toInt() == m_data->getId()){
            updateDevice(&devices.at(i));
        }
    }
}

void DeviceTypeWidget::postResultReply(QString path)
{
    // OK, data was saved!
    if (path==WEB_DEVICETYPE_PATH){
        if (parent())
            emit closeRequest();
    }
}

void DeviceTypeWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &DeviceTypeWidget::processFormsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &DeviceTypeWidget::postResultReply);
    connect(m_comManager, &ComManager::devicesReceived, this, &DeviceTypeWidget::processDevicesReply);
}

void DeviceTypeWidget::on_tabNav_currentChanged(int index)
{
    QUrlQuery query;
    QWidget* current_tab = ui->tabNav->widget(index);

    if (current_tab == ui->tabDevices){
        // Devices
        if (m_listDevices_items.isEmpty() && m_data){
            query.addQueryItem(WEB_QUERY_ID_DEVICE_TYPE, QString::number(m_data->getId()));
            queryDataRequest(WEB_DEVICEINFO_PATH, query);
        }
    }
}

