#include "SessionWidget.h"
#include "ui_SessionWidget.h"

#include "GlobalMessageBox.h"

#include "TeraSessionStatus.h"
#include "TeraSessionEvent.h"

#include <QFileDialog>
#include <QStandardPaths>

SessionWidget::SessionWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, nullptr, parent),
    ui(new Ui::SessionWidget)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image
    m_alwaysShowAssets = false;

    // Init UI
    ui->wdgSession->setHighlightConditions(false); // Hide conditional questions indicator
    ui->wdgSession->setComManager(m_comManager);
    ui->tabNav->setCurrentIndex(0);

    ui->wdgInvitations->setComManager(m_comManager);
    ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabInvitations), false);

    ui->wdgSessionInvitees->setComManager(m_comManager);
    ui->wdgSessionInvitees->showOnlineFilter(false);

    ui->tabAssets->setComManager(m_comManager);
    ui->tabTests->setComManager(m_comManager);

    // Use base class to manage editing
    setEditorControls(ui->wdgSession, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    // Query form definition
    m_idProject = -1;
    QUrlQuery args(WEB_FORMS_QUERY_SESSION);
    if (!data->isNew()){
        args.addQueryItem(WEB_QUERY_ID, QString::number(data->getId()));
    }else{
        if (data->hasFieldName("id_project")){
            args.addQueryItem(WEB_QUERY_ID_PROJECT, data->getFieldValue("id_project").toString());
        }
    }

     if (data->hasFieldName("id_project")){
         m_idProject = data->getFieldValue("id_project").toInt();
         ui->wdgInvitations->setCurrentProject(m_idProject);
         ui->tabAssets->setAssociatedProject(m_idProject);
     }

    if (data->hasFieldName("participant_uuid")){
        m_baseParticipantUuid = data->getFieldValue("participant_uuid").toString();
    }

    SessionWidget::setData(data);
    m_data->removeFieldName("id_project"); // Remove field if present, as not useful otherwise and will cause issues when saving data
    m_data->removeFieldName("participant_uuid");

    setLimited(false);

    queryDataRequest(WEB_FORMS_PATH, args);

}

SessionWidget::~SessionWidget()
{
    if (ui)
        delete ui;
}

void SessionWidget::saveData(bool signal){

    // If data is new, we request all the fields.
    QJsonDocument group_data = ui->wdgSession->getFormDataJson(m_data->isNew());

    // Filter empty creator fields
    QJsonObject session_data = group_data["session"].toObject();
    QJsonObject base_obj;

    if (session_data["id_creator_device"].toInt() == 0){
        session_data.remove("id_creator_device");
    }
    if (session_data["id_creator_participant"].toInt() == 0){
        session_data.remove("id_creator_participant");
    }
    if (session_data["id_creator_service"].toInt() == 0){
        session_data.remove("id_creator_service");
    }
    if (session_data["id_creator_user"].toInt() == 0){
        session_data.remove("id_creator_user");
    }
    session_data.remove("session_creator_device");
    session_data.remove("session_creator_participant");
    session_data.remove("session_creator_user");
    session_data.remove("session_creator_service");
    session_data.remove("session_has_device_data");

    if (ui->wdgSessionInvitees->isEditable()){
        // Append session invitees to the post request
        if (ui->wdgSessionInvitees->getDevicesInSessionCount() > 0){
            QJsonArray data_list;
            QList<int> ids = ui->wdgSessionInvitees->getDevicesIdsInSession();
            for(const int &data_id:std::as_const(ids)){
                data_list.append(QJsonValue(data_id));
            }
            session_data.insert("session_devices_ids", data_list);
        }

        if (ui->wdgSessionInvitees->getParticipantsInSessionCount() > 0){
            QJsonArray data_list;
            QList<int> ids = ui->wdgSessionInvitees->getParticipantsIdsInSession();
            for(const int &data_id:std::as_const(ids)){
                data_list.append(QJsonValue(data_id));
            }
            session_data.insert("session_participants_ids", data_list);
        }

        if (ui->wdgSessionInvitees->getUsersInSessionCount() > 0){
            QJsonArray data_list;
            QList<int> ids = ui->wdgSessionInvitees->getUsersIdsInSession();
            for(const int &data_id:std::as_const(ids)){
                data_list.append(QJsonValue(data_id));
            }
            session_data.insert("session_users_ids", data_list);
        }
    }

    base_obj.insert("session", session_data);
    group_data.setObject(base_obj);

    postDataRequest(WEB_SESSIONINFO_PATH, group_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgSession->getFormDataObject(TERADATA_SESSION);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void SessionWidget::setData(const TeraData *data)
{
    DataEditorWidget::setData(data);

    if (!dataIsNew()){
        // Participants
        updateSessionParticipants();
        updateSessionUsers();
        updateSessionDevices();

        // Query stats
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(m_data->getId()));
        queryDataRequest(WEB_STATS_PATH, args);

    }
}

void SessionWidget::alwaysShowAssets(const bool &allow)
{
    m_alwaysShowAssets = allow;

}

void SessionWidget::showAssets()
{
    // Display assets tab by default
    ui->tabNav->setCurrentWidget(ui->tabAssets);
}

void SessionWidget::showTests()
{
    // Display tests tab by default
    ui->tabNav->setCurrentWidget(ui->tabTests);
}

void SessionWidget::setTestTypes(QList<TeraData> *test_types)
{
    m_testTypes = test_types;
    bool show = m_testTypes != nullptr;
    if (show && m_testTypes){
        show = !m_testTypes->isEmpty();
    }
    ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabInvitations), show);
    if (show){
        ui->wdgInvitations->setCurrentTestTypes(*m_testTypes);
    }
}

