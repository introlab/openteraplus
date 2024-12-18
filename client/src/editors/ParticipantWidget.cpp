#include "ParticipantWidget.h"
#include "ui_ParticipantWidget.h"

#include <QLocale>
#include <QClipboard>
#include <QThread>
#include <QStyledItemDelegate>

#include "dialogs/GeneratePasswordDialog.h"
#include "dialogs/PasswordStrengthDialog.h"

#include "services/DanceService/DanceConfigWidget.h"

ParticipantWidget::ParticipantWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::ParticipantWidget)
{
#ifndef OPENTERA_WEBASSEMBLY
    m_sessionLobby = nullptr;
#endif
    m_allowFileTransfers = false;

    ui->setupUi(this);

    ui->cmbServices->setItemDelegate(new QStyledItemDelegate(ui->cmbServices));
    ui->cmbSessionType->setItemDelegate(new QStyledItemDelegate(ui->cmbSessionType));

    ui->btnEmailWeb->hide();

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image
    setLimited(false);

    // Use base class to manage editing
    setEditorControls(ui->wdgParticipant, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Initialize user interface
    initUI();

    // Connect signals and slots
    connectSignals();

    ParticipantWidget::setData(data);

    // Query form definition
    QUrlQuery args(WEB_FORMS_QUERY_PARTICIPANT);
    if (m_data->hasFieldName("id_project")){
        args.addQueryItem(WEB_QUERY_ID_PROJECT, m_data->getFieldValue("id_project").toString());
    }else{
        if (!dataIsNew())
            args.addQueryItem(WEB_QUERY_ID, QString::number(m_data->getId()));
    }
    queryDataRequest(WEB_FORMS_PATH, args);
    ui->wdgParticipant->setComManager(m_comManager);

    if (!dataIsNew()){
        // Query services if not a new participant
        args.clear();
        args.addQueryItem(WEB_QUERY_LIST, "1");
        args.addQueryItem(WEB_QUERY_ID_PROJECT, m_data->getFieldValue("id_project").toString());
        queryDataRequest(WEB_SERVICEINFO_PATH, args);

        // Query devices if not a new participant
        args.clear();
        args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getFieldValue("id_project").toInt()));
        queryDataRequest(WEB_DEVICEPROJECTINFO_PATH, args);

        // Query test types if not a new participant
        args.clear();
        args.addQueryItem(WEB_QUERY_LIST, "1");
        args.addQueryItem(WEB_QUERY_ID_PROJECT, m_data->getFieldValue("id_project").toString());
        queryDataRequest(WEB_TESTTYPEINFO_PATH, args);

    }

    // Default display
    ui->tabNav->setCurrentIndex(0);
    ui->tabServicesDetails->setCurrentIndex(0);
    ui->tabInfosDetails->setCurrentIndex(0);

}

ParticipantWidget::~ParticipantWidget()
{
    qDeleteAll(m_ids_session_types);
    delete ui;

#ifndef OPENTERA_WEBASSEMBLY
    if (m_sessionLobby) {
        m_sessionLobby->deleteLater();
    }
#endif

    if (m_diag_qr) {
        m_diag_qr->deleteLater();
    }
}




void ParticipantWidget::saveData(bool signal)
{
    // If data is new, we request all the fields.
    QJsonDocument part_data = ui->wdgParticipant->getFormDataJson(m_data->isNew());
    //qDebug() << part_data.toJson();

    postDataRequest(WEB_PARTICIPANTINFO_PATH, part_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgParticipant->getFormDataObject(TERADATA_PARTICIPANT);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void ParticipantWidget::setData(const TeraData *data)
{
    DataEditorWidget::setData(data);

    if (!dataIsNew()){
        // Query stats
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_PARTICIPANT, QString::number(m_data->getId()));
        queryDataRequest(WEB_STATS_PATH, args);
    }
}

void ParticipantWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &ParticipantWidget::processFormsReply);
    connect(m_comManager, &ComManager::sessionTypesReceived, this, &ParticipantWidget::processSessionTypesReply);
    connect(m_comManager, &ComManager::deviceProjectsReceived, this, &ParticipantWidget::processDeviceProjectsReply);
    connect(m_comManager, &ComManager::deviceParticipantsReceived, this, &ParticipantWidget::processDeviceParticipantsReply);
    connect(m_comManager, &ComManager::participantsReceived, this, &ParticipantWidget::processParticipantsReply);
    connect(m_comManager, &ComManager::servicesReceived, this, &ParticipantWidget::processServicesReply);
    connect(m_comManager, &ComManager::testTypesReceived, this, &ParticipantWidget::processTestTypesReply);
    connect(m_comManager, &ComManager::statsReceived, this, &ParticipantWidget::processStatsReply);
    connect(m_comManager, &ComManager::deleteResultsOK, this, &ParticipantWidget::deleteDataReply);


    connect(m_comManager->getWebSocketManager(), &WebSocketManager::participantEventReceived, this, &ParticipantWidget::ws_participantEvent);
    connect(ui->btnAddDevice, &QPushButton::clicked, this, &ParticipantWidget::btnAddDevice_clicked);
    connect(ui->btnDelDevice, &QPushButton::clicked, this, &ParticipantWidget::btnDelDevice_clicked);

    connect(ui->lstAvailDevices, &QListWidget::currentItemChanged, this, &ParticipantWidget::currentAvailDeviceChanged);
    connect(ui->lstDevices, &QListWidget::currentItemChanged, this, &ParticipantWidget::currentDeviceChanged);
