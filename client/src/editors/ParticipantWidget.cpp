#include "ParticipantWidget.h"
#include "ui_ParticipantWidget.h"

#include <QLocale>
#include <QClipboard>
#include <QThread>

#include "editors/DataListWidget.h"
#include "editors/SessionWidget.h"

ParticipantWidget::ParticipantWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::ParticipantWidget)
{

    m_diag_editor = nullptr;
    m_sessionLobby = nullptr;

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image
    setLimited(false);

    // Use base class to manage editing
    setEditorControls(ui->wdgParticipant, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Initialize user interface
    initUI();

    // Connect signals and slots
    connectSignals();

    // Query form definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_PARTICIPANT));
    ui->wdgParticipant->setComManager(m_comManager);
    setData(data);

    // Query sessions types
    queryDataRequest(WEB_SESSIONTYPE_PATH);

    // Query devices for that participant
    if (!m_data->isNew()){
        /*QUrlQuery query;
        query.addQueryItem(WEB_QUERY_ID_PARTICIPANT, QString::number(m_data->getId()));
        queryDataRequest(WEB_DEVICEPARTICIPANTINFO_PATH, query);

        // Query devices for the current site
        query.removeQueryItem(WEB_QUERY_ID_PARTICIPANT);
        query.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getFieldValue("id_project").toInt()));
        queryDataRequest(WEB_DEVICEPROJECTINFO_PATH, query);*/
        //query.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_data->getFieldValue("id_site").toInt()));
        //queryDataRequest(WEB_DEVICESITEINFO_PATH, query);
    }else{
        updateCalendars(QDate::currentDate());
    }

    // Default display
    ui->tabNav->setCurrentIndex(0);
    ui->tabInfos->setCurrentIndex(0);
}

ParticipantWidget::~ParticipantWidget()
{
    delete ui;
    qDeleteAll(m_ids_session_types);
    qDeleteAll(m_ids_sessions);

    if (m_sessionLobby)
        m_sessionLobby->deleteLater();
}

void ParticipantWidget::saveData(bool signal)
{
    // If data is new, we request all the fields.
    QJsonDocument part_data = ui->wdgParticipant->getFormDataJson(m_data->isNew());

    postDataRequest(WEB_PARTICIPANTINFO_PATH, part_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgParticipant->getFormDataObject(TERADATA_PARTICIPANT);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void ParticipantWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &ParticipantWidget::processFormsReply);
    connect(m_comManager, &ComManager::sessionsReceived, this, &ParticipantWidget::processSessionsReply);
    connect(m_comManager, &ComManager::sessionTypesReceived, this, &ParticipantWidget::processSessionTypesReply);
    connect(m_comManager, &ComManager::deviceProjectsReceived, this, &ParticipantWidget::processDeviceProjectsReply);
    connect(m_comManager, &ComManager::deviceParticipantsReceived, this, &ParticipantWidget::processDeviceParticipantsReply);
    connect(m_comManager, &ComManager::deleteResultsOK, this, &ParticipantWidget::deleteDataReply);
    connect(m_comManager, &ComManager::downloadCompleted, this, &ParticipantWidget::onDownloadCompleted);

    connect(ui->btnDelSession, &QPushButton::clicked, this, &ParticipantWidget::btnDeleteSession_clicked);
    connect(ui->btnAddDevice, &QPushButton::clicked, this, &ParticipantWidget::btnAddDevice_clicked);
    connect(ui->btnDelDevice, &QPushButton::clicked, this, &ParticipantWidget::btnDelDevice_clicked);
    connect(ui->btnDownloadAll, &QPushButton::clicked, this, &ParticipantWidget::btnDowloadAll_clicked);

    connect(ui->tableSessions, &QTableWidget::currentItemChanged, this, &ParticipantWidget::currentSelectedSessionChanged);
    connect(ui->tableSessions, &QTableWidget::itemDoubleClicked, this, &ParticipantWidget::displaySessionDetails);
    connect(ui->lstFilters, &QListWidget::itemChanged, this, &ParticipantWidget::currentTypeFiltersChanged);
    connect(ui->btnNextCal, &QPushButton::clicked, this, &ParticipantWidget::displayNextMonth);
    connect(ui->btnPrevCal, &QPushButton::clicked, this, &ParticipantWidget::displayPreviousMonth);
    connect(ui->calMonth1, &HistoryCalendarWidget::clicked, this, &ParticipantWidget::currentCalendarDateChanged);
    connect(ui->calMonth2, &HistoryCalendarWidget::clicked, this, &ParticipantWidget::currentCalendarDateChanged);
    connect(ui->calMonth3, &HistoryCalendarWidget::clicked, this, &ParticipantWidget::currentCalendarDateChanged);

    connect(ui->lstAvailDevices, &QListWidget::currentItemChanged, this, &ParticipantWidget::currentAvailDeviceChanged);
    connect(ui->lstDevices, &QListWidget::currentItemChanged, this, &ParticipantWidget::currentDeviceChanged);
}

void ParticipantWidget::updateControlsState()
{
    //ui->tabParticipantInfos->setEnabled(!m_data->isNew());
    /*ui->wdgParticipant->setEnabled(!isWaitingOrLoading() && !m_limited);

    // Buttons update
    ui->btnSave->setEnabled(!isWaitingOrLoading());
    ui->btnUndo->setEnabled(!isWaitingOrLoading());

    ui->frameButtons->setVisible(!m_limited);*/
}

void ParticipantWidget::updateFieldsValue()
{
    if (m_data){
        ui->lblTitle->setText(m_data->getName());
        ui->chkEnabled->setChecked(m_data->getFieldValue("participant_enabled").toBool());
        ui->chkLogin->setChecked(m_data->getFieldValue("participant_login_enabled").toBool());
        ui->chkWebAccess->setChecked(!m_data->getFieldValue("participant_token").toString().isEmpty());
        // TODO: Real URL
        ui->txtWeb->setText(m_data->getFieldValue("participant_token").toString());
        ui->txtUsername->setText(m_data->getFieldValue("participant_username").toString());
        // Must "trigger" the slots for username - password, since they are not set otherwise
        on_txtUsername_textEdited(ui->txtUsername->text());
        on_txtPassword_textEdited("");

        ui->wdgParticipant->fillFormFromData(m_data->toJson());
    }
}

void ParticipantWidget::initUI()
{
    ui->btnDownloadAll->hide();
    ui->frameLogin->hide();
    ui->frameActive->hide();
    ui->frameWeb->hide();

    // TODO: Implement template email feature
    ui->btnEmailWeb->hide();

    // Hide some fields in the detailled participant widget
    QStringList ignore_fields = {"participant_enabled", "participant_token", "participant_login_enabled",
                                "participant_username", "participant_password"};
    ui->wdgParticipant->hideFields(ignore_fields);
}

bool ParticipantWidget::validateData()
{
    bool valid = false;

    valid = ui->wdgParticipant->validateFormData();

    return valid;
}

void ParticipantWidget::updateSession(TeraData *session)
{
    int id_session = session->getId();

    QTableWidgetItem* name_item;
    QTableWidgetItem* date_item;
    QTableWidgetItem* type_item;
    QTableWidgetItem* duration_item;
    QTableWidgetItem* user_item;
    QTableWidgetItem* status_item;
    QToolButton* btnDownload = nullptr;

    if (m_listSessions_items.contains(id_session)){
        // Already there, get items
       name_item = m_listSessions_items[id_session];
       date_item = ui->tableSessions->item(name_item->row(), 1);
       type_item = ui->tableSessions->item(name_item->row(), 2);
       status_item = ui->tableSessions->item(name_item->row(), 3);
       duration_item = ui->tableSessions->item(name_item->row(), 4);
       user_item = ui->tableSessions->item(name_item->row(), 5);
       if (ui->tableSessions->cellWidget(name_item->row(), 6))
           if(ui->tableSessions->cellWidget(name_item->row(), 6)->layout())
               if(ui->tableSessions->cellWidget(name_item->row(), 6)->layout()->itemAt(1))
                  btnDownload = dynamic_cast<QToolButton*>(ui->tableSessions->cellWidget(name_item->row(), 6)->layout()->itemAt(1)->widget());
       delete m_ids_sessions[id_session];
    }else{

        ui->tableSessions->setRowCount(ui->tableSessions->rowCount()+1);
        int current_row = ui->tableSessions->rowCount()-1;
        name_item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_SESSION)),"");
        ui->tableSessions->setItem(current_row, 0, name_item);
        date_item = new QTableWidgetItem("");
        ui->tableSessions->setItem(current_row, 1, date_item);
        type_item = new QTableWidgetItem("");
        ui->tableSessions->setItem(current_row, 2, type_item);
        status_item = new QTableWidgetItem("");
        ui->tableSessions->setItem(current_row, 3, status_item);
        duration_item = new QTableWidgetItem("");
        duration_item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->tableSessions->setItem(current_row, 4, duration_item);
        user_item = new QTableWidgetItem("");
        ui->tableSessions->setItem(current_row, 5, user_item);

        // Create action buttons
        QFrame* action_frame = new QFrame();
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        layout->setAlignment(Qt::AlignLeft);
        action_frame->setLayout(layout);

        // View session
        QToolButton* btnView = new QToolButton();
        btnView->setIcon(QIcon(":/icons/search.png"));
        btnView->setProperty("id_session", session->getId());
        btnView->setCursor(Qt::PointingHandCursor);
        btnView->setMaximumWidth(32);
        btnView->setToolTip(tr("Ouvrir"));
        connect(btnView, &QToolButton::clicked, this, &ParticipantWidget::btnViewSession_clicked);
        layout->addWidget(btnView);

        // Delete
        QToolButton* btnDelete = new QToolButton();
        btnDelete->setIcon(QIcon(":/icons/delete_old.png"));
        btnDelete->setProperty("id_session", session->getId());
        btnDelete->setCursor(Qt::PointingHandCursor);
        btnDelete->setMaximumWidth(32);
        btnDelete->setToolTip(tr("Supprimer"));
        connect(btnDelete, &QToolButton::clicked, this, &ParticipantWidget::btnDeleteSession_clicked);
        layout->addWidget(btnDelete);

        // Download data
        btnDownload = new QToolButton();
        btnDownload->setIcon(QIcon(":/icons/download.png"));
        btnDownload->setProperty("id_session", session->getId());
        btnDownload->setCursor(Qt::PointingHandCursor);
        btnDownload->setMaximumWidth(32);
        btnDownload->setToolTip(tr("Télécharger les données"));
        connect(btnDownload, &QToolButton::clicked, this, &ParticipantWidget::btnDownloadSession_clicked);
        layout->addWidget(btnDownload);

        ui->tableSessions->setCellWidget(current_row, 6, action_frame);

        m_listSessions_items[id_session] = name_item;
    }
    m_ids_sessions[id_session] = new TeraData(*session);

    // Update values
    name_item->setText(session->getName());
    QDateTime session_date = session->getFieldValue("session_start_datetime").toDateTime();
    date_item->setText(session_date.toString("dd-MM-yyyy hh:mm:ss"));
    int session_type = session->getFieldValue("id_session_type").toInt();
    if (m_ids_session_types.contains(session_type)){
        type_item->setText(m_ids_session_types[session_type]->getFieldValue("session_type_name").toString());
        type_item->setForeground(QColor(m_ids_session_types[session_type]->getFieldValue("session_type_color").toString()));
    }else{
        type_item->setText("Inconnu");
    }
    duration_item->setText(QTime(0,0).addSecs(session->getFieldValue("session_duration").toInt()).toString("hh:mm:ss"));
    TeraSessionStatus::SessionStatus session_status = static_cast<TeraSessionStatus::SessionStatus>(session->getFieldValue("session_status").toInt());
    status_item->setText(TeraSessionStatus::getStatusName(session_status));
    // Set color depending on status_item
    //status_item->setTextColor(QColor(TeraSessionStatus::getStatusColor(session_status)));
    status_item->setForeground(Qt::black);
    status_item->setBackground(QColor(TeraSessionStatus::getStatusColor(session_status)));
    //QColor back_color = TeraForm::getGradientColor(3, 18, 33, static_cast<int>(session_date.daysTo(QDateTime::currentDateTime())));
    //back_color.setAlphaF(0.5);
    //date_item->setBackgroundColor(back_color);

    // Session creator
    if (session->hasFieldName("session_creator_user"))
        user_item->setText(session->getFieldValue("session_creator_user").toString());
    else if(session->hasFieldName("session_creator_device"))
        user_item->setText(tr("Appareil: ") + session->getFieldValue("session_creator_device").toString());
    else if(session->hasFieldName("session_creator_participant"))
        user_item->setText(tr("Participant: ") + session->getFieldValue("session_creator_participant").toString());
    else if(session->hasFieldName("session_creator_service"))
        user_item->setText(tr("Service: ") + session->getFieldValue("session_creator_service").toString());
    else {
        user_item->setText(tr("Inconnu"));
    }

    // Download data
    if (btnDownload){
        btnDownload->setVisible(session->getFieldValue("session_has_device_data").toBool());
        if (session->getFieldValue("session_has_device_data").toBool())
            ui->btnDownloadAll->show();
    }

    ui->tableSessions->resizeColumnsToContents();
}

