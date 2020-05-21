#include "SessionWidget.h"
#include "ui_SessionWidget.h"

#include <QFileDialog>
#include <QStandardPaths>

SessionWidget::SessionWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::SessionWidget)
{

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    ui->wdgSession->setHighlightConditions(false); // Hide conditional questions indicator

    setLimited(false);

    // Use base class to manage editing
    setEditorControls(ui->wdgSession, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    // Query form definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_SESSION));

    ui->wdgSession->setComManager(m_comManager);
    setData(data);

}

SessionWidget::~SessionWidget()
{
    if (ui)
        delete ui;
}

void SessionWidget::saveData(bool signal){

    // If data is new, we request all the fields.
    QJsonDocument group_data = ui->wdgSession->getFormDataJson(m_data->isNew());

    postDataRequest(WEB_SESSIONINFO_PATH, group_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgSession->getFormDataObject(TERADATA_GROUP);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void SessionWidget::setData(const TeraData *data)
{
    DataEditorWidget::setData(data);

    if (!dataIsNew()){
        // Loads first detailled informations tab
        on_tabSessionInfos_tabBarClicked(0);

    }else{
        ui->tabDetails->setEnabled(false);
    }
}

void SessionWidget::updateControlsState(){

}

void SessionWidget::updateFieldsValue(){
    if (m_data){
        ui->wdgSession->fillFormFromData(m_data->toJson());
        ui->lblTitle->setText(m_data->getName());

        int session_status = m_data->getFieldValue("session_status").toInt();
        ui->lblStatus->setText(tr("Séance: ") + TeraSessionStatus::getStatusName(session_status));
        ui->lblStatus->setStyleSheet("background-color: " + TeraSessionStatus::getStatusColor(session_status) + "; color: black;");
    }
}

bool SessionWidget::validateData(){
    bool valid = false;

    valid = ui->wdgSession->validateFormData();

    return valid;
}

void SessionWidget::updateParticipant(TeraData *participant)
{
    QListWidgetItem* item = nullptr;
    for(int i=0; i<ui->lstParticipants->count(); i++){
        int part_id = ui->lstParticipants->item(i)->data(Qt::UserRole).toInt();
        if (part_id == participant->getId()){
            // Participant already present
            item = ui->lstParticipants->item(i);
            break;
        }
    }

    // New participant
    if (!item){
        item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_PARTICIPANT)), participant->getName());
        item->setData(Qt::UserRole, participant->getId());
        ui->lstParticipants->addItem(item);
    }

    // Update participant name
    item->setText(participant->getName());
}

void SessionWidget::updateDeviceData(TeraData *device_data)
{
    int id_device_data = device_data->getId();
    QTableWidgetItem* base_item;

    if (m_listDeviceDatas.contains(id_device_data)){
        // Item is already present
        base_item = m_listDeviceDatas[id_device_data];
    }else{
        ui->tableData->setRowCount(ui->tableData->rowCount()+1);
        int current_row = ui->tableData->rowCount()-1;

        // Must create new item
        base_item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_DEVICEDATA)),"");
        ui->tableData->setItem(current_row, 0, base_item);
        m_listDeviceDatas[id_device_data] = base_item;

        QTableWidgetItem* item = new QTableWidgetItem();
        ui->tableData->setItem(current_row, 1, item);

        item = new QTableWidgetItem();
        ui->tableData->setItem(current_row, 2, item);

        item = new QTableWidgetItem();
        ui->tableData->setItem(current_row, 3, item);

        // Action button
        QPushButton* btnDownload = new QPushButton(tr("Télécharger"));
        btnDownload->setProperty("id_device_data", device_data->getId());
        btnDownload->setCursor(Qt::PointingHandCursor);
        connect(btnDownload, &QPushButton::clicked, this, &SessionWidget::btnDownload_clicked);
        ui->tableData->setCellWidget(current_row, 4, btnDownload);
    }

    // Update values
    base_item->setText(device_data->getFieldValue("device_name").toString());
    ui->tableData->item(base_item->row(), 1)->setText(device_data->getFieldValue("devicedata_saved_date").toDateTime().toString("dd-MM-yyyy hh:mm:ss"));
    ui->tableData->item(base_item->row(), 2)->setText(device_data->getFieldValue("devicedata_name").toString());
    int file_size = device_data->getFieldValue("devicedata_filesize").toInt();
    QString suffix = " MB";
    if (file_size < 1024*1024){
        suffix = " KB";
        file_size /= 1024;
    }else{
        file_size /= (1024*1024);
    }
    ui->tableData->item(base_item->row(), 3)->setText(QString::number(file_size, 'f', 2) + suffix);
}