#ifndef OPENTERA_WEBASSEMBLY
    connect(ui->wdgSessions, &SessionsListWidget::startSessionRequested, this, &ParticipantWidget::showSessionLobby);
#endif
    connect(ui->wdgSessions, &SessionsListWidget::sessionsCountUpdated, this, &ParticipantWidget::sessionTotalCountUpdated);
}

void ParticipantWidget::updateControlsState()
{
    if (dataIsNew()){
        // Clean up the widget
        if (ui->tabNav->count() > 1){
            ui->tabNav->setCurrentWidget(ui->tabInfos);
            ui->tabNav->removeTab(0);

            while (ui->tabNav->count() > 1)
                ui->tabNav->removeTab(1);

            ui->frameNewSession->hide();
        }
    }else{
        bool is_admin = isProjectAdmin();
        // Not admin in current project - disable deleting, enable it otherwise
        ui->wdgSessions->enableDeletion(is_admin);
        if (!is_admin){
            // Also disable device assignation
            ui->frameAssignDevicesButtons->hide();
            ui->frameAvailDevices->hide();
        }
    }
}

void ParticipantWidget::updateFieldsValue()
{

    if (m_data){
        if (!dataIsNew()){
            ui->lblTitle->setText(m_data->getName());
            ui->chkEnabled->setChecked(m_data->getFieldValue("participant_enabled").toBool());
            if (m_data->hasFieldName("participant_project_enabled")){
                ui->chkEnabled->setEnabled(m_data->getFieldValue("participant_project_enabled").toBool());
            }
            ui->chkLogin->setChecked(m_data->getFieldValue("participant_login_enabled").toBool());
            if (ui->chkLogin->isChecked()){
                ui->btnSaveLogin->setEnabled(false); // Disable save login infos on already enabled participants
            }
            ui->chkWebAccess->setChecked(m_data->getFieldValue("participant_token_enabled").toBool());
            refreshWebAccessUrl();
            ui->txtUsername->setText(m_data->getFieldValue("participant_username").toString());

            // Hide service combo box if only one service
            ui->cmbServices->setVisible(ui->cmbServices->count()>1);
            ui->frameWeb->setVisible(ui->cmbServices->count()>0 && ui->chkWebAccess->isChecked());

            // Must "trigger" the slots for username - password, since they are not set otherwise
            on_txtUsername_textEdited(ui->txtUsername->text());
            on_txtPassword_textEdited("");

            // Status
            //ui->icoOnline->setVisible(m_data->isEnabled());
            ui->icoTitle->setPixmap(QPixmap(m_data->getIconStateFilename()));
            if (m_data->isBusy()){
                ui->icoOnline->setPixmap(QPixmap("://status/status_busy.png"));
            }else if (m_data->isOnline()){
                ui->icoOnline->setPixmap(QPixmap("://status/status_online.png"));
            }else{
                ui->icoOnline->setPixmap(QPixmap("://status/status_offline.png"));
            }

            bool can_start = canStartNewSession();
            //ui->frameNewSession->setVisible(canStartNewSession());
            ui->btnNewSession->setEnabled(can_start);
            ui->cmbSessionType->setVisible(can_start);
            ui->lblInfos->setVisible(!can_start);
        }

        if (ui->wdgParticipant->formHasStructure())
            ui->wdgParticipant->fillFormFromData(m_data->toJson());
        on_cmbSessionType_currentIndexChanged(ui->cmbSessionType->currentIndex());

    }
}