void SessionWidget::enableEmails(const bool &enable)
{
    m_allowEmails = enable;
    ui->wdgInvitations->setEnableEmail(m_allowEmails);
}

void SessionWidget::updateControlsState()
{
    ui->grpSummary->setVisible(!dataIsNew());
    if (dataIsNew()){
        if (ui->tabNav->count()>1){
            // Move invitee list to first tab
            ui->tabInvitees->layout()->removeWidget(ui->wdgSessionInvitees);
            ui->tabDashboard->layout()->removeWidget(ui->frameButtons);

            /*QLabel* lblInvitees = new QLabel(tr("Invités"));
            QFont labelFont;
            labelFont.setBold(true);
            labelFont.setPointSize(10);
            lblInvitees->setFont(labelFont);

            ui->tabDashboard->layout()->addWidget(lblInvitees);*/
            ui->layoutEditor->addWidget(ui->wdgSessionInvitees);
            ui->tabDashboard->layout()->addWidget(ui->frameButtons);

            while (ui->tabNav->count() > 1)
                ui->tabNav->removeTab(1);

            queryAvailableInvitees();
        }
        ui->wdgSession->hideField("session_duration");
        ui->wdgSessionInvitees->setEditable(true);
        //ui->wdgSessionInvitees->showAvailableInvitees(true, true);
        ui->wdgSessionInvitees->autoSelectFilters();
        ui->lblStatus->hide();

    }else{
        TeraSessionStatus::SessionStatus status = static_cast<TeraSessionStatus::SessionStatus>(m_data->getFieldValue("session_status").toInt());
        if (status == TeraSessionStatus::STATUS_NOTSTARTED){
            ui->btnEditInvitees->hide();
            ui->wdgSessionInvitees->setEditable(true);
        }else{
            ui->wdgSessionInvitees->setEditable(ui->btnEditInvitees->isChecked());
        }
    }

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
    // Check if we have a least one invitee in the session
    if (ui->wdgSessionInvitees->getInviteesCount() == 0){
        GlobalMessageBox msgbox;
        msgbox.showError(tr("Invités manquants"), tr("Au moins un invité doit être dans la séance"));
        return false;
    }

    bool valid = false;

    valid = ui->wdgSession->validateFormData();

    return valid;
}