void SessionWidget::updateEvent(TeraData *event)
{
    int id_event = event->getId();
    int event_type = event->getFieldValue("id_session_event_type").toInt();
    QTableWidgetItem* base_item;

    if (m_listSessionEvents.contains(id_event)){
        // Item is already present
        base_item = m_listSessionEvents[id_event];
    }else{
        ui->tableEvents->setRowCount(ui->tableEvents->rowCount()+1);
        int current_row = ui->tableEvents->rowCount()-1;

        // Must create new item
        base_item = new QTableWidgetItem();
        base_item->setIcon(QIcon(TeraSessionEvent::getIconFilenameForEventType(event_type)));

        ui->tableEvents->setItem(current_row, 0, base_item);
        m_listSessionEvents[id_event] = base_item;

        QTableWidgetItem* item = new QTableWidgetItem();
        ui->tableEvents->setItem(current_row, 1, item);

        item = new QTableWidgetItem();
        ui->tableEvents->setItem(current_row, 2, item);

        item = new QTableWidgetItem();
        ui->tableEvents->setItem(current_row, 3, item);
    }

    // Update values
    // Type, Date, Context, Description
    base_item->setText(TeraSessionEvent::getEventTypeName(event_type));
    ui->tableEvents->item(base_item->row(), 1)->setText(event->getFieldValue("session_event_datetime").toDateTime().toString("dd-MM-yyyy hh:mm:ss"));
    ui->tableEvents->item(base_item->row(), 2)->setText(event->getFieldValue("session_event_context").toString());
    ui->tableEvents->item(base_item->row(), 3)->setText(event->getFieldValue("session_event_text").toString());
}

void SessionWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type == WEB_FORMS_QUERY_SESSION){
        ui->wdgSession->buildUiFromStructure(data);
        return;
    }
}

void SessionWidget::processParticipantsReply(QList<TeraData> participants)
{
    if (!m_data)
        return;

    QList<QVariant> part_ids;
    if (m_data->hasFieldName("session_participants_ids")){
        part_ids = m_data->getFieldValue("session_participants_ids").toList();
    }

    for (TeraData part:participants){
        // Participant is part of the current session
        if (part_ids.contains(part.getId())){
            updateParticipant(&part);
        }
    }

}

void SessionWidget::processDeviceDatasReply(QList<TeraData> device_datas)
{
    for (TeraData device_data:device_datas){
        if (device_data.getFieldValue("id_session").toInt() == m_data->getId()){
            // Device data is for us
            updateDeviceData(&device_data);
        }
    }
}

void SessionWidget::processSessionEventsReply(QList<TeraData> events)
{
    for (TeraData event:events){
        if (event.getFieldValue("id_session").toInt() == m_data->getId()){
            // Event is for us
            updateEvent(&event);
        }
    }
}

void SessionWidget::postResultReply(QString path)
{
    Q_UNUSED(path)
    // OK, data was saved!
    /*if (path == WEB_SESSIONINFO_PATH){
        if (parent())
            emit closeRequest();
    }*/
}

void SessionWidget::deleteDataReply(QString path, int id)
{
    if (path == WEB_DEVICEDATAINFO_PATH){
        // Remove data from list
        if (m_listDeviceDatas.contains(id)){
            ui->tableData->removeRow(m_listDeviceDatas[id]->row());
            m_listDeviceDatas.remove(id);
            qDebug() << ui->tableData->rowCount();
        }
    }
}

void SessionWidget::onDownloadCompleted(DownloadedFile *file)
{
    if (!m_comManager->hasPendingDownloads()){
        setEnabled(true);
        setReady();
        GlobalMessageBox msgbox;
        msgbox.showInfo(tr("Téléchargement"), tr("Téléchargement terminé: ") + file->getFullFilename());
    }
}