void ParticipantWidget::initUI()
{
    ui->wdgSessions->setComManager(m_comManager);
    ui->wdgSessions->setViewMode(SessionsListWidget::VIEW_PARTICIPANT_SESSIONS, m_data->getUuid(), m_data->getId(), m_data->getFieldValue("id_project").toInt());

    ui->frameParticipantLogin->hide();
    ui->frameActive->hide();
    ui->frameWeb->hide();
    ui->txtWeb->hide();
    ui->icoOnline->hide();

    // Disable random password button, handled in the PasswordStrengthDialog now!
    ui->btnRandomPass->hide();

    // Hide some fields in the detailled participant widget
    QStringList ignore_fields = {"participant_enabled", "participant_token_enabled", "participant_token", "participant_login_enabled",
                                "participant_username", "participant_password"};
    ui->wdgParticipant->hideFields(ignore_fields);

    // Hide device, service and invitation tabs by default
    ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabDevices), false);
    ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabServices), false);
    ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabInvitations), false);
    ui->tabServicesDetails->setTabVisible(ui->tabServicesDetails->indexOf(ui->tabServiceParams), false);

    // Configure log view
    ui->wdgLogins->setComManager(m_comManager);
    ui->wdgLogins->setViewMode(LogViewWidget::VIEW_LOGINS_PARTICIPANT, m_data->getUuid());
    ui->wdgLogins->setUuidName(m_data->getUuid(), m_data->getName());

    // Configure invitation widget
    ui->wdgInvitations->setComManager(m_comManager);

}

bool ParticipantWidget::canStartNewSession(const int &id_session_type)
{
    if (!m_data)
        return false;

    bool can_start = true;

    if (!m_data->isEnabled()){
        ui->lblInfos->setText(tr("Participant désactivé"));
        can_start = false;
    }

    if (m_data->isBusy()){
        ui->lblInfos->setText(tr("Participant en séance"));
        can_start = false;
    }

    if (!m_data->getFieldValue("participant_login_enabled").toBool() && !m_data->getFieldValue("participant_token_enabled").toBool()){
        ui->lblInfos->setText(tr("Le participant n'a pas d'accès (web ou identification)"));
        can_start = false;
    }

    if (m_ids_session_types.isEmpty()){
        ui->lblInfos->setText(tr("Aucun type de séance associé au projet"));
        can_start = false;
    }

    if (can_start && id_session_type>0){
        // Check if that session type is in the handled service keys for that version of OpenTeraPlus
        if (m_ids_session_types.contains(id_session_type)){
            TeraData* session_type = m_ids_session_types[id_session_type];
            if (session_type->hasFieldName("session_type_service_key")){
                QString service_key = session_type->getFieldValue("session_type_service_key").toString();
                can_start = BaseServiceWidget::getHandledServiceKeys().contains(service_key);
            }else{
                ui->lblInfos->setText(tr("Ce type de séance n'est pas supporté dans cette version"));
                can_start = false;
            }

        }else{
            // Unknown session type - can't start!
            ui->lblInfos->setText(tr("Type de séance inconnu"));
            can_start = false;
        }
    }
    return can_start;
}

bool ParticipantWidget::validateData()
{
    bool valid = false;

    valid = ui->wdgParticipant->validateFormData();

    return valid;
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

void ParticipantWidget::updateServiceTabs()
{
    if (dataIsNew())
        return;

    QList<int> ids_service;

    for(const TeraData &service: std::as_const(m_services)){
        ids_service.append(service.getId());

        // Create specific tabs for services
        addServiceTab(service);
    }

    // Remove tabs not anymore present
    /*for(QWidget* tab: std::as_const(m_services_tabs)){
        if (!ids_service.contains(m_services_tabs.key(tab))){
            ui->tabNav->removeTab(ui->tabNav->indexOf(tab));
            tab->deleteLater();
            m_services_tabs.remove(m_services_tabs.key(tab));
        }
    }*/
}

void ParticipantWidget::addServiceTab(const TeraData &service)
{
    int id_service = service.getId();
    if (m_services_tabs.contains(id_service)) // Already there
        return;

    QString service_key = service.getFieldValue("service_key").toString();

    // Dance Service
    if (service_key == "DanceService"){
        DanceConfigWidget* wdg = new DanceConfigWidget(m_comManager, m_data->getFieldValue("id_project").toInt(), m_data->getUuid());
        ui->tabServicesDetails->insertTab(0, wdg, QIcon("://icons/service.png"), service.getName());
        m_services_tabs.insert(id_service, wdg);
    }

    int indexTabServices = ui->tabNav->indexOf(ui->tabServices);
    if (!ui->tabNav->isTabVisible(indexTabServices)){
        ui->tabNav->setTabVisible(indexTabServices, true);
        // Select first tab
        ui->tabServicesDetails->setCurrentIndex(0);
    }
}

void ParticipantWidget::refreshWebAccessUrl()
{
    int index = ui->cmbServices->currentIndex();
    if (index >= m_services.count() || index <0 || dataIsNew())
        return;

    QString service_url = TeraData::getServiceParticipantUrl(m_services[index],
                                                             m_comManager->getServerUrl(),
                                                             m_data->getFieldValue("participant_token").toString());

    ui->txtWeb->setText(service_url);
}

void ParticipantWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type.startsWith(WEB_FORMS_QUERY_PARTICIPANT)){
        if (!ui->wdgParticipant->formHasStructure())
            ui->wdgParticipant->buildUiFromStructure(data);
        return;
    }
}

