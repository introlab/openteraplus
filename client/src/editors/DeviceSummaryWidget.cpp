#include "DeviceSummaryWidget.h"
#include "ui_DeviceSummaryWidget.h"

#include <QLocale>

DeviceSummaryWidget::DeviceSummaryWidget(ComManager *comMan, const TeraData *data, const int &id_project, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::DeviceSummaryWidget)
{

    m_diag_editor = nullptr;
#ifndef OPENTERA_WEBASSEMBLY
    m_sessionLobby = nullptr;
#endif
    m_idProject = id_project;

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image
    setLimited(true);

    // Use base class to manage editing
    setEditorControls(ui->wdgDevice, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Initialize user interface
    initUI();

    // Connect signals and slots
    connectSignals();

    // Query form definition
    QUrlQuery args(WEB_FORMS_QUERY_DEVICE);
    if (!dataIsNew())
        args.addQueryItem(WEB_QUERY_ID, QString::number(m_data->getId()));

    queryDataRequest(WEB_FORMS_PATH, args);

    // Query sites related to that device (to check for edit access)
    args.clear();
    args.addQueryItem(WEB_QUERY_ID_DEVICE, QString::number(m_data->getId()));
    queryDataRequest(WEB_DEVICESITEINFO_PATH, args);

    // Query stats
    queryDataRequest(WEB_STATS_PATH, args); // args already contains id_device

    ui->wdgDevice->setComManager(m_comManager);
}

DeviceSummaryWidget::~DeviceSummaryWidget()
{
    delete ui;
#ifndef OPENTERA_WEBASSEMBLY
    if (m_sessionLobby)
        m_sessionLobby->deleteLater();
#endif
}

void DeviceSummaryWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &DeviceSummaryWidget::processFormsReply);
    connect(m_comManager, &ComManager::sessionTypesReceived, this, &DeviceSummaryWidget::processSessionTypesReply);
    connect(m_comManager, &ComManager::deviceSitesReceived, this, &DeviceSummaryWidget::processSiteAccessReply);
    connect(m_comManager, &ComManager::statsReceived, this, &DeviceSummaryWidget::processStatsReply);

    connect(m_comManager->getWebSocketManager(), &WebSocketManager::deviceEventReceived, this, &DeviceSummaryWidget::ws_deviceEvent);

    connect(ui->wdgSessions, &SessionsListWidget::sessionsCountUpdated, this, &DeviceSummaryWidget::sessionTotalCountUpdated);

}

void DeviceSummaryWidget::updateControlsState()
{
    ui->frameEdit->setVisible(!m_limited);
    ui->wdgSessions->enableDeletion(!m_limited);
}

void DeviceSummaryWidget::updateFieldsValue()
{
    if (m_data){
        ui->lblTitle->setText(m_data->getName());

        // Status
        ui->icoOnline->setVisible(m_data->isEnabled());

        ui->icoTitle->setPixmap(QPixmap(m_data->getIconStateFilename()));
        if (m_data->isOnline() || m_data->isBusy()){
            ui->icoOnline->setPixmap(QPixmap("://status/status_online.png"));
        }else{
            ui->icoOnline->setPixmap(QPixmap("://status/status_offline.png"));
        }

        bool onlineable = m_data->getFieldValue("device_onlineable").toBool();
        ui->frameNewSession->setVisible(m_data->isEnabled() && !m_data->isNew() && onlineable);
        if (!onlineable)
            ui->lblInfos->show();

        if (m_data->hasBusyStateField())
            ui->btnNewSession->setEnabled(!m_data->isBusy());

        if (ui->wdgDevice->formHasStructure())
            ui->wdgDevice->fillFormFromData(m_data->toJson());
    }
}

void DeviceSummaryWidget::initUI()
{
    ui->wdgSessions->setComManager(m_comManager);
    ui->wdgSessions->setViewMode(SessionsListWidget::VIEW_DEVICE_SESSIONS, m_data->getUuid(), m_data->getId());

    ui->cmbSessionType->setItemDelegate(new QStyledItemDelegate(ui->cmbSessionType));
    ui->wdgDevice->hideFields(QStringList() << "device_token");
    ui->lblInfos->hide();

    ui->tabNav->setCurrentIndex(0);

    // Access log widget init
    ui->wdgLogins->setComManager(m_comManager);
    ui->wdgLogins->setViewMode(LogViewWidget::VIEW_LOGINS_DEVICE, m_data->getUuid());
    ui->wdgLogins->setUuidName(m_data->getUuid(), m_data->getName());
}

bool DeviceSummaryWidget::validateData()
{
    return ui->wdgDevice->validateFormData();
}

void DeviceSummaryWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type.startsWith(WEB_FORMS_QUERY_DEVICE)){
        if (!ui->wdgDevice->formHasStructure()){
            ui->wdgDevice->buildUiFromStructure(data);
        }
        return;
    }
}