void ParticipantWidget::updateDeviceProject(TeraData *device_project)
{
    int id_device = device_project->getFieldValue("id_device").toInt();

    // Check if device is already assigned to this participant
    if (m_listDevices_items.contains(id_device)){
        // It is - return
        return;
    }

    QListWidgetItem* item;
    // Check if device already exists in available list
    if (m_listAvailDevices_items.contains(id_device)){
        item = m_listAvailDevices_items[id_device];
    }else{
        // Must create a new item
        item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_DEVICE)),"");
        ui->lstAvailDevices->addItem(item);
        m_listAvailDevices_items[id_device] = item;
    }

    // Update values
    item->setText(device_project->getFieldValue("device_name").toString());
    if (device_project->hasFieldName("device_available")){
        if (!device_project->getFieldValue("device_available").toBool())
            item->setIcon(QIcon("://icons/device_installed.png"));
        else
            item->setIcon(QIcon(TeraData::getIconFilenameForDataType(TERADATA_DEVICE)));
        item->setData(Qt::UserRole, device_project->getFieldValue("device_available").toBool());
    }
}

void ParticipantWidget::updateDeviceParticipant(TeraData *device_participant)
{
    int id_device = device_participant->getFieldValue("id_device").toInt();
    QListWidgetItem* item;

    // Check if device is in the available list
    if (m_listAvailDevices_items.contains(id_device)){
        // It is - remove it from the list
        ui->lstAvailDevices->removeItemWidget(m_listAvailDevices_items[id_device]);
        delete m_listAvailDevices_items[id_device];
        m_listAvailDevices_items.remove(id_device);
    }

    // Check if device already exists in participant list
    if (m_listDevices_items.contains(id_device)){
        item = m_listDevices_items[id_device];
    }else{
        // Must create a new item
        item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_DEVICE)),"");
        ui->lstDevices->addItem(item);
        m_listDevices_items[id_device] = item;
        item->setData(Qt::UserRole, device_participant->getId());
    }

    // Update values
    item->setText(device_participant->getFieldValue("device_name").toString());
}

void ParticipantWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type == WEB_FORMS_QUERY_PARTICIPANT){
        ui->wdgParticipant->buildUiFromStructure(data);
        return;
    }
}

void ParticipantWidget::processSessionsReply(QList<TeraData> sessions)
{
    for(TeraData session:sessions){
        QVariantList session_parts_list = session.getFieldValue("session_participants").toList();

        for(QVariant session_part:session_parts_list){
            QVariantMap part_info = session_part.toMap();

            // Is that session for the current participant?
            if (part_info["id_participant"].toInt() == m_data->getId()){
                // Add session in table or update it
                updateSession(&session);
            }else{
                // Session is not for us - ignore it.
                continue;
            }
        }
    }

    // Update calendar view
    currentTypeFiltersChanged(nullptr);
    updateCalendars(getMinimumSessionDate());
    ui->calMonth1->setData(m_ids_sessions.values());
    ui->calMonth2->setData(m_ids_sessions.values());
    ui->calMonth2->setData(m_ids_sessions.values());
}

void ParticipantWidget::processSessionTypesReply(QList<TeraData> session_types)
{
    ui->cmbSessionType->clear();

    for (TeraData st:session_types){
        if (!m_ids_session_types.contains(st.getId())){
            m_ids_session_types[st.getId()] = new TeraData(st);
            // Add to session list
            QListWidgetItem* s = new QListWidgetItem(st.getName());
            s->setData(Qt::UserRole,st.getId());
            s->setCheckState(Qt::Checked);
            s->setForeground(QColor(st.getFieldValue("session_type_color").toString()));
            s->setFont(QFont("Arial",10));
           /* QPixmap* pxmap = new QPixmap(8,16);
            pxmap->fill(Qt::transparent);
            QPainter* paint = new QPainter(pxmap);
            paint->setBrush(QColor(st.getFieldValue("session_type_color").toString()));
            paint->setPen(Qt::transparent);
            paint->drawRect(0,0,8,16);
            QIcon* icon = new QIcon(*pxmap);
            s->setIcon(*icon);*/
            ui->lstFilters->addItem(s);

            // New session ComboBox
            ui->cmbSessionType->addItem(st.getName(), st.getId());
        }else{
            *m_ids_session_types[st.getId()] = st;
        }
    }

    ui->calMonth1->setSessionTypes(m_ids_session_types.values());
    ui->calMonth2->setSessionTypes(m_ids_session_types.values());
    ui->calMonth3->setSessionTypes(m_ids_session_types.values());

    // Query sessions for that participant
    if (!m_data->isNew() && m_listSessions_items.isEmpty()){
        QUrlQuery query;
        query.addQueryItem(WEB_QUERY_ID_PARTICIPANT, QString::number(m_data->getId()));
        queryDataRequest(WEB_SESSIONINFO_PATH, query);
    }
}