void ParticipantWidget::processSessionTypesReply(QList<TeraData> session_types)
{
    // ui->cmbSessionType->clear();

    for (const TeraData &st:session_types){
        if (!m_ids_session_types.contains(st.getId())){
            m_ids_session_types[st.getId()] = new TeraData(st);

            // New session ComboBox
            QString ses_type_name = st.getName();
            /*if (st.getFieldValue("session_type_online").toBool())
                ses_type_name += " (en ligne)";*/

            // Check if it is a session type handled
            if (st.hasFieldName("session_type_category")){
                TeraSessionCategory::SessionTypeCategories st_category = static_cast<TeraSessionCategory::SessionTypeCategories>(st.getFieldValue("session_type_category").toInt());
                if (BaseServiceWidget::getHandledSessionCategories().contains(st_category)){
                    bool can_start_session = true;
                    if (st_category == TeraSessionCategory::SESSION_TYPE_SERVICE){
                        if (st.hasFieldName("session_type_service_key")){
                            can_start_session = BaseServiceWidget::getHandledServiceKeys().contains(st.getFieldValue("session_type_service_key").toString());
                        }else{
                            can_start_session=false;
                        }
                    }
                    if (can_start_session){
                        ui->cmbSessionType->addItem(ses_type_name, st.getId());
                    }
                }
            }


        }else{
            for (int i=0; i<ui->cmbSessionType->count(); i++){
                if (ui->cmbSessionType->itemData(i).toInt() == st.getId()){
                    ui->cmbSessionType->setItemText(i, st.getName());
                }
            }
        }
    }

    // Query sessions for that participant
    if (!m_data->isNew()){
        // Select current session type based on last session type used with that participant
        int last_session_type_id = TeraSettings::getUserSettingForProject(m_comManager->getCurrentUser().getUuid(),
                                                                          m_data->getFieldValue("id_project").toInt(),
                                                                          SETTINGS_LASTSESSIONTYPEID).toInt();

        if (last_session_type_id > 0){
            // We have a setting - try to select it from the list
            for (int i=0; i<ui->cmbSessionType->count(); i++){
                if (ui->cmbSessionType->itemData(i).toInt() == last_session_type_id){
                    ui->cmbSessionType->setCurrentIndex(i);
                    break;
                }
            }
        }

        // Query sessions
        ui->wdgSessions->setSessionTypes(session_types);
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

    ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabDevices), !device_projects.isEmpty());
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

void ParticipantWidget::processParticipantsReply(QList<TeraData> participants)
{
    for (int i=0; i<participants.count(); i++){
        if (participants.at(i) == *m_data){
            // We found "ourself" in the list - update data.
            m_data->updateFrom(participants.at(i));
            updateFieldsValue();
            break;
        }
    }
}

void ParticipantWidget::processTestTypesReply(QList<TeraData> test_types, QUrlQuery reply_query)
{
    if (dataIsNew())
        return;

    if (!reply_query.hasQueryItem(WEB_QUERY_ID_PROJECT))
        return;

    if (reply_query.queryItemValue(WEB_QUERY_ID_PROJECT) == m_data->getFieldValue("id_project").toString()){
        // For us!
        m_testTypes.clear();
        m_testTypes = test_types;
        ui->wdgInvitations->setCurrentTestTypes(m_testTypes);
        ui->tabNav->setTabVisible(ui->tabNav->indexOf(ui->tabInvitations), !m_testTypes.isEmpty());
    }

}

void ParticipantWidget::processServicesReply(QList<TeraData> services, QUrlQuery reply_query)
{
    if (!reply_query.hasQueryItem(WEB_QUERY_ID_PROJECT))
        return; // Probably not for us!

    if (reply_query.queryItemValue(WEB_QUERY_ID_PROJECT).toInt() != m_data->getFieldValue("id_project").toInt())
        return; // Really not for us!

    ui->cmbServices->clear();
    m_services.clear();

    bool has_email_service = false;
    foreach(TeraData service, services){
        QString service_key = service.getFieldValue("service_key").toString();
        if (service_key != "FileTransferService"){
            m_services.append(service);
            ui->cmbServices->addItem(service.getName(), service_key);
        }else{
            m_allowFileTransfers = true; // We have a file transfer service with that project - allow uploads!
            ui->wdgSessions->enableFileTransfers(true);
        }

        if (service_key == "EmailService"){
            has_email_service = true;
        }

        if (service.hasFieldName("service_editable_config")){
            if (service.getFieldValue("service_editable_config").toBool()){
                // At least one service with editable config - show services tabs
                int indexTabServices = ui->tabNav->indexOf(ui->tabServices);
                if (!ui->tabNav->isTabVisible(indexTabServices)){
                    ui->tabNav->setTabVisible(indexTabServices, true);
                }
                indexTabServices = ui->tabServicesDetails->indexOf(ui->tabServiceParams);
                ui->tabServicesDetails->setTabVisible(indexTabServices, true);
            }
        }
    }

    ui->btnEmailWeb->setVisible(has_email_service);
    ui->wdgInvitations->setEnableEmail(has_email_service);

    // Find and select VideoRehab by default in the combobox
    int default_index = ui->cmbServices->findData("VideoRehabService");
    if (default_index>=0)
        ui->cmbServices->setCurrentIndex(default_index);

    // Add specific services configuration tabs
    updateServiceTabs();
}