void SessionWidget::updateSessionParticipants()
{
    if (!m_data)
        return;


    if (m_data->hasFieldName("session_participants")){
        QVariantList session_parts_list = m_data->getFieldValue("session_participants").toList();
        QList<TeraData> participants;
        for(const QVariant &session_part:std::as_const(session_parts_list)){
            QVariantMap part_info = session_part.toMap();
            TeraData part(TERADATA_PARTICIPANT);
            part.fromMap(part_info);
            participants.append(part);
            /*
            int id_participant = part_info["id_participant"].toInt();
            QString participant_name = part_info["participant_name"].toString();
            QListWidgetItem* item = nullptr;

            for(int i=0; i<ui->lstParticipants->count(); i++){
                int part_id = ui->lstParticipants->item(i)->data(Qt::UserRole).toInt();
                if (part_id == id_participant){
                    // Participant already present
                    item = ui->lstParticipants->item(i);
                    break;
                }
            }
            // New participant
            if (!item){
                item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_PARTICIPANT)), participant_name);
                item->setData(Qt::UserRole, id_participant);
                ui->lstParticipants->addItem(item);
            }

            // Update participant name
            item->setText(participant_name);*/


        }
        ui->wdgSessionInvitees->addParticipantsToSession(participants);
    }
}

void SessionWidget::updateSessionUsers()
{
    if (!m_data)
        return;

    if (m_data->hasFieldName("session_users")){
        QVariantList session_users_list = m_data->getFieldValue("session_users").toList();

        QList<TeraData> users;
        for(const QVariant &session_user:std::as_const(session_users_list)){
            QVariantMap user_info = session_user.toMap();
            TeraData user(TERADATA_USER);
            user.fromMap(user_info);
            users.append(user);
            /*int id_user = user_info["id_user"].toInt();
            QString user_name = user_info["user_name"].toString();
            QListWidgetItem* item = nullptr;
            for(int i=0; i<ui->lstUsers->count(); i++){
                int user_id = ui->lstUsers->item(i)->data(Qt::UserRole).toInt();
                if (user_id == id_user){
                    // User already present
                    item = ui->lstUsers->item(i);
                    break;
                }
            }
            // New user
            if (!item){
                item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_USER)), user_name);
                item->setData(Qt::UserRole, id_user);
                ui->lstUsers->addItem(item);
            }

            // Update name
            item->setText(user_name);*/
        }
        ui->wdgSessionInvitees->addUsersToSession(users);
    }
}

void SessionWidget::updateSessionDevices()
{
    if (!m_data)
        return;

    if (m_data->hasFieldName("session_devices")){
        QVariantList session_devices_list = m_data->getFieldValue("session_devices").toList();

        QList<TeraData> devices;
        for(const QVariant &session_device:std::as_const(session_devices_list)){
            QVariantMap device_info = session_device.toMap();
            TeraData device(TERADATA_DEVICE);
            device.fromMap(device_info);
            devices.append(device);
            /*int id_device = device_info["id_device"].toInt();
            QString device_name = device_info["device_name"].toString();
            QListWidgetItem* item = nullptr;
            for(int i=0; i<ui->lstDevices->count(); i++){
                int device_id = ui->lstDevices->item(i)->data(Qt::UserRole).toInt();
                if (device_id == id_device){
                    // User already present
                    item = ui->lstDevices->item(i);
                    break;
                }
            }
            // New device
            if (!item){
                item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_DEVICE)), device_name);
                item->setData(Qt::UserRole, id_device);
                ui->lstDevices->addItem(item);
            }

            // Update name
            item->setText(device_name);*/
        }
        ui->wdgSessionInvitees->addDevicesToSession(devices);
    }
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

void SessionWidget::queryAvailableInvitees()
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_LIST, "1");
    args.addQueryItem(WEB_QUERY_ENABLED, "1");
    if (m_idProject >= 0){
        args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_idProject));
    }

    // Query participants
    queryDataRequest(WEB_PARTICIPANTINFO_PATH, args);

    // Query users
    queryDataRequest(WEB_USERINFO_PATH, args);

    // Query devices
    queryDataRequest(WEB_DEVICEINFO_PATH, args);

}

void SessionWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type.startsWith(WEB_FORMS_QUERY_SESSION)){
        ui->wdgSession->buildUiFromStructure(data);
        return;
    }
}
/*
void SessionWidget::processDeviceDatasReply(QList<TeraData> device_datas)
{
    for (TeraData device_data:device_datas){
        if (device_data.getFieldValue("id_session").toInt() == m_data->getId()){
            // Device data is for us
            updateDeviceData(&device_data);
        }
    }
}*/

void SessionWidget::processSessionEventsReply(QList<TeraData> events)
{
    for (TeraData event:events){
        if (event.getFieldValue("id_session").toInt() == m_data->getId()){
            // Event is for us
            updateEvent(&event);
        }
    }
}

void SessionWidget::processStatsReply(TeraData stats, QUrlQuery reply_query)
{
    // Check if stats are for us
    if (!reply_query.hasQueryItem("id_session"))
        return;

    if (reply_query.queryItemValue("id_session").toInt() != m_data->getId())
        return;

    // Fill stats information
    ui->lblUsersStats->setText(stats.getFieldValue("users_total_count").toString() + tr(" Utilisateurs"));
    ui->lblParticipantsStats->setText(stats.getFieldValue("participants_total_count").toString() + tr(" Participants"));
    ui->lblDevicesStats->setText(stats.getFieldValue("devices_total_count").toString() + tr(" Appareils"));
    ui->lblAssets->setText(stats.getFieldValue("assets_total_count").toString() + tr(" Données"));
    ui->lblEvents->setText(stats.getFieldValue("events_total_count").toString() + tr(" Événements"));
    ui->lblTests->setText(stats.getFieldValue("tests_total_count").toString() + tr(" Évaluations"));

    // Hide unused sections
    for (int i=ui->tabNav->count()-1; i>=0; i--){
        if (ui->tabNav->widget(i) == ui->tabAssets && stats.getFieldValue("assets_total_count").toInt() == 0 && !m_alwaysShowAssets){
            ui->tabNav->removeTab(i);
        }
        if (ui->tabNav->widget(i) == ui->tabEvents && stats.getFieldValue("events_total_count").toInt() == 0){
            ui->tabNav->removeTab(i);
        }
        if (ui->tabNav->widget(i) == ui->tabTests && stats.getFieldValue("tests_total_count").toInt() == 0){
            ui->tabNav->removeTab(i);
        }
    }

}

void SessionWidget::processParticipantsReply(QList<TeraData> participants)
{
    if (!ui->wdgSessionInvitees->hasAvailableParticipants()){
        ui->wdgSessionInvitees->setAvailableParticipants(participants);
        if (dataIsNew()){
            // Add current participant to session
            ui->wdgSessionInvitees->addParticipantToSession(m_baseParticipantUuid);
            ui->wdgSessionInvitees->addRequiredParticipant(m_baseParticipantUuid);
        }else{
            ui->wdgSessionInvitees->autoSelectFilters();
        }
    }
}

void SessionWidget::processDevicesReply(QList<TeraData> devices)
{
    if (!ui->wdgSessionInvitees->hasAvailableDevices()){
        ui->wdgSessionInvitees->setAvailableDevices(devices);
        ui->wdgSessionInvitees->autoSelectFilters();
    }
}

