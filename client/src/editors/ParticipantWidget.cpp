#include "ParticipantWidget.h"
#include "ui_ParticipantWidget.h"

#include <QLocale>
#include <QClipboard>
#include <QThread>
#include <QStyledItemDelegate>

#include "editors/DataListWidget.h"
#include "editors/SessionWidget.h"

#include "dialogs/GeneratePasswordDialog.h"
#include "dialogs/PasswordStrengthDialog.h"

#include "services/DanceService/DanceConfigWidget.h"

ParticipantWidget::ParticipantWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::ParticipantWidget)
{

    m_diag_editor = nullptr;
    m_sessionLobby = nullptr;

    ui->setupUi(this);

    ui->cmbServices->setItemDelegate(new QStyledItemDelegate());
    ui->cmbSessionType->setItemDelegate(new QStyledItemDelegate());

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image
    setLimited(false);

    // Use base class to manage editing
    setEditorControls(ui->wdgParticipant, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Initialize user interface
    initUI();

    // Connect signals and slots
    connectSignals();

    setData(data);

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

    // Query sessions types
    args.clear();
    args.addQueryItem(WEB_QUERY_ID_PROJECT, m_data->getFieldValue("id_project").toString());
    queryDataRequest(WEB_SESSIONTYPE_PATH, args);

    // Query services
    args.clear();
    args.addQueryItem(WEB_QUERY_LIST, "1");
    args.addQueryItem(WEB_QUERY_ID_PROJECT, m_data->getFieldValue("id_project").toString());
    queryDataRequest(WEB_SERVICEINFO_PATH, args);

    // Set default calendar view for new participants
    if (m_data->isNew()){
        updateCalendars(QDate::currentDate());
    }

    // Default display
    ui->tabNav->setCurrentIndex(0);
    ui->tabInfos->setCurrentIndex(0);
    ui->frameFilterSessionTypes->hide();
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
    qDebug() << part_data.toJson();

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
    connect(m_comManager, &ComManager::participantsReceived, this, &ParticipantWidget::processParticipantsReply);
    connect(m_comManager, &ComManager::servicesReceived, this, &ParticipantWidget::processServicesReply);
    connect(m_comManager, &ComManager::deleteResultsOK, this, &ParticipantWidget::deleteDataReply);
    connect(m_comManager, &ComManager::downloadCompleted, this, &ParticipantWidget::onDownloadCompleted);

    connect(m_comManager->getWebSocketManager(), &WebSocketManager::participantEventReceived, this, &ParticipantWidget::ws_participantEvent);

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
    if (dataIsNew()){
        // Clean up the widget

        if (ui->tabNav->count() > 1){
            ui->tabNav->setCurrentWidget(ui->tabDetails);
            ui->tabNav->removeTab(0);

            int tabCount = ui->tabInfos->count();
            for (int i=1; i<tabCount; i++){
                ui->tabInfos->removeTab(1);
            }
        }
    }
    //ui->frameSessionsControls->setHidden(dataIsNew());
}

void ParticipantWidget::updateFieldsValue()
{

    if (m_data){
        if (!dataIsNew()){
            ui->lblTitle->setText(m_data->getName());
            ui->chkEnabled->setChecked(m_data->getFieldValue("participant_enabled").toBool());
            ui->chkLogin->setChecked(m_data->getFieldValue("participant_login_enabled").toBool());
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
            ui->icoOnline->setVisible(m_data->isEnabled());
            ui->icoTitle->setPixmap(QPixmap(m_data->getIconStateFilename()));
            if (m_data->isBusy()){
                ui->icoOnline->setPixmap(QPixmap("://status/status_busy.png"));
            }else if (m_data->isOnline()){
                ui->icoOnline->setPixmap(QPixmap("://status/status_online.png"));
            }else{
                ui->icoOnline->setPixmap(QPixmap("://status/status_offline.png"));
            }

            ui->frameNewSession->setVisible(canStartNewSession());
        }

        if (ui->wdgParticipant->formHasStructure())
            ui->wdgParticipant->fillFormFromData(m_data->toJson());
        on_cmbSessionType_currentIndexChanged(ui->cmbSessionType->currentIndex());

    }
}

void ParticipantWidget::initUI()
{
    ui->btnDownloadAll->hide();
    ui->frameLogin->hide();
    ui->frameActive->hide();
    ui->frameWeb->hide();
    ui->txtWeb->hide();
    ui->btnCheckSessionTypes->hide();

    // Hide some fields in the detailled participant widget
    QStringList ignore_fields = {"participant_enabled", "participant_token_enabled", "participant_token", "participant_login_enabled",
                                "participant_username", "participant_password"};
    ui->wdgParticipant->hideFields(ignore_fields);
}

bool ParticipantWidget::canStartNewSession()
{
    if (!m_data)
        return false;

    return m_data->isEnabled() && !m_data->isNew() && !m_data->isBusy();
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
    TableDateWidgetItem* date_item;
    QTableWidgetItem* type_item;
    QTableWidgetItem* duration_item;
    QTableWidgetItem* user_item;
    QTableWidgetItem* status_item;
    QToolButton* btnDownload = nullptr;
    QToolButton* btnResume = nullptr;

    if (m_listSessions_items.contains(id_session)){
        // Already there, get items
       name_item = m_listSessions_items[id_session];
       date_item = dynamic_cast<TableDateWidgetItem*>(ui->tableSessions->item(name_item->row(), 1));
       type_item = ui->tableSessions->item(name_item->row(), 2);
       status_item = ui->tableSessions->item(name_item->row(), 3);
       duration_item = ui->tableSessions->item(name_item->row(), 4);
       user_item = ui->tableSessions->item(name_item->row(), 5);
       if (ui->tableSessions->cellWidget(name_item->row(), 6))
           if (ui->tableSessions->cellWidget(name_item->row(), 6)->layout()){
               if (ui->tableSessions->cellWidget(name_item->row(), 6)->layout()->itemAt(2))
                  btnDownload = dynamic_cast<QToolButton*>(ui->tableSessions->cellWidget(name_item->row(), 6)->layout()->itemAt(2)->widget());
               if (ui->tableSessions->cellWidget(name_item->row(), 6)->layout()->itemAt(3))
                  btnResume = dynamic_cast<QToolButton*>(ui->tableSessions->cellWidget(name_item->row(), 6)->layout()->itemAt(3)->widget());
           }
       delete m_ids_sessions[id_session];
    }else{

        ui->tableSessions->setRowCount(ui->tableSessions->rowCount()+1);
        int current_row = ui->tableSessions->rowCount()-1;
        name_item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_SESSION)),"");
        ui->tableSessions->setItem(current_row, 0, name_item);
        date_item = new TableDateWidgetItem("");
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

        // Resume session
        btnResume = new QToolButton();
        btnResume->setIcon(QIcon(":/icons/play.png"));
        btnResume->setProperty("id_session", session->getId());
        btnResume->setProperty("id_session_type", session->getFieldValue("id_session_type").toInt());
        btnResume->setCursor(Qt::PointingHandCursor);
        btnResume->setMaximumWidth(32);
        btnResume->setToolTip(tr("Continuer la séance"));
        connect(btnResume, &QToolButton::clicked, this, &ParticipantWidget::btnResumeSession_clicked);
        layout->addWidget(btnResume);

        ui->tableSessions->setCellWidget(current_row, 6, action_frame);

        m_listSessions_items[id_session] = name_item;
    }
    m_ids_sessions[id_session] = new TeraData(*session);

    // Update values
    name_item->setText(session->getName());
    /*QDateTime session_date = session->getFieldValue("session_start_datetime").toDateTime().toLocalTime();
    date_item->setText(session_date.toString("dd-MM-yyyy hh:mm:ss"));*/
    date_item->setDate(session->getFieldValue("session_start_datetime"));
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
        /*btnDownload->setVisible(session->getFieldValue("session_has_device_data").toBool());
        if (session->getFieldValue("session_has_device_data").toBool())
            ui->btnDownloadAll->show();*/
        btnDownload->hide();
    }

    // Resume session
    if (btnResume){
        if (session->hasFieldName("session_start_datetime")){
            QDateTime session_date = session->getFieldValue("session_start_datetime").toDateTime().toLocalTime();
            btnResume->setVisible(session_date.date() == QDate::currentDate() && canStartNewSession());
        }else{
            btnResume->hide();
        }
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

void ParticipantWidget::updateServiceTabs()
{
    if (dataIsNew())
        return;

    QList<int> ids_service;

    for(const TeraData &service: qAsConst(m_services)){
        ids_service.append(service.getId());

        // Create specific tabs for services
        addServiceTab(service);
    }

    // Remove tabs not anymore present
    for(QWidget* tab: qAsConst(m_services_tabs)){
        if (!ids_service.contains(m_services_tabs.key(tab))){
            ui->tabNav->removeTab(ui->tabNav->indexOf(tab));
            tab->deleteLater();
            m_services_tabs.remove(m_services_tabs.key(tab));
        }
    }
}

void ParticipantWidget::addServiceTab(const TeraData &service)
{
    int id_service = service.getId();
    if (m_services_tabs.contains(id_service)) // Already there
        return;

    QString service_key = service.getFieldValue("service_key").toString();

    // Dance Service
    if (service_key == "DanceService"){
        DanceConfigWidget* wdg = new DanceConfigWidget(m_comManager, m_data->getFieldValue("id_project").toInt());
        ui->tabNav->addTab(wdg, QIcon("://icons/service.png"), service.getName());
        m_services_tabs.insert(id_service, wdg);

    }
}

void ParticipantWidget::refreshWebAccessUrl()
{
    int index = ui->cmbServices->currentIndex();
    if (index >= m_services.count() || index <0 || dataIsNew())
        return;

    /*TeraData* current_service = &m_services[index];
    QUrl server_url = m_comManager->getServerUrl();
    QString participant_endpoint = "";
    if (current_service->hasFieldName("service_endpoint_participant"))
        participant_endpoint = current_service->getFieldValue("service_endpoint_participant").toString();
    QString service_url = "https://" + server_url.host() + ":" + QString::number(server_url.port()) +
    //QString service_url = "https://" + current_service->getFieldValue("service_hostname").toString() + ":" + QString::number(server_url.port()) +
            current_service->getFieldValue("service_clientendpoint").toString() +
            participant_endpoint + "?token=" +
            m_data->getFieldValue("participant_token").toString();*/
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

void ParticipantWidget::processSessionsReply(QList<TeraData> sessions)
{
    for(TeraData session:sessions){
        QVariantList session_parts_list = session.getFieldValue("session_participants").toList();

        for(const QVariant &session_part:qAsConst(session_parts_list)){
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
    updateCalendars(getMaximumSessionDate().addMonths(-2));
    //updateCalendars(getMinimumSessionDate());
    ui->calMonth1->setData(m_ids_sessions.values());
    ui->calMonth2->setData(m_ids_sessions.values());
    ui->calMonth3->setData(m_ids_sessions.values());
}

void ParticipantWidget::processSessionTypesReply(QList<TeraData> session_types)
{
    ui->cmbSessionType->clear();

    for (const TeraData &st:session_types){
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
            QString ses_type_name = st.getName();
            if (st.getFieldValue("session_type_online").toBool())
                ses_type_name += " (en ligne)";
            ui->cmbSessionType->addItem(ses_type_name, st.getId());
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

void ParticipantWidget::processServicesReply(QList<TeraData> services)
{
    ui->cmbServices->clear();
    m_services.clear();

    foreach(TeraData service, services){
        m_services.append(service);
        ui->cmbServices->addItem(service.getName(), service.getFieldValue("service_key"));
    }

    // Find and select VideoRehab by default in the combobox
    int default_index = ui->cmbServices->findData("VideoRehabService");
    if (default_index>=0)
        ui->cmbServices->setCurrentIndex(default_index);

    // Add specific services configuration tabs
    updateServiceTabs();
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
            ui->calMonth3->setData(m_ids_sessions.values());
        }
    }

    if (path == WEB_DEVICEPARTICIPANTINFO_PATH){
        // A participant device association was deleted
        for (QListWidgetItem* item: qAsConst(m_listDevices_items)){
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

void ParticipantWidget::onDownloadCompleted(DownloadingFile *file)
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
            QList<int> ids = diag->getDeviceParticipantsIds();
            for (int id:qAsConst(ids)){
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
            //int id_session = button->property("id_session").toInt();
            /*QUrlQuery args;
            args.addQueryItem(WEB_QUERY_DOWNLOAD, "");
            args.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(id_session));
            downloadDataRequest(save_path, WEB_DEVICEDATAINFO_PATH, args);
            setWaiting();*/
        }
    }
}

void ParticipantWidget::btnDowloadAll_clicked()
{
    QString save_path = QFileDialog::getExistingDirectory(this, tr("Sélectionnez un dossier pour le téléchargement"),
                                                          QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    if (!save_path.isEmpty()){
        //int id_participant = m_data->getId();
        /*QUrlQuery args;
        args.addQueryItem(WEB_QUERY_DOWNLOAD, "");
        args.addQueryItem(WEB_QUERY_ID_PARTICIPANT, QString::number(id_participant));
        downloadDataRequest(save_path, WEB_DEVICEDATAINFO_PATH, args);
        setWaiting();*/
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

void ParticipantWidget::btnResumeSession_clicked()
{
    // Check if the sender is a QToolButton (from the action column)
    QToolButton* action_btn = dynamic_cast<QToolButton*>(sender());

    if (action_btn){
        // Select row according to the session id of that button
        int id_session = action_btn->property("id_session").toInt();
        int id_session_type = action_btn->property("id_session_type").toInt();
        showSessionLobby(id_session_type, id_session);
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
    //foreach(TeraData* session, m_ids_sessions){
    for(TeraData* session: qAsConst(m_ids_sessions)){
        if (session->getFieldValue("session_start_datetime").toDateTime().toLocalTime().date() == current_date){
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
        ses_data->setFieldValue("id_project", m_data->getFieldValue("id_project"));
        SessionWidget* ses_widget = new SessionWidget(m_comManager, ses_data, nullptr);
        m_diag_editor->setCentralWidget(ses_widget);

        m_diag_editor->setWindowTitle(tr("Séance"));
        //m_diag_editor->setMinimumSize(2*this->width()/3, 5*this->height()/6);
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

    if (ui->calMonth1->yearShown()<=min_date.year() && ui->calMonth1->monthShown()<=min_date.month())
        ui->btnPrevCal->setEnabled(false);
    else
        ui->btnPrevCal->setEnabled(true);

}

QDate ParticipantWidget::getMinimumSessionDate()
{
    QDate min_date = QDate::currentDate();
    for (TeraData* session:qAsConst(m_ids_sessions)){
        QDate session_date = session->getFieldValue("session_start_datetime").toDateTime().toLocalTime().date();
        if (session_date < min_date)
            min_date = session_date;
    }

    return min_date;
}

QDate ParticipantWidget::getMaximumSessionDate()
{
    QDate max_date = QDate::currentDate();
    for (TeraData* session:qAsConst(m_ids_sessions)){
        QDate session_date = session->getFieldValue("session_start_datetime").toDateTime().toLocalTime().date();
        if (session_date > max_date)
            max_date = session_date;
    }

    return max_date;
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

void ParticipantWidget::showSessionLobby(const int &id_session_type, const int &id_session)
{

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

    // Show Session Lobby
    m_sessionLobby->exec();
}

void ParticipantWidget::sessionLobbyStartSessionRequested()
{

    int id_session_type = m_sessionLobby->getIdSessionType(); //ui->cmbSessionType->currentData().toInt();
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
            }
        }
    }

    ui->frameLogin->setVisible(checkState == Qt::Checked);
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
    }else{
        ui->txtPassword->setText("");
        //wdg_editor->undo();
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
    if (ui->cmbSessionType->currentIndex() < 0)
        return;

    if (!m_data)
        return;

    int id_session_type = ui->cmbSessionType->currentData().toInt();
    int id_session = 0;

    // Check if we have a recent session (started in the last hour) of the same type we might want to continue
    foreach(TeraData* session, m_ids_sessions){
        if (id_session_type == session->getFieldValue("id_session_type").toInt()){
            int session_status = session->getFieldValue("session_status").toInt();
            if (session_status == TeraSessionStatus::STATUS_INPROGRESS || session_status == TeraSessionStatus::STATUS_NOTSTARTED){
                QDateTime session_start_time = session->getFieldValue("session_start_datetime").toDateTime().toLocalTime();

                if (session_start_time.date() == QDate::currentDate()){
                    // Adds duration to have 1h since it was ended
                    session_start_time = session_start_time.addSecs(session->getFieldValue("session_duration").toInt());
                    // Allow for +/- 1h30 around the time of the session, in case of planned session
                    if(qAbs(session_start_time.secsTo(QDateTime::currentDateTime())) <= 5400){
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
                            id_session = session->getId();
                        }
                        break;
                    }
                }
            }
        }
    }
    showSessionLobby(id_session_type, id_session);
}

void ParticipantWidget::on_btnCheckSessionTypes_clicked()
{
    for (int i=0; i<ui->lstFilters->count(); i++){
        ui->lstFilters->item(i)->setCheckState(Qt::Checked);
    }
    ui->btnUnchekSessionTypes->show();
    ui->btnCheckSessionTypes->hide();
}

void ParticipantWidget::on_btnUnchekSessionTypes_clicked()
{
    for (int i=0; i<ui->lstFilters->count(); i++){
        ui->lstFilters->item(i)->setCheckState(Qt::Unchecked);
    }
    ui->btnUnchekSessionTypes->hide();
    ui->btnCheckSessionTypes->show();
}

void ParticipantWidget::on_btnViewLink_clicked()
{
    ui->txtWeb->setVisible(ui->btnViewLink->isChecked());
}

void ParticipantWidget::on_cmbServices_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    refreshWebAccessUrl();

}

void ParticipantWidget::on_btnEmailWeb_clicked()
{
    EmailInviteDialog email_diag(m_comManager, m_data);

    QHash<QString, QString>fields;
    fields["url"] = ui->txtWeb->text();

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
    if (m_ids_session_types[id_session_type]->getFieldValue("session_type_online").toBool()){

        if (m_data->getFieldValue("participant_login_enabled").toBool() || m_data->getFieldValue("participant_token_enabled").toBool()){
            ui->btnNewSession->setEnabled(true);
        }else{
            ui->btnNewSession->setEnabled(false);
        }
    }else{
        ui->btnNewSession->setEnabled(true);
    }

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

void ParticipantWidget::on_btnFilterSessionsTypes_clicked()
{
    ui->frameFilterSessionTypes->setVisible(ui->btnFilterSessionsTypes->isChecked());
}

void ParticipantWidget::on_btnAddSession_clicked()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }
    m_diag_editor = new BaseDialog(this);

    TeraData* new_session = new TeraData(TERADATA_SESSION);
    new_session->setFieldValue("session_name", tr("Nouvelle séance"));
    QDateTime session_datetime = QDateTime::currentDateTime();
    QTime session_time = QTime::currentTime();
    int minutes = (session_time.minute() / 15) * 15;
    session_time.setHMS(session_datetime.time().addSecs(3600).hour(), minutes, 0);
    session_datetime.setTime(session_time);
    new_session->setFieldValue("session_start_datetime", session_datetime);
    new_session->setFieldValue("session_status", TeraSessionStatus::STATUS_NOTSTARTED);
    new_session->setFieldValue("id_project", m_data->getFieldValue("id_project"));
    new_session->setFieldValue("id_creator_user", m_comManager->getCurrentUser().getId());
    new_session->setFieldValue("participant_uuid", m_data->getUuid());

    SessionWidget* ses_widget = new SessionWidget(m_comManager, new_session);
    m_diag_editor->setCentralWidget(ses_widget);

    m_diag_editor->setWindowTitle(tr("Séance"));
    m_diag_editor->setMinimumSize(this->width(), this->height());

    connect(ses_widget, &SessionWidget::closeRequest, m_diag_editor, &QDialog::accept);
    connect(ses_widget, &SessionWidget::dataWasChanged, m_diag_editor, &QDialog::accept);
    connect(ses_widget, &SessionWidget::dataWasDeleted, m_diag_editor, &QDialog::accept);

    m_diag_editor->open();

}