void ParticipantWidget::processStatsReply(TeraData stats, QUrlQuery reply_query)
{
    // Check if stats are for us
    if (!reply_query.hasQueryItem("id_participant"))
        return;

    if (reply_query.queryItemValue("id_participant").toInt() != m_data->getId())
        return;

    // Fill stats information
    int total_sessions = stats.getFieldValue("sessions_total_count").toInt();
    ui->wdgSessions->setSessionsCount(total_sessions);

    // Query sessions types
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_PROJECT, m_data->getFieldValue("id_project").toString());
    queryDataRequest(WEB_SESSIONTYPE_PATH, args);
}

void ParticipantWidget::deleteDataReply(QString path, int id)
{
    if (id==0)
        return;

    if (path == WEB_SESSIONINFO_PATH){
        // Nothing to do here - all managed in SessionsListWidget
    }

    if (path == WEB_DEVICEPARTICIPANTINFO_PATH){
        // A participant device association was deleted
        for (QListWidgetItem* item: std::as_const(m_listDevices_items)){
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

void ParticipantWidget::ws_participantEvent(opentera::protobuf::ParticipantEvent event)
{
    if (!m_data)
        return;

    if (QString::fromStdString(event.participant_uuid()) != m_data->getUuid())
        return; // Not for us!

    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_CONNECTED){
        m_data->setOnline(true);
    }
    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_DISCONNECTED){
        m_data->setOnline(false);
    }
    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_JOINED_SESSION){
        m_data->setBusy(true);
    }
    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_LEFT_SESSION){
        m_data->setBusy(false);
    }
    updateFieldsValue();
}

void ParticipantWidget::sessionTotalCountUpdated(int new_count)
{
    ui->grpSession->setTitle(tr("Séances") + " ( " + QString::number(new_count) + " )");
}

void ParticipantWidget::btnAddDevice_clicked()
{
    if (!isProjectAdmin())
        return;

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
            QList<int> ids = diag->getDeviceParticipantsIds();
            for (int id:std::as_const(ids)){
                deleteDataRequest(WEB_DEVICEPARTICIPANTINFO_PATH, id);
            }
        }
    }

    // Add device to participant
    QJsonDocument document;
    QJsonObject base_obj;
   // QJsonArray devices;

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
    if (!isProjectAdmin())
        return;

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

bool ParticipantWidget::isProjectAdmin()
{
    if (!m_comManager)
        return false;

    return m_comManager->getCurrentUserProjectRole(m_data->getFieldValue("id_project").toInt()) == "admin";
}
#ifndef OPENTERA_WEBASSEMBLY
void ParticipantWidget::showSessionLobby(const int &id_session_type, const int &id_session)
{
    if (!canStartNewSession(id_session_type)){
        GlobalMessageBox msg;
        msg.showError(tr("Impossible de démarrer cette séance"), tr("Impossible de démarrer cette séance."));
        return;
    }

    if (m_sessionLobby)
        m_sessionLobby->deleteLater();
    m_sessionLobby = new SessionLobbyDialog(m_comManager, *m_ids_session_types[id_session_type], m_data->getFieldValue("id_project").toInt(), id_session, this);


    // Add current participant to session
    m_sessionLobby->addParticipantsToSession(QList<TeraData>() << *m_data, QList<int>() << m_data->getId());

    // Add current user to session
    m_sessionLobby->addUsersToSession(QList<TeraData>() << m_comManager->getCurrentUser(), QList<int>() <<m_comManager->getCurrentUser().getId());

    connect(m_sessionLobby, &QDialog::accepted, this, &ParticipantWidget::sessionLobbyStartSessionRequested);
    connect(m_sessionLobby, &QDialog::rejected, this, &ParticipantWidget::sessionLobbyStartSessionCancelled);
    if (height()<800)
        m_sessionLobby->showMaximized();
    else{
        m_sessionLobby->setMinimumSize(3*QGuiApplication::primaryScreen()->availableGeometry().width() / 4,
                                       2*QGuiApplication::primaryScreen()->availableGeometry().height() / 3);
    }

    // Show Session Lobby
    m_sessionLobby->exec();
}