void SessionWidget::processUsersReply(QList<TeraData> users)
{
    if (!ui->wdgSessionInvitees->hasAvailableUsers()){
        ui->wdgSessionInvitees->setAvailableUsers(users);
        if (dataIsNew()){
            // Add current user to session
            ui->wdgSessionInvitees->addUserToSession(m_comManager->getCurrentUser().getUuid());
        }else{
            ui->wdgSessionInvitees->autoSelectFilters();
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
   /* if (path == WEB_DEVICEDATAINFO_PATH){
        // Remove data from list
        if (m_listDeviceDatas.contains(id)){
            ui->tableData->removeRow(m_listDeviceDatas[id]->row());
            m_listDeviceDatas.remove(id);
            qDebug() << ui->tableData->rowCount();
        }
    }*/
}

void SessionWidget::onAssetsCountChanged(int new_count)
{
    ui->lblAssets->setText(QString::number(new_count) + tr(" Données"));

    if (!dataIsNew())
        emit assetsCountChanged(m_data->getId(), new_count);
}

void SessionWidget::onTestsCountChanged(int new_count)
{
    ui->lblTests->setText(QString::number(new_count) + tr(" Évaluations"));

    if (!dataIsNew())
        emit testsCountChanged(m_data->getId(), new_count);
}

void SessionWidget::sessionInviteesChanged(QStringList user_uuids, QStringList participant_uuids, QStringList device_uuids)
{
    if (ui->wdgSessionInvitees->getUsersInSessionCount() == 0
        && ui->wdgSessionInvitees->getParticipantsInSessionCount() == 0
        && ui->wdgSessionInvitees->getDevicesInSessionCount() == 0){
        GlobalMessageBox msg;
        msg.showError(tr("Séance vide"), tr("Une séance ne peut avoir aucun invité. Veuillez ajouter des invités à celle-ci."));
        return;
    }
    // Post session update with new lists
    QJsonDocument document;
    QJsonObject base_obj;

    QJsonObject item_obj;
    item_obj.insert("id_session", m_data->getId());


    QJsonArray data_list;
    if (ui->wdgSessionInvitees->getDevicesInSessionCount() > 0){

        QList<int> ids = ui->wdgSessionInvitees->getDevicesIdsInSession();
        for(const int &data_id:std::as_const(ids)){
            data_list.append(QJsonValue(data_id));
        }
    }
    item_obj.insert("session_devices_ids", data_list);

    while(data_list.count()) {
         data_list.pop_back();
    }

    if (ui->wdgSessionInvitees->getParticipantsInSessionCount() > 0){

        QList<int> ids = ui->wdgSessionInvitees->getParticipantsIdsInSession();
        for(const int &data_id:std::as_const(ids)){
            data_list.append(QJsonValue(data_id));
        }
    }
    item_obj.insert("session_participants_ids", data_list);

    while(data_list.count()) {
         data_list.pop_back();
    }
    if (ui->wdgSessionInvitees->getUsersInSessionCount() > 0){
        QList<int> ids = ui->wdgSessionInvitees->getUsersIdsInSession();
        for(const int &data_id:std::as_const(ids)){
            data_list.append(QJsonValue(data_id));
        }
    }
    item_obj.insert("session_users_ids", data_list);

    base_obj.insert("session", item_obj);
    document.setObject(base_obj);

    if (!dataIsNew())
        postDataRequest(WEB_SESSIONINFO_PATH, document.toJson());

}

void SessionWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &SessionWidget::processFormsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &SessionWidget::postResultReply);
    //connect(m_comManager, &ComManager::deviceDatasReceived, this, &SessionWidget::processDeviceDatasReply);
    connect(m_comManager, &ComManager::sessionEventsReceived, this, &SessionWidget::processSessionEventsReply);
    connect(m_comManager, &ComManager::statsReceived, this, &SessionWidget::processStatsReply);
    connect(m_comManager, &ComManager::usersReceived, this, &SessionWidget::processUsersReply);
    connect(m_comManager, &ComManager::participantsReceived, this, &SessionWidget::processParticipantsReply);
    connect(m_comManager, &ComManager::devicesReceived, this, &SessionWidget::processDevicesReply);

    connect(m_comManager, &ComManager::deleteResultsOK, this, &SessionWidget::deleteDataReply);

    connect(ui->wdgSessionInvitees, &SessionInviteWidget::newInvitees, this, &SessionWidget::sessionInviteesChanged);
    connect(ui->wdgSessionInvitees, &SessionInviteWidget::removedInvitees, this, &SessionWidget::sessionInviteesChanged);

    connect(ui->tabAssets, &AssetsWidget::assetCountChanged, this, &SessionWidget::onAssetsCountChanged);
    connect(ui->tabTests, &TestsWidget::testCountChanged, this, &SessionWidget::onTestsCountChanged);
}