void DeviceSummaryWidget::processSiteAccessReply(QList<TeraData> access_list, QUrlQuery reply_query)
{
    // Check if the current logged user is admin in at least one of the device site
    bool is_admin = false;
    for(const TeraData &access: access_list){
        if(m_comManager->isCurrentUserSiteAdmin(access.getFieldValue("id_site").toInt())){
            is_admin = true;
            break;
        }
    }

    setLimited(!is_admin);
}

void DeviceSummaryWidget::saveData(bool signal)
{
    QJsonDocument device_data = ui->wdgDevice->getFormDataJson(m_data->isNew());
    postDataRequest(WEB_DEVICEINFO_PATH, device_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgDevice->getFormDataObject(TERADATA_DEVICE);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void DeviceSummaryWidget::processSessionTypesReply(QList<TeraData> session_types)
{
    ui->cmbSessionType->clear();

    for (const TeraData &st:session_types){
        if (!m_ids_session_types.contains(st.getId())){
            m_ids_session_types[st.getId()] = new TeraData(st);
            // New session ComboBox
            ui->cmbSessionType->addItem(st.getName(), st.getId());
        }else{
            *m_ids_session_types[st.getId()] = st;
        }
    }

    // Query sessions
    ui->wdgSessions->setSessionTypes(session_types);

}

void DeviceSummaryWidget::processDevicesReply(QList<TeraData> devices)
{
    for (int i=0; i<devices.count(); i++){
        if (devices.at(i) == *m_data){           
            // We found "ourself" in the list - update data.
            m_data->updateFrom(devices.at(i));
            updateFieldsValue();
            break;
        }
    }
}

void DeviceSummaryWidget::processStatsReply(TeraData stats, QUrlQuery reply_query)
{

    // Check if stats are for us
    if (!reply_query.hasQueryItem("id_device"))
        return;

    if (reply_query.queryItemValue("id_device").toInt() != m_data->getId())
        return;

    // Fill stats information
    int total_sessions = stats.getFieldValue("sessions_total_count").toInt();
    ui->wdgSessions->setSessionsCount(total_sessions);

    // Query sessions types
    QUrlQuery args;
   /* if (m_idProject > 0)
        args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_idProject));*/
    queryDataRequest(WEB_SESSIONTYPE_PATH, args);
}

void DeviceSummaryWidget::ws_deviceEvent(DeviceEvent event)
{
    if (!m_data)
        return;

    if (QString::fromStdString(event.device_uuid()) != m_data->getUuid())
        return; // Not for us!

    if (event.type() == DeviceEvent_EventType_DEVICE_CONNECTED){
        m_data->setOnline(true);
    }
    if (event.type() == DeviceEvent_EventType_DEVICE_DISCONNECTED){
        m_data->setOnline(false);
    }
    if (event.type() == DeviceEvent_EventType_DEVICE_JOINED_SESSION){
        m_data->setBusy(true);
    }
    if (event.type() == DeviceEvent_EventType_DEVICE_LEFT_SESSION){
        m_data->setBusy(false);
    }
    updateFieldsValue();
}
#ifndef OPENTERA_WEBASSEMBLY
void DeviceSummaryWidget::sessionLobbyStartSessionRequested()
{
    int id_session_type = ui->cmbSessionType->currentData().toInt();
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

void DeviceSummaryWidget::sessionLobbyStartSessionCancelled()
{
    if (m_sessionLobby){
        m_sessionLobby->deleteLater();
        m_sessionLobby = nullptr;
    }
}

void DeviceSummaryWidget::on_btnNewSession_clicked()
{
    if (ui->cmbSessionType->currentIndex() < 0)
        return;

    if (!m_data)
        return;

    int id_session_type = ui->cmbSessionType->currentData().toInt();

    if (m_sessionLobby)
        m_sessionLobby->deleteLater();

    m_sessionLobby = new SessionLobbyDialog(m_comManager, *m_ids_session_types[id_session_type], -1, 0, this);

    // Add current devices to session
    m_sessionLobby->addUsersToSession(QList<TeraData>() << m_comManager->getCurrentUser(), QList<int>() << m_comManager->getCurrentUser().getId());
	m_sessionLobby->addDevicesToSession(QList<TeraData>() << *m_data, QList<int>() << m_data->getId());

    connect(m_sessionLobby, &QDialog::accepted, this, &DeviceSummaryWidget::sessionLobbyStartSessionRequested);
    connect(m_sessionLobby, &QDialog::rejected, this, &DeviceSummaryWidget::sessionLobbyStartSessionCancelled);

    // Show Session Lobby
    m_sessionLobby->exec();
}
#endif
void DeviceSummaryWidget::on_tabNav_currentChanged(int index)
{
    Q_UNUSED(index)
    if (ui->tabNav->currentWidget() == ui->tabLogs){
        ui->wdgLogins->refreshData();
    }
}

void DeviceSummaryWidget::sessionTotalCountUpdated(int new_count)
{
    ui->grpSessions->setTitle(tr("Séances") + " ( " + QString::number(new_count) + " )");
}