void ParticipantWidget::sessionLobbyStartSessionRequested()
{

    int id_session_type = m_sessionLobby->getIdSessionType(); //ui->cmbSessionType->currentData().toInt();

    // Save last session type for further use
    TeraSettings::setUserSettingForProject(m_comManager->getCurrentUser().getUuid(),
                                           m_data->getFieldValue("id_project").toInt(),
                                           SETTINGS_LASTSESSIONTYPEID,
                                           id_session_type);

    // Start session
    m_comManager->startSession(*m_ids_session_types[id_session_type],
                               m_sessionLobby->getIdSession(),
                               m_sessionLobby->getSessionParticipantsUuids(),
                               m_sessionLobby->getSessionUsersUuids(),
                               m_sessionLobby->getSessionDevicesUuids(),
                               m_sessionLobby->getSessionConfig());

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
#endif
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
            //saveData();
            QJsonDocument document;
            QJsonObject data_obj;
            QJsonObject base_obj;

            TeraData* part_info = ui->wdgParticipant->getFormDataObject(TERADATA_PARTICIPANT);
            data_obj.insert("id_participant", QJsonValue::fromVariant(part_info->getId()));
            //data_obj.insert("id_project", QJsonValue::fromVariant(part_info->getFieldValue("id_project")));
            data_obj.insert("participant_enabled", QJsonValue::fromVariant(current_state));

            base_obj.insert(TeraData::getDataTypeName(TERADATA_PARTICIPANT), data_obj);
            document.setObject(base_obj);

            postDataRequest(WEB_PARTICIPANTINFO_PATH, document.toJson());

        }
    }

    // Update UI
    ui->frameActive->setVisible(current_state);

}

void ParticipantWidget::on_chkWebAccess_stateChanged(int checkState)
{
    bool current_state = (checkState == Qt::Checked);

    if (!current_state){
        GlobalMessageBox msg;
        if (msg.showYesNo(tr("Confirmation"), tr("En désactivant l'accès web, le lien sera supprimé.\n\nSi un accès est à nouveau créé, le lien sera différent et il faudra envoyer à nouveau le lien au participant.\n\nSouhaitez-vous continuer?")) != QMessageBox::Yes){
            ui->chkWebAccess->setCheckState(Qt::Checked); // Warning: use "setCheckState" and not "setChecked" since the signal is on CheckState!
            return;
        }
    }

    if (m_data){
        if (m_data->getFieldValue("participant_token_enabled").toBool() != current_state){
            ui->wdgParticipant->setFieldValue("participant_token_enabled", current_state);
            //saveData();
            QJsonDocument document;
            QJsonObject data_obj;
            QJsonObject base_obj;

            TeraData* part_info = ui->wdgParticipant->getFormDataObject(TERADATA_PARTICIPANT);
            data_obj.insert("id_participant", QJsonValue::fromVariant(part_info->getId()));
            data_obj.insert("id_project", QJsonValue::fromVariant(part_info->getFieldValue("id_project")));
            data_obj.insert("participant_token_enabled", QJsonValue::fromVariant(current_state));

            base_obj.insert(TeraData::getDataTypeName(TERADATA_PARTICIPANT), data_obj);
            document.setObject(base_obj);

            postDataRequest(WEB_PARTICIPANTINFO_PATH, document.toJson());
        }
    }

    ui->frameWeb->setVisible(checkState == Qt::Checked);
}

void ParticipantWidget::on_chkLogin_stateChanged(int checkState)
{
    bool current_state = (checkState == Qt::Checked);
    if (m_data){
        if (m_data->getFieldValue("participant_login_enabled").toBool() != current_state){
            ui->wdgParticipant->setFieldValue("participant_login_enabled", current_state);
            //saveData();
            if (!current_state){ // Disable login
                QJsonDocument document;
                QJsonObject data_obj;
                QJsonObject base_obj;

                TeraData* part_info = ui->wdgParticipant->getFormDataObject(TERADATA_PARTICIPANT);
                data_obj.insert("id_participant", QJsonValue::fromVariant(part_info->getId()));
                data_obj.insert("id_project", QJsonValue::fromVariant(part_info->getFieldValue("id_project")));
                data_obj.insert("participant_login_enabled", QJsonValue::fromVariant(current_state));

                base_obj.insert(TeraData::getDataTypeName(TERADATA_PARTICIPANT), data_obj);
                document.setObject(base_obj);

                postDataRequest(WEB_PARTICIPANTINFO_PATH, document.toJson());
                ui->btnSaveLogin->setEnabled(false);
            }else{
                ui->txtPassword->setStyleSheet("background-color: #ffaaaa;");
                ui->btnSaveLogin->setEnabled(false);
            }
        }
    }

    ui->frameParticipantLogin->setVisible(checkState == Qt::Checked);
    on_cmbSessionType_currentIndexChanged(ui->cmbSessionType->currentIndex());
}