void SessionWidget::currentSelectedDataChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    Q_UNUSED(previous)

    ui->btnDelData->setEnabled(current);
}

void SessionWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &SessionWidget::processFormsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &SessionWidget::postResultReply);
    connect(m_comManager, &ComManager::participantsReceived, this, &SessionWidget::processParticipantsReply);
    connect(m_comManager, &ComManager::deviceDatasReceived, this, &SessionWidget::processDeviceDatasReply);
    connect(m_comManager, &ComManager::sessionEventsReceived, this, &SessionWidget::processSessionEventsReply);
    connect(m_comManager, &ComManager::downloadCompleted, this, &SessionWidget::onDownloadCompleted);
    connect(m_comManager, &ComManager::deleteResultsOK, this, &SessionWidget::deleteDataReply);

    connect(ui->btnDelData, &QPushButton::clicked, this, &SessionWidget::btnDeleteData_clicked);
    connect(ui->btnDownloadAll, &QPushButton::clicked, this, &SessionWidget::btnDownloadAll_clicked);
    connect(ui->tableData, &QTableWidget::currentItemChanged, this, &SessionWidget::currentSelectedDataChanged);
}

void SessionWidget::btnDownload_clicked()
{
    QPushButton* button = dynamic_cast<QPushButton*>(QObject::sender());

    if (button){
        // Query folder to save file
        QString save_path = QFileDialog::getExistingDirectory(this, tr("Sélectionnez un dossier pour le téléchargement"),
                                                              QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        if (!save_path.isEmpty()){
            int id_device_data = button->property("id_device_data").toInt();
            QUrlQuery args;
            args.addQueryItem(WEB_QUERY_DOWNLOAD, "");
            args.addQueryItem(WEB_QUERY_ID_DEVICE_DATA, QString::number(id_device_data));
            downloadDataRequest(save_path, WEB_DEVICEDATAINFO_PATH, args);
            setEnabled(false);
        }
    }
}

void SessionWidget::btnDeleteData_clicked()
{
    QTableWidgetItem* item_todel = ui->tableData->currentItem();

    if (!item_todel)
        return;

    QString data_name = ui->tableData->item(item_todel->row(), 2)->text();
    QString device_name = ui->tableData->item(item_todel->row(), 0)->text();

    GlobalMessageBox diag;
    QMessageBox::StandardButton answer = diag.showYesNo(tr("Supression?"),
                                                        tr("Êtes-vous sûrs de vouloir supprimer les données """) + data_name + """ de l'appareil '" + device_name + "'?");
    if (answer == QMessageBox::Yes){
        // We must delete!
        int id_device_data = m_listDeviceDatas.key(ui->tableData->item(item_todel->row(), 0));
        m_comManager->doDelete(WEB_DEVICEDATAINFO_PATH, id_device_data);
    }
}

void SessionWidget::btnDownloadAll_clicked()
{
    // Query folder to save file
    QString save_path = QFileDialog::getExistingDirectory(this, tr("Sélectionnez un dossier pour le téléchargement"),
                                                          QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    if (!save_path.isEmpty()){
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_DOWNLOAD, "");
        args.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(m_data->getId()));
        downloadDataRequest(save_path, WEB_DEVICEDATAINFO_PATH, args);
        setEnabled(false);
    }
}

void SessionWidget::on_tabSessionInfos_tabBarClicked(int index)
{
    // Load data depending on selected tab
    QUrlQuery query;

    if (index == 0){
        // Participants
        query.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(m_data->getId()));
        queryDataRequest(WEB_PARTICIPANTINFO_PATH, query);
    }

    if (index == 1){
        // Data
        if (m_listDeviceDatas.isEmpty()){
            // Query session device data
            query.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(m_data->getId()));
            queryDataRequest(WEB_DEVICEDATAINFO_PATH, query);
        }
    }

    if (index == 2){
        // Tests
        // TODO
    }

    if (index == 3){
        // Session events
        if (m_listSessionEvents.isEmpty()){
            query.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(m_data->getId()));
            queryDataRequest(WEB_SESSIONEVENT_PATH, query);
        }
    }
}