void SessionWidget::on_icoUsers_clicked()
{
    ui->tabNav->setCurrentWidget(ui->tabInvitees);
}

void SessionWidget::on_icoParticipant_clicked()
{
    ui->tabNav->setCurrentWidget(ui->tabInvitees);
}

void SessionWidget::on_icoDevices_clicked()
{
    ui->tabNav->setCurrentWidget(ui->tabInvitees);
}

void SessionWidget::on_icoEvents_clicked()
{
    ui->tabNav->setCurrentWidget(ui->tabEvents);
}

void SessionWidget::on_icoAssets_clicked()
{
    ui->tabNav->setCurrentWidget(ui->tabAssets);
}

void SessionWidget::on_icoTests_clicked()
{
    ui->tabNav->setCurrentWidget(ui->tabTests);
}

void SessionWidget::on_tabNav_currentChanged(int index)
{
    // Load data depending on selected tab
    QUrlQuery query;

    QWidget* current_tab = ui->tabNav->widget(index);

    if (current_tab == ui->tabInvitees){
        /*if (ui->wdgSessionInvitees->isEditable()){
            if (!ui->wdgSessionInvitees->hasAvailableDevices() || !ui->wdgSessionInvitees->hasAvailableParticipants() || !ui->wdgSessionInvitees->hasAvailableUsers()){
                queryAvailableInvitees();
                ui->wdgSessionInvitees->selectFilterParticipant();
            }
        }*/
    }

    if (current_tab == ui->tabEvents){
        // Session events
        if (m_listSessionEvents.isEmpty() && !dataIsNew()){
            query.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(m_data->getId()));
            queryDataRequest(WEB_SESSIONEVENT_PATH, query);
        }
    }

    if (current_tab == ui->tabAssets){
        // Data
        if (ui->tabAssets->isEmpty()){
            bool enable_new_assets = m_comManager->isCurrentUserSuperAdmin();
            if (!enable_new_assets){
                // Not super admin - check if we are project admin
                enable_new_assets = m_comManager->isCurrentUserProjectAdmin(m_idProject);
                if (!enable_new_assets){
                    // Check if we are part of the users in this session
                    enable_new_assets = ui->wdgSessionInvitees->getUsersIdsInSession().contains(m_comManager->getCurrentUser().getId());
                    if (!enable_new_assets){
                        // Finally... Check if we created that session!
                        if (m_data->hasFieldName("id_creator_user")){
                            enable_new_assets = m_data->getFieldValue("id_creator_user").toInt() == m_comManager->getCurrentUser().getId();
                        }
                    }
                }
            }
            ui->tabAssets->enableNewAssets(enable_new_assets); // Enable creation of new assets, if possible.

            ui->tabAssets->displayAssetsForSession(m_data->getId());           
        }
        /*if (m_listDeviceDatas.isEmpty()){
            // Query session device data
            query.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(m_data->getId()));
            queryDataRequest(WEB_DEVICEDATAINFO_PATH, query);
        }*/
    }

    if (current_tab == ui->tabTests){
        // Tests
        ui->tabTests->enableNewTests(false);
        ui->tabTests->displayTestsForSession(m_data->getId());
    }

    if (current_tab == ui->tabInvitations){
        if (m_data){
            ui->wdgInvitations->loadForSession(m_data);
        }
    }
}

void SessionWidget::on_btnEditInvitees_toggled(bool checked)
{
    ui->wdgSessionInvitees->setEditable(checked);
    if (checked){
        if (!ui->wdgSessionInvitees->hasAvailableDevices() || !ui->wdgSessionInvitees->hasAvailableParticipants() || !ui->wdgSessionInvitees->hasAvailableUsers()){
            queryAvailableInvitees();
        }
        ui->wdgSessionInvitees->selectDefaultFilter();
    }
    ui->wdgSessionInvitees->showAvailableInvitees(checked, true);

}