void ParticipantWidget::on_btnCopyWeb_clicked()
{
    QClipboard* clipboard = QApplication::clipboard();

    clipboard->setText(ui->txtWeb->text(), QClipboard::Clipboard);

    /*if (clipboard->supportsSelection()) {
        clipboard->setText(ui->txtWeb->text(), QClipboard::Selection);
    }*/

    #if defined(Q_OS_LINUX)
        QThread::msleep(1); //workaround for copied text not being available...
    #endif

}

void ParticipantWidget::on_btnRandomPass_clicked()
{
    // Show random password dialog
    GeneratePasswordDialog dlg;

    if (dlg.exec() == QDialog::Accepted){
        QString password = dlg.getPassword();
        ui->txtPassword->setText(password);
    }
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

    if (ui->txtPassword->text().isEmpty()){
        all_ok = false;
        err_msg.append(tr("Aucun mot de passe spécifié."));
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
    if (current.isEmpty())
        return;

    // Show password validator
    PasswordStrengthDialog dlg(current);
    //QLineEdit* wdg_editor = dynamic_cast<QLineEdit*>(ui->wdgUser->getWidgetForField("user_password"));
    //dlg.setCursorPosition(wdg_editor->cursorPosition());

    if (dlg.exec() == QDialog::Accepted){
        //m_passwordJustGenerated = true;
        ui->txtPassword->setText(dlg.getCurrentPassword());
        ui->btnSaveLogin->setEnabled(true);
    }else{
        ui->txtPassword->setText("");
        ui->btnSaveLogin->setEnabled(false);
        //wdg_editor->undo();
    }

    if (!m_data->getFieldValue("participant_login_enabled").toBool() && ui->txtPassword->text().isEmpty()){
        ui->txtPassword->setStyleSheet("background-color: #ffaaaa;");
    }else{
        ui->txtPassword->setStyleSheet("");
    }

    /*QString confirm_pass = ui->txtPasswordConfirm->text();
    if (current != confirm_pass || ui->txtPassword->text().isEmpty()){
        ui->txtPassword->setStyleSheet("background-color: #ffaaaa;");
        ui->txtPasswordConfirm->setStyleSheet("background-color: #ffaaaa;");
    }else{
        ui->txtPassword->setStyleSheet("");
        ui->txtPasswordConfirm->setStyleSheet("");
    }*/
}

void ParticipantWidget::on_btnNewSession_clicked()
{
    if (ui->cmbSessionType->currentIndex() < 0)
        return;

    if (!m_data)
        return;

    int id_session_type = ui->cmbSessionType->currentData().toInt();
    int id_session = 0;

    // Check if we have a recent session (started in the last hour) of the same type we might want to continue
    const TeraData* session = ui->wdgSessions->hasResumableSession(id_session_type, QDate::currentDate());
    if (session){
        int session_status = session->getFieldValue("session_status").toInt();
        GlobalMessageBox msg;
        QString status_msg = tr("existe déjà.");
        if (session_status == TeraSessionStatus::STATUS_INPROGRESS){
            status_msg = tr("a été réalisée récemment et n'a pas été terminée.");
        }
        if (session_status == TeraSessionStatus::STATUS_NOTSTARTED){
            status_msg = tr("a été planifiée.");
        }
        if (msg.showYesNo(tr("Reprendre une séance?"), tr("Un séance de ce type, ") + session->getFieldValue("session_name").toString() + ", "
                      + status_msg + tr("\n\nSouhaitez-vous continuer cette séance?")) == QMessageBox::Yes){
            id_session = session->getId(); // Will resume that session
        }
    }

    // If id_session == 0, will start a new session. Otherwise, will resume that session with id_session
#ifndef OPENTERA_WEBASSEMBLY
    showSessionLobby(id_session_type, id_session);
#endif
}

void ParticipantWidget::on_btnViewLink_clicked()
{
    ui->txtWeb->setVisible(ui->btnViewLink->isChecked());
    if (ui->btnViewLink->isChecked()){
        QLayoutItem* spacer = ui->frameWeb->layout()->takeAt(ui->frameWeb->layout()->count()-1);
        if (spacer)
            delete spacer;
        ui->txtWeb->setCursorPosition(0);
    }else{
        ui->frameWeb->layout()->addItem(new QSpacerItem(40,20,QSizePolicy::Expanding));
    }
}

void ParticipantWidget::on_cmbServices_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    refreshWebAccessUrl();

}

void ParticipantWidget::on_btnEmailWeb_clicked()
{
    EmailInviteDialog email_diag(m_comManager, m_data, this);

    QHash<QString, QString>fields;
    fields["$join_link"] = "<a href=\"" + ui->txtWeb->text() + "\">" + tr("cliquez ici") + "</a>";

    email_diag.setFieldValues(fields);

    email_diag.exec();
}