void ParticipantWidget::processDeviceProjectsReply(QList<TeraData> device_projects)
{
    // Check if device is for us
    for(TeraData device_project:device_projects){
        if (device_project.getFieldValue("id_project").toInt() == m_data->getFieldValue("id_project").toInt()){
            // For us! Update device...
            updateDeviceProject(&device_project);
        }
    }
}

void ParticipantWidget::processDeviceParticipantsReply(QList<TeraData> device_participants)
{
    // Check if device is for us
    for(TeraData device_part:device_participants){
        if (device_part.getFieldValue("id_participant").toInt() == m_data->getId()){
            // For us! Update device...
            updateDeviceParticipant(&device_part);
        }
    }
}

void ParticipantWidget::deleteDataReply(QString path, int id)
{
    if (id==0)
        return;

    if (path == WEB_SESSIONINFO_PATH){
        // A session got deleted - check if it affects the current display
        if (m_listSessions_items.contains(id)){
            ui->tableSessions->removeRow(m_listSessions_items[id]->row());
            delete m_ids_sessions[id];
            m_ids_sessions.remove(id);
            m_listSessions_items.remove(id);

            // Update calendars
            ui->calMonth1->setData(m_ids_sessions.values());
            ui->calMonth2->setData(m_ids_sessions.values());
            ui->calMonth2->setData(m_ids_sessions.values());
        }
    }

    if (path == WEB_DEVICEPARTICIPANTINFO_PATH){
        // A participant device association was deleted
        for (QListWidgetItem* item: m_listDevices_items.values()){
            // Check for id_device_participant, which is stored in "data" of the item
            if (item->data(Qt::UserRole).toInt() == id){
                // We found it - remove it and request update
                m_listDevices_items.remove(m_listDevices_items.key(item));
                ui->lstDevices->removeItemWidget(item);
                delete item;
                // Request refresh of available devices
                QUrlQuery query;
                query.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getFieldValue("id_project").toInt()));
                queryDataRequest(WEB_DEVICEPROJECTINFO_PATH, query);
                break;
            }
        }
    }
}