void ParticipantWidget::on_cmbSessionType_currentIndexChanged(int index)
{
    Q_UNUSED(index)

    if (ui->cmbSessionType->currentIndex() < 0){
        ui->btnNewSession->setDisabled(true);
        return;
    }

    if (!m_data)
        return;

    // Online session - make sure that the participant can login to allow such a session
    int id_session_type = ui->cmbSessionType->currentData().toInt();
    bool can_start = canStartNewSession();
    ui->btnNewSession->setEnabled(can_start);
    ui->cmbSessionType->setVisible(can_start);

    // Session type related to a service: select the correct item in the link combo box to generate correct link
    if (m_ids_session_types[id_session_type]->hasFieldName("session_type_service_key")){
        QString service_key = m_ids_session_types[id_session_type]->getFieldValue("session_type_service_key").toString();
        for(int i=0; i<ui->cmbServices->count(); i++){
            if (ui->cmbServices->itemData(i).toString() == service_key){
                ui->cmbServices->setCurrentIndex(i);
                break;
            }
        }
    }

}

void ParticipantWidget::on_btnAddSession_clicked()
{
    QDateTime session_datetime = QDateTime::currentDateTime();
    QTime session_time = QTime::currentTime();
    int minutes = (session_time.minute() / 15) * 15;
    session_time.setHMS(session_datetime.time().addSecs(3600).hour(), minutes, 0);
    session_datetime.setTime(session_time);

    ui->wdgSessions->newSessionRequest(session_datetime);

}

void ParticipantWidget::on_tabNav_currentChanged(int index)
{
    QUrlQuery query;

    QWidget* current_tab = ui->tabNav->widget(index);

    if (current_tab == ui->tabDevices){ // Devices
        if (m_listDevices_items.isEmpty()){
            query.addQueryItem(WEB_QUERY_ID_PARTICIPANT, QString::number(m_data->getId()));
            queryDataRequest(WEB_DEVICEPARTICIPANTINFO_PATH, query);

            // Query devices for the current project - already done when loading widget
            /*query.removeQueryItem(WEB_QUERY_ID_PARTICIPANT);
            query.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_data->getFieldValue("id_project").toInt()));
            queryDataRequest(WEB_DEVICEPROJECTINFO_PATH, query);*/
        }
    }

    if (current_tab == ui->tabServices){ // Services
        if (!ui->wdgServiceConfig->layout()){
            QHBoxLayout* layout = new QHBoxLayout();

            //layout->setMargin(0);
            ui->wdgServiceConfig->setLayout(layout);
        }
        if (ui->wdgServiceConfig->layout()->count() == 0){
            ServiceConfigWidget* service_config_widget = new ServiceConfigWidget(m_comManager, m_data->getIdFieldName(), m_data->getId(), ui->wdgServiceConfig);
            ui->wdgServiceConfig->layout()->addWidget(service_config_widget);
        }
    }

    // Test invitations
    if (current_tab == ui->tabInvitations){
        if (m_data){
            ui->wdgInvitations->loadForParticipant(m_data);
        }
    }
}


void ParticipantWidget::on_lstAvailDevices_itemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item)
    btnAddDevice_clicked();
}


void ParticipantWidget::on_lstDevices_itemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item)
    btnDelDevice_clicked();
}

void ParticipantWidget::on_btnQR_clicked()
{
    if (m_diag_qr){
        m_diag_qr->deleteLater();
    }

    m_diag_qr = new QRCodeDialog(ui->txtWeb->text(), this);
    m_diag_qr->setContext(m_data->getName());
    m_diag_qr->open();

    /*if (m_diag_editor){
        m_diag_editor->deleteLater();
    }
    m_diag_editor = new BaseDialog(this);

    QRWidget* qr_widget = new QRWidget(m_diag_editor);
    qr_widget->setText(ui->txtWeb->text());
    m_diag_editor->setCentralWidget(qr_widget);

    m_diag_editor->setWindowTitle(tr("Code QR du lien"));
    m_diag_editor->setFixedSize(this->height()/2 - 40, this->height()/2);

    m_diag_editor->open();*/
}


void ParticipantWidget::on_tabInfosDetails_currentChanged(int index)
{
    if (ui->tabInfosDetails->currentWidget() == ui->tabLogins){
        ui->wdgLogins->refreshData();
    }
}


void ParticipantWidget::on_tabServicesDetails_currentChanged(int index)
{
    QWidget* current_tab = ui->tabServicesDetails->widget(index);

    if (dynamic_cast<DanceConfigWidget*>(current_tab)){
        dynamic_cast<DanceConfigWidget*>(current_tab)->refresh();
    }
}