void ParticipantWidget::onDownloadCompleted(DownloadedFile *file)
{
    if (!m_comManager->hasPendingDownloads()){
        setEnabled(true);
        setReady();
        if (!m_diag_editor){
            GlobalMessageBox msgbox;
            msgbox.showInfo(tr("Téléchargement"), tr("Téléchargement terminé: ") + file->getFullFilename());
        }
    }


}

void ParticipantWidget::btnDeleteSession_clicked()
{
    // Check if the sender is a QToolButton (from the action column)
    QToolButton* action_btn = dynamic_cast<QToolButton*>(sender());
    if (action_btn){
        // Select row according to the session id of that button
        int id_session = action_btn->property("id_session").toInt();
        QTableWidgetItem* session_item = m_listSessions_items[id_session];
        ui->tableSessions->selectRow(session_item->row());
    }

    if (ui->tableSessions->selectedItems().count()==0)
        return;

    GlobalMessageBox diag;
    QMessageBox::StandardButton answer = QMessageBox::No;
    if (ui->tableSessions->selectionModel()->selectedRows().count() == 1){
        QTableWidgetItem* base_item = ui->tableSessions->item(ui->tableSessions->currentRow(),0);
        answer = diag.showYesNo(tr("Suppression?"), tr("Êtes-vous sûrs de vouloir supprimer """) + base_item->text() + """?");
    }else{
        answer = diag.showYesNo(tr("Suppression?"), tr("Êtes-vous sûrs de vouloir supprimer toutes les séances sélectionnées?"));
    }

    if (answer == QMessageBox::Yes){
        // We must delete!
        foreach(QModelIndex selected_row, ui->tableSessions->selectionModel()->selectedRows()){
            QTableWidgetItem* base_item = ui->tableSessions->item(selected_row.row(),0); // Get item at index 0 which is linked to session id
            //m_comManager->doDelete(TeraData::getPathForDataType(TERADATA_SESSION), m_listSessions_items.key(base_item));
            m_comManager->doDelete(TeraData::getPathForDataType(TERADATA_SESSION), m_listSessions_items.key(base_item));
        }
    }
}

void ParticipantWidget::btnAddDevice_clicked()
{
    QListWidgetItem* item_toadd = ui->lstAvailDevices->currentItem();

    if (!item_toadd)
        return;

    int id_device = m_listAvailDevices_items.key(item_toadd);
    if (!item_toadd->data(Qt::UserRole).toBool()){
        // Item is already assigned to at least 1 participant - show dialog.
        DeviceAssignDialog* diag = new DeviceAssignDialog(m_comManager, id_device, this);
        diag->exec();
        if (diag->result() == DeviceAssignDialog::DEVICEASSIGN_CANCEL){
            return;
        }

        if (diag->result() == DeviceAssignDialog::DEVICEASSIGN_DEASSIGN){
            // Delete all associated participants
            for (int id:diag->getDeviceParticipantsIds()){
                deleteDataRequest(WEB_DEVICEPARTICIPANTINFO_PATH, id);
            }
        }
    }

    // Add device to participant
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray devices;

    QJsonObject item_obj;
    item_obj.insert("id_device", id_device);
    item_obj.insert("id_participant", m_data->getId());

    // Update query
    base_obj.insert("device_participant", item_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_DEVICEPARTICIPANTINFO_PATH, document.toJson());
}

void ParticipantWidget::btnDelDevice_clicked()
{
    QListWidgetItem* item_todel = ui->lstDevices->currentItem();

    if (!item_todel)
        return;

    //int id_device = m_listDevices_items.key(item_todel);

    GlobalMessageBox diag;
    QMessageBox::StandardButton answer = diag.showYesNo(tr("Déassignation?"),
                                                        tr("Êtes-vous sûrs de vouloir désassigner """) + item_todel->text() + """?");
    if (answer == QMessageBox::Yes){
        // We must delete!
        m_comManager->doDelete(WEB_DEVICEPARTICIPANTINFO_PATH, item_todel->data(Qt::UserRole).toInt());
    }
}

void ParticipantWidget::btnDownloadSession_clicked()
{
    QToolButton* button = dynamic_cast<QToolButton*>(sender());
    if (button){
        // Query folder to save file
        QString save_path = QFileDialog::getExistingDirectory(this, tr("Sélectionnez un dossier pour le téléchargement"),
                                                              QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        if (!save_path.isEmpty()){
            int id_session = button->property("id_session").toInt();
            QUrlQuery args;
            args.addQueryItem(WEB_QUERY_DOWNLOAD, "");
            args.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(id_session));
            downloadDataRequest(save_path, WEB_DEVICEDATAINFO_PATH, args);
            setWaiting();
        }
    }
}

void ParticipantWidget::btnDowloadAll_clicked()
{
    QString save_path = QFileDialog::getExistingDirectory(this, tr("Sélectionnez un dossier pour le téléchargement"),
                                                          QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    if (!save_path.isEmpty()){
        int id_participant = m_data->getId();
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_DOWNLOAD, "");
        args.addQueryItem(WEB_QUERY_ID_PARTICIPANT, QString::number(id_participant));
        downloadDataRequest(save_path, WEB_DEVICEDATAINFO_PATH, args);
        setWaiting();
    }
}

void ParticipantWidget::btnViewSession_clicked()
{
    // Check if the sender is a QToolButton (from the action column)
    QToolButton* action_btn = dynamic_cast<QToolButton*>(sender());
    QTableWidgetItem* session_item = nullptr;
    if (action_btn){
        // Select row according to the session id of that button
        int id_session = action_btn->property("id_session").toInt();
        session_item = m_listSessions_items[id_session];
    }

    if (session_item){
        displaySessionDetails(session_item);
    }


}

void ParticipantWidget::currentSelectedSessionChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    Q_UNUSED(previous)
    ui->btnDelSession->setEnabled(current);
}

void ParticipantWidget::currentCalendarDateChanged(QDate current_date)
{
    // Clear current selection
    ui->tableSessions->clearSelection();

    // Temporarly set multi-selection
    QAbstractItemView::SelectionMode current_mode = ui->tableSessions->selectionMode();
    ui->tableSessions->setSelectionMode(QAbstractItemView::MultiSelection);

    // Select all the sessions in the list that fits with that date
    QTableWidgetItem* first_item = nullptr;
    foreach(TeraData* session, m_ids_sessions.values()){
        if (session->getFieldValue("session_start_datetime").toDateTime().date() == current_date){
            QTableWidgetItem* session_item = m_listSessions_items.value(session->getId());
            if (session_item){
                ui->tableSessions->selectRow(session_item->row());
                if (!first_item)
                    first_item = session_item;
            }
        }
    }

    // Resets selection mode
    ui->tableSessions->setSelectionMode(current_mode);

    // Ensure first session is visible
    if (first_item){
        ui->tableSessions->scrollToItem(first_item);
    }
}

void ParticipantWidget::displaySessionDetails(QTableWidgetItem *session_item)
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }
    m_diag_editor = new BaseDialog(this);

    int id_session = m_listSessions_items.key(ui->tableSessions->item(session_item->row(),0));
    TeraData* ses_data = m_ids_sessions[id_session];
    if (ses_data){
        SessionWidget* ses_widget = new SessionWidget(m_comManager, ses_data, nullptr);
        m_diag_editor->setCentralWidget(ses_widget);

        m_diag_editor->setWindowTitle(tr("Séance"));
        m_diag_editor->setMinimumSize(this->width(), this->height());

        connect(ses_widget, &SessionWidget::closeRequest, m_diag_editor, &QDialog::accept);

        m_diag_editor->open();
    }
}

void ParticipantWidget::currentTypeFiltersChanged(QListWidgetItem *changed)
{
    QList<int> ids;

    for (int i=0; i<ui->lstFilters->count(); i++){
        if (ui->lstFilters->item(i)->checkState() == Qt::Checked){
            ids.append(ui->lstFilters->item(i)->data(Qt::UserRole).toInt());
        }
    }

    ui->calMonth1->setFilters(ids);
    ui->calMonth2->setFilters(ids);
    ui->calMonth3->setFilters(ids);

    if (!changed)
        return;
    // Update session tables
    QString current_ses_type = changed->text();
    for (int i=0; i<ui->tableSessions->rowCount(); i++){
        if (ui->tableSessions->item(i,2)->text() == current_ses_type){
            ui->tableSessions->setRowHidden(i, changed->checkState()==Qt::Unchecked);
        }
    }
}

void ParticipantWidget::updateCalendars(QDate left_date){
    ui->calMonth1->setCurrentPage(left_date.year(),left_date.month());
    ui->lblMonth1->setText(QLocale::system().monthName(left_date.month()) + " " + QString::number(left_date.year()));

    left_date = left_date.addMonths(1);
    ui->calMonth2->setCurrentPage(left_date.year(),left_date.month());
    ui->lblMonth2->setText(QLocale::system().monthName(left_date.month()) + " " + QString::number(left_date.year()));

    left_date = left_date.addMonths(1);
    ui->calMonth3->setCurrentPage(left_date.year(),left_date.month());
    ui->lblMonth3->setText(QLocale::system().monthName(left_date.month()) + " " + QString::number(left_date.year()));

    // Check if we must enable the previous month button
    QDate min_date = getMinimumSessionDate();

    if (ui->calMonth1->yearShown()==min_date.year() && ui->calMonth1->monthShown()==min_date.month())
        ui->btnPrevCal->setEnabled(false);
    else
        ui->btnPrevCal->setEnabled(true);

}

QDate ParticipantWidget::getMinimumSessionDate()
{
    QDate min_date = QDate::currentDate();
    for (TeraData* session:m_ids_sessions.values()){
        QDate session_date = session->getFieldValue("session_start_datetime").toDateTime().date();
        if (session_date < min_date)
            min_date = session_date;
    }

    return min_date;
}

void ParticipantWidget::displayNextMonth(){
    QDate new_date;
    new_date.setDate(ui->calMonth1->yearShown(), ui->calMonth1->monthShown(), 1);
    new_date = new_date.addMonths(1);

    updateCalendars(new_date);
}

void ParticipantWidget::displayPreviousMonth(){
    QDate new_date;
    new_date.setDate(ui->calMonth1->yearShown(), ui->calMonth1->monthShown(), 1);
    new_date = new_date.addMonths(-1);

    updateCalendars(new_date);
}

void ParticipantWidget::sessionLobbyStartSessionRequested()
{
    int id_session_type = ui->cmbSessionType->currentData().toInt();
    // Start session
    // TODO: Get real participant list from SessionLobbyDialog!
    m_comManager->startSession(*m_ids_session_types[id_session_type], QStringList(m_data->getFieldValue("participant_uuid").toString()), QStringList());

    m_sessionLobby->deleteLater();
    m_sessionLobby = nullptr;
}

void ParticipantWidget::sessionLobbyStartSessionCancelled()
{
    if (m_sessionLobby){
        m_sessionLobby->deleteLater();
        m_sessionLobby = nullptr;
    }
}

void ParticipantWidget::currentAvailDeviceChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)
    ui->btnAddDevice->setEnabled(current);
}

void ParticipantWidget::currentDeviceChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)
    ui->btnDelDevice->setEnabled(current);
}


void ParticipantWidget::on_chkEnabled_stateChanged(int checkState)
{
    // Update active state
    bool current_state = (checkState == Qt::Checked);
    if (m_data){
        if (m_data->getFieldValue("participant_enabled").toBool() != current_state){
            ui->wdgParticipant->setFieldValue("participant_enabled", current_state);
            saveData();
        }
    }

    // Update UI
    ui->frameActive->setVisible(current_state);

}

void ParticipantWidget::on_chkWebAccess_stateChanged(int checkState)
{
    ui->frameWeb->setVisible(checkState == Qt::Checked);
}

void ParticipantWidget::on_chkLogin_stateChanged(int checkState)
{
    bool current_state = (checkState == Qt::Checked);
    if (m_data){
        if (m_data->getFieldValue("participant_login_enabled").toBool() != current_state){
            ui->wdgParticipant->setFieldValue("participant_login_enabled", current_state);
            saveData();
        }
    }

    ui->frameLogin->setVisible(checkState == Qt::Checked);
}

void ParticipantWidget::on_btnCopyWeb_clicked()
{
    QClipboard* clipboard = QApplication::clipboard();

    clipboard->setText(ui->txtWeb->text(), QClipboard::Clipboard);

    if (clipboard->supportsSelection()) {
        clipboard->setText(ui->txtWeb->text(), QClipboard::Selection);
    }

    #if defined(Q_OS_LINUX)
        QThread::msleep(1); //workaround for copied text not being available...
    #endif
}

void ParticipantWidget::on_btnRandomPass_clicked()
{
    // Generate random password
    QString password = Utils::generatePassword(10);


    GlobalMessageBox msg(this);
    msg.showInfo(tr("Mot de passe généré"), password);

    ui->txtPassword->setText(password);
    ui->txtPasswordConfirm->setText(password);



}

void ParticipantWidget::on_btnSaveLogin_clicked()
{
    // Validate required fields
    bool all_ok = true;
    QString err_msg;

    if (ui->txtUsername->text().isEmpty()){
        all_ok = false;
        err_msg.append(tr("Code utilisateur manquant<br/>"));
    }

    if (!ui->txtPassword->text().isEmpty() || !ui->txtPasswordConfirm->text().isEmpty()){
        if (ui->txtPassword->text() != ui->txtPasswordConfirm->text()){
            all_ok = false;
            err_msg.append(tr("Les mots de passe ne correspondent pas."));
        }
    }

    if (!all_ok){
        GlobalMessageBox msg;
        msg.showError(tr("Informations manquantes"), tr("Les informations suivantes sont incorrectes:") + "<br/><br/>" + err_msg);
        return;
    }

    // Update username and/or password
    ui->wdgParticipant->setFieldValue("participant_username", ui->txtUsername->text());
    if (!ui->txtPassword->text().isEmpty())
        ui->wdgParticipant->setFieldValue("participant_password", ui->txtPassword->text());
    saveData();
    ui->wdgParticipant->resetFormValues(); // Ensure data is always sent when using this button
    ui->txtPassword->clear();
    ui->txtPasswordConfirm->clear();

}

void ParticipantWidget::on_txtUsername_textEdited(const QString &current)
{
    if (current.isEmpty()){
        ui->txtUsername->setStyleSheet("background-color: #ffaaaa;");
    }else{
        ui->txtUsername->setStyleSheet("");
    }
}

void ParticipantWidget::on_txtPassword_textEdited(const QString &current)
{
    QString confirm_pass = ui->txtPasswordConfirm->text();
    if (current != confirm_pass || ui->txtUsername->text().isEmpty()){
        ui->txtPassword->setStyleSheet("background-color: #ffaaaa;");
        ui->txtPasswordConfirm->setStyleSheet("background-color: #ffaaaa;");
    }else{
        ui->txtPassword->setStyleSheet("");
        ui->txtPasswordConfirm->setStyleSheet("");
    }
}

void ParticipantWidget::on_txtPasswordConfirm_textEdited(const QString &current)
{
    QString pass = ui->txtPassword->text();
    if (current != pass || ui->txtUsername->text().isEmpty()){
        ui->txtPasswordConfirm->setStyleSheet("background-color: #ffaaaa;");
        ui->txtPassword->setStyleSheet("background-color: #ffaaaa;");
    }else{
        ui->txtPasswordConfirm->setStyleSheet("");
        ui->txtPassword->setStyleSheet("");
    }
}

void ParticipantWidget::on_tabInfos_currentChanged(int index)
{
    QUrlQuery query;

    QWidget* current_tab = ui->tabInfos->widget(index);

    if (current_tab == ui->tabDevices){ // Devices
        if (m_listDevices_items.isEmpty()){
            query.addQueryItem(WEB_QUERY_ID_PARTICIPANT, QString::number(m_data->getId()));
            queryDataRequest(WEB_DEVICEPARTICIPANTINFO_PATH, query);

            // Query devices for the current site
            query.removeQueryItem(WEB_QUERY_ID_PARTICIPANT);
            query.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getFieldValue("id_project").toInt()));
            queryDataRequest(WEB_DEVICEPROJECTINFO_PATH, query);
        }
    }

    if (current_tab == ui->tabConfig){ // Configuration
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

void ParticipantWidget::on_btnNewSession_clicked()
{
    // Build JSON structure to request session start

    /*QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray participants;

    QJsonObject item_obj;
    item_obj.insert("id_service", 3); // TODO Get correct ID
    item_obj.insert("action", "start");
    item_obj.insert("parameters", "{start_muted: false}"); // TODO: Get parameters from session type

    participants.append(QJsonValue(m_data->getFieldValue("participant_uuid").toString()));
    item_obj.insert("session_participants", participants);

    // Update query
    base_obj.insert("session_manage", item_obj);
    document.setObject(base_obj);
    postDataRequest(WEB_SESSIONMANAGER_PATH, document.toJson());*/

    // TODO: UI to select multiple participants if the session is setted to "multi"

    if (ui->cmbSessionType->currentIndex() < 0)
        return;

    if (!m_data)
        return;

    int id_session_type = ui->cmbSessionType->currentData().toInt();

    if (m_sessionLobby)
        m_sessionLobby->deleteLater();
    m_sessionLobby = new SessionLobbyDialog(m_comManager, *m_ids_session_types[id_session_type], m_data->getFieldValue("id_project").toInt(), this);

    // Add current participant to session
    m_sessionLobby->addParticipantsToSession(QList<TeraData>() << *m_data, QList<int>() << m_data->getId());

    // Add current user to session
    m_sessionLobby->addUsersToSession(QList<TeraData>() << m_comManager->getCurrentUser(), QList<int>() <<m_comManager->getCurrentUser().getId());

    connect(m_sessionLobby, &QDialog::accepted, this, &ParticipantWidget::sessionLobbyStartSessionRequested);
    connect(m_sessionLobby, &QDialog::rejected, this, &ParticipantWidget::sessionLobbyStartSessionCancelled);

    // Show Session Lobby
    m_sessionLobby->exec();
}

void ParticipantWidget::on_btnCheckSesstionTypes_clicked()
{
    for (int i=0; i<ui->lstFilters->count(); i++){
        ui->lstFilters->item(i)->setCheckState(Qt::Checked);
    }
}

void ParticipantWidget::on_btnUnchekSessionTypes_clicked()
{
    for (int i=0; i<ui->lstFilters->count(); i++){
        ui->lstFilters->item(i)->setCheckState(Qt::Unchecked);
    }
}
