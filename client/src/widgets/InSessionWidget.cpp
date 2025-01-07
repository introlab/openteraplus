#include "InSessionWidget.h"
#include "widgets/SessionInviteWidget.h"
#include "ui_InSessionWidget.h"

#include "TeraSettings.h"
#include <QStandardPaths>
#include <QFileDialog>

InSessionWidget::InSessionWidget(ComManager *comMan, const TeraData* session_type, const int id_session, const int id_project, JoinSessionEvent* initial_event, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InSessionWidget),
    m_sessionType(*session_type),
    m_projectId(id_project)
{
    ui->setupUi(this);

    m_comManager = comMan;
    m_serviceWidget = nullptr;
    m_serviceToolsWidget = nullptr;
    m_startDiag = nullptr;
    m_session = nullptr;

    // Do we have an initial event (i.e. session invitation)?
    setPendingEvent(initial_event);

    m_sessionTimer.setInterval(1000);
    m_sessionTimer.setSingleShot(false);

    connectSignals();

    //Query session information
    if (id_session >0){
        setSessionId(id_session);
    }

    queryLists();

    initUI();

}

InSessionWidget::~InSessionWidget()
{
    delete ui;
    if (m_session)
        delete m_session;
    if (m_serviceWidget)
        m_serviceWidget->deleteLater();
    if (m_startDiag)
        m_startDiag->deleteLater();
    if (m_serviceToolsWidget)
        m_serviceToolsWidget->deleteLater();
}

void InSessionWidget::disconnectSignals()
{
    disconnect(m_comManager, nullptr, this, nullptr);
}

void InSessionWidget::setSessionId(int session_id)
{
    // Create temporary object
    m_session = new TeraData(TERADATA_SESSION);
    m_session->setId(session_id);

    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(session_id));
    m_comManager->doGet(WEB_SESSIONINFO_PATH, args);
}

void InSessionWidget::setPendingEvent(JoinSessionEvent *event)
{
    if (event != nullptr){
        m_pendingEvent = new JoinSessionEvent(*event);
    }else{
        m_pendingEvent = nullptr;
    }
}

bool InSessionWidget::sessionCanBeEnded()
{
    if (m_serviceToolsWidget){
        return m_serviceToolsWidget->sessionCanBeEnded(false);
    }
    return true;
}

void InSessionWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (!m_session){
        showStartSessionDiag(tr("Démarrage de séance en cours..."));
    }

}

void InSessionWidget::showStartSessionDiag(const QString &msg)
{
    if (m_startDiag){
        m_startDiag->deleteLater();
    }

    m_startDiag = new StartSessionDialog(msg, m_comManager);
    m_startDiag->setModal(true);
    connect(m_startDiag, &StartSessionDialog::timeout, this, &InSessionWidget::startSessionDiagTimeout);
    connect(m_startDiag, &StartSessionDialog::accepted, this, &InSessionWidget::startSessionDiagAccepted);
    connect(m_startDiag, &StartSessionDialog::rejected, this, &InSessionWidget::startSessionDiagRejected);
    //diag.exec();
    m_startDiag->show();
}

void InSessionWidget::startSessionDiagTimeout()
{
    GlobalMessageBox msg_box;
    msg_box.showError(tr("Délai expiré"), tr("L'opération n'a pu être complétée. Veuillez réessayer à nouveau."));
}

void InSessionWidget::startSessionDiagAccepted()
{
    m_startDiag->deleteLater();
    m_startDiag = nullptr;
}

void InSessionWidget::startSessionDiagRejected()
{
    m_startDiag->deleteLater();
    m_startDiag = nullptr;
    emit sessionEndedWithError();
}

void InSessionWidget::newSessionInviteesRequested(QStringList user_uuids, QStringList participant_uuids, QStringList device_uuids)
{
    if (!m_session)
        return;

    QJsonDocument document;
    QJsonObject base_obj;

    QJsonObject item_obj;
    item_obj.insert("id_session", m_session->getId());
    item_obj.insert("id_service", m_sessionType.getFieldValue("id_service").toInt());
    item_obj.insert("action", "invite");

    // Devices
    if (!device_uuids.isEmpty()){
        QJsonArray devices;
        for(const QString &device_uuid:device_uuids){
            devices.append(QJsonValue(device_uuid));
        }
        item_obj.insert("session_devices", devices);
    }

    // Participants
    if (!participant_uuids.isEmpty()){
        QJsonArray participants;
        for(const QString &part_uuid:participant_uuids){
            participants.append(QJsonValue(part_uuid));
        }
        item_obj.insert("session_participants", participants);
    }

    if (!user_uuids.isEmpty()){
        QJsonArray users;
        for(const QString &user_uuid:user_uuids){
            users.append(QJsonValue(user_uuid));
        }
        item_obj.insert("session_users", users);
    }

    // Update query
    base_obj.insert("session_manage", item_obj);
    document.setObject(base_obj);
    m_comManager->doPost(WEB_SESSIONMANAGER_PATH, document.toJson());
}

void InSessionWidget::removeSessionInviteesRequested(QStringList user_uuids, QStringList participant_uuids, QStringList device_uuids)
{
    if (!m_session)
        return;

    QJsonDocument document;
    QJsonObject base_obj;

    QJsonObject item_obj;
    item_obj.insert("id_session", m_session->getId());
    item_obj.insert("id_service", m_sessionType.getFieldValue("id_service").toInt());
    item_obj.insert("action", "remove");

    // Devices
    if (!device_uuids.isEmpty()){
        QJsonArray devices;
        for(const QString &device_uuid:device_uuids){
            devices.append(QJsonValue(device_uuid));
        }
        item_obj.insert("session_devices", devices);
    }

    // Participants
    if (!participant_uuids.isEmpty()){
        QJsonArray participants;
        for(const QString &part_uuid:participant_uuids){
            participants.append(QJsonValue(part_uuid));
        }
        item_obj.insert("session_participants", participants);
    }

    if (!user_uuids.isEmpty()){
        QJsonArray users;
        for(const QString &user_uuid:user_uuids){
            users.append(QJsonValue(user_uuid));
        }
        item_obj.insert("session_users", users);
    }

    // Update query
    base_obj.insert("session_manage", item_obj);
    document.setObject(base_obj);
    m_comManager->doPost(WEB_SESSIONMANAGER_PATH, document.toJson());
}

void InSessionWidget::sessionTimer()
{
    m_sessionDuration = m_sessionDuration.addSecs(1);
    ui->lblTimer->setText(m_sessionDuration.toString("hh:mm:ss"));
}

void InSessionWidget::setReadyState(bool state)
{
    ui->btnEndSession->setEnabled(state);
    ui->btnLeaveSession->setEnabled(state);
    ui->btnInSessionInfos->setEnabled(state);

    if (m_serviceToolsWidget)
        m_serviceToolsWidget->setReadyState(state);
}


void InSessionWidget::on_btnEndSession_clicked()
{
    GlobalMessageBox msg_box;
    if (msg_box.showYesNo(tr("Terminer la séance?"), tr("Mettre fin à la séance?")) == QMessageBox::Yes){
        if (m_serviceToolsWidget){
            if (!m_serviceToolsWidget->sessionCanBeEnded()){
                return;
            }
        }
        int id_service = 0;
        if (getSessionTypeCategory() == TeraSessionCategory::SESSION_TYPE_SERVICE){
            id_service = m_sessionType.getFieldValue("id_service").toInt();
        }
        if (m_session){
            m_comManager->stopSession(*m_session, id_service);
        }else{
            LOG_CRITICAL("No session! Can't stop it!", "InSessionWidget::on_btnEndSession_clicked");
        }
        showStartSessionDiag(tr("Arrêt de la séance en cours..."));
    }
}

void InSessionWidget::on_btnInSessionInfos_toggled(bool checked)
{
    int mainWidth = ui->widgetMain->width();
    ui->tabInfos->setVisible(checked);
    if (checked){
        ui->widgetMain->setMaximumWidth(mainWidth - ui->tabInfos->width());
    }else{
        ui->widgetMain->setMaximumWidth(mainWidth + ui->tabInfos->width());
    }
}

void InSessionWidget::processSessionsReply(QList<TeraData> sessions)
{
    for(const TeraData &session:sessions){
        if (session.getId() == m_session->getId()){
            // This is an update to the session information we have
            delete m_session;
            m_session = new TeraData(session);

            ui->wdgInvitees->setCurrentSessionUuid(m_session->getUuid());

            // Get participants, users and devices list, and mark them as "required" and invited
            // Lock everyone that was initially in the session? Is that OK??
            // SB - Only lock current user, allow for more flexibility...
            QVariantList item_list;

           /*if (session.hasFieldName("session_participants")){
                item_list = session.getFieldValue("session_participants").toList();

                for(const QVariant &session_part:std::as_const(item_list)){
                    QVariantMap part_info = session_part.toMap();
                    ui->wdgInvitees->addRequiredParticipant(part_info["id_participant"].toInt());
                }
            }*/

            if (session.hasFieldName("session_users")){
                item_list = session.getFieldValue("session_users").toList();

                for(const QVariant &session_user:std::as_const(item_list)){
                    QVariantMap user_info = session_user.toMap();
                    int id_user = user_info["id_user"].toInt();
                    if (id_user == m_comManager->getCurrentUser().getId())
                        ui->wdgInvitees->addRequiredUser(id_user);
                }
            }

            /*if (session.hasFieldName("session_devices")){
                item_list = session.getFieldValue("session_devices").toList();

                for(const QVariant &session_device:std::as_const(item_list)){
                    QVariantMap device_info = session_device.toMap();
                    ui->wdgInvitees->addRequiredDevice(device_info["id_device"].toInt());
                }
            }*/
            ui->wdgInvitees->selectDefaultFilter();


            updateUI();

            // Check if we already have a pending event
            if (m_pendingEvent){
                ws_JoinSessionEvent(*m_pendingEvent);
                delete m_pendingEvent;
                m_pendingEvent = nullptr;
            }
        }
    }
}

void InSessionWidget::processDevicesReply(QList<TeraData> devices)
{
    if (!ui->wdgInvitees->hasAvailableDevices()){
        ui->wdgInvitees->setAvailableDevices(devices);
    }
}

void InSessionWidget::processUsersReply(QList<TeraData> users)
{
    if (!ui->wdgInvitees->hasAvailableUsers()){
        ui->wdgInvitees->setAvailableUsers(users);
    }
}

void InSessionWidget::processParticipantsReply(QList<TeraData> participants)
{
    if (!ui->wdgInvitees->hasAvailableParticipants()){
        ui->wdgInvitees->setAvailableParticipants(participants);
    }
}

void InSessionWidget::ws_JoinSessionEvent(JoinSessionEvent event)
{
    QString session_uuid = QString::fromStdString(event.session_uuid());

    // Check if that event is really for us
    /*if (!m_session){
        LOG_ERROR("Received JoinSessionEvent, but no current session!", "InSessionWidget::processJoinSessionEvent");
        m_comManager->sendJoinSessionReply(session_uuid, JoinSessionReplyEvent::REPLY_DENIED, tr("En attente de démarrage de séance..."));
        return;
    }*/

    if (m_session){
        if (m_session->hasUuidField()){
            if (m_session->getUuid() != session_uuid){
                LOG_WARNING("Received JoinSessionEvent, but it's not for current session - replying busy", "InSessionWidget::processJoinSessionEvent");
                m_comManager->sendJoinSessionReply(session_uuid, JoinSessionReplyEvent::REPLY_BUSY, tr("Déjà en séance"));
                showNotification(NotificationWindow::TYPE_WARNING, QString::fromStdString(event.session_creator_name())
                                             + tr(" vous a invité dans une séance, mais nous avons refusé l'invitation pour vous."),
                                             "://icons/warning.png");
                return;
            }
        }
    }

    // Forward to widget
    if (m_serviceWidget){
        // SB Qt6 WebEngineView seems to cause issue when loading / displaying... Bug?
        //    This patch this behaviour and ensures that the session always starts in maximized mode

        // SB - Doesn't seem to be needed anymore with Qt 6.7+
        /*QWidget* parent = parentWidget();
        while(parent){
            parent = parent->parentWidget();
            if (QString::fromStdString(parent->metaObject()->className()) == "MainWindow"){
                parent->showNormal();
                parent->showMaximized();
                break;
            }
        }*/
        bool result = m_serviceWidget->handleJoinSessionEvent(event);
        if (result){
            // If we have a result here, it's that the join was accepted for the first time.
            m_comManager->sendJoinSessionReply(session_uuid, JoinSessionReplyEvent::REPLY_ACCEPTED);
        }
        // Don't reply anything here, since it might just be an update to the session (new invitees joined in).
        /*else{
            m_comManager->sendJoinSessionReply(session_uuid, JoinSessionReplyEvent::REPLY_DENIED);
        }*/
    }

}

void InSessionWidget::ws_JoinSessionReplyEvent(JoinSessionReplyEvent event)
{
    if (!m_session){
        LOG_ERROR("Received JoinSessionReplyEvent, but no session info!", "InSessionWidget::ws_JoinSessionReplyEvent");
        return;
    }

    if (QString::fromStdString(event.session_uuid()) != m_session->getUuid()){
        LOG_WARNING("Received JoinSessionReplyEvent, but not for the current session", "InSessionWidget::ws_JoinSessionReplyEvent");
        return;
    }

    if (event.join_reply() != JoinSessionReplyEvent::REPLY_ACCEPTED){

        TeraData* invitee_info = nullptr;
        QString action = tr("n'a pas répondu à");
        if (!event.user_uuid().empty()){
            invitee_info = ui->wdgInvitees->getUserFromUuid(QString::fromStdString(event.user_uuid()));
        }

        if (!event.participant_uuid().empty()){
            invitee_info = ui->wdgInvitees->getParticipantFromUuid(QString::fromStdString(event.participant_uuid()));
        }

        if (!event.device_uuid().empty()){
            invitee_info = ui->wdgInvitees->getDeviceFromUuid(QString::fromStdString(event.device_uuid()));
        }

        if (event.join_reply() == JoinSessionReplyEvent::REPLY_DENIED){
            action = tr("a refusé");
        }

        if (event.join_reply() == JoinSessionReplyEvent::REPLY_BUSY){
            action = tr("est occupé et ne peut répondre à");
        }

        if (invitee_info){
            // Show notification with message
            QString notify_text = invitee_info->getName() + " " + action + tr(" l'invitation.");
            if (!event.reply_msg().empty()){
                notify_text += "\n" + tr("Raison: ") + QString::fromStdString(event.reply_msg());
            }
            showNotification(NotificationWindow::TYPE_MESSAGE, notify_text, TeraData::getIconFilenameForDataType(invitee_info->getDataType()));
        }
    }
}

void InSessionWidget::ws_StopSessionEvent(StopSessionEvent event)
{
    if (QString::fromStdString(event.session_uuid()) != m_session->getUuid()){
        LOG_WARNING("Received StopSessionEvent, but not for the current session", "InSessionWidget::ws_StopSessionEvent");
        return;
    }

    if (m_session){
        m_comManager->leaveSession(m_session->getId(), false);
    }
}

void InSessionWidget::connectSignals()
{
    connect(m_comManager, &ComManager::sessionsReceived, this, &InSessionWidget::processSessionsReply);
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::joinSessionEventReceived, this, &InSessionWidget::ws_JoinSessionEvent);
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::joinSessionReplyEventReceived, this, &InSessionWidget::ws_JoinSessionReplyEvent);
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::stopSessionEventReceived, this, &InSessionWidget::ws_StopSessionEvent);

    connect(m_comManager, &ComManager::usersReceived, this, &InSessionWidget::processUsersReply);
    connect(m_comManager, &ComManager::participantsReceived, this, &InSessionWidget::processParticipantsReply);
    connect(m_comManager, &ComManager::devicesReceived, this, &InSessionWidget::processDevicesReply);

    connect(&m_sessionTimer, &QTimer::timeout, this, &InSessionWidget::sessionTimer);

    connect(ui->wdgInvitees, &SessionInviteWidget::newInvitees, this, &InSessionWidget::newSessionInviteesRequested);
    connect(ui->wdgInvitees, &SessionInviteWidget::removedInvitees, this, &InSessionWidget::removeSessionInviteesRequested);
}

void InSessionWidget::initUI()
{
    ui->btnEndSession->hide();
    ui->grpSavePath->hide();

    ui->wdgInvitees->setConfirmOnRemove(true);
    ui->wdgInvitees->setComManager(m_comManager);
    //ui->wdgInvitees->showAvailableInvitees(true);

    ui->btnInSessionInfos->setChecked(false);
    ui->tabInfos->hide();
    ui->tabInfos->setCurrentIndex(0);

    // Clean up, if needed
    if (m_serviceWidget){
        m_serviceWidget->deleteLater();
        m_serviceWidget = nullptr;
    }

    // Get session type category
    TeraSessionCategory::SessionTypeCategories category = getSessionTypeCategory();

    switch(category){
    case TeraSessionCategory::SESSION_TYPE_SERVICE:{
        // Get service key to load the proper ui
        QString service_key = m_sessionType.getFieldValue("session_type_service_key").toString();
        bool handled = false;
        if (service_key == "VideoRehabService"){
            // Main widget = QWebEngine
            m_serviceWidget = new VideoRehabWidget(m_comManager);
            setMainWidget(m_serviceWidget);
            m_serviceToolsWidget = new VideoRehabToolsWidget(m_comManager, m_serviceWidget, this);
            setToolsWidget(m_serviceToolsWidget);
            connect(dynamic_cast<VideoRehabWidget*>(m_serviceWidget), &VideoRehabWidget::fileDownloading,
                    dynamic_cast<VideoRehabToolsWidget*>(m_serviceToolsWidget), &VideoRehabToolsWidget::setFileDownloading);

            QString service_config = m_sessionType.getFieldValue("session_type_config").toString();
            QJsonDocument doc = QJsonDocument::fromJson(service_config.toUtf8());
            if (!doc.isNull()){
                if (doc.object().contains("session_recordable")){
                    if (doc.object()["session_recordable"].toBool()){
                        QString path = TeraSettings::getUserSetting(m_comManager->getCurrentUser().getUuid(), SETTINGS_DATA_SAVEPATH).toString();
                        ui->txtSavePath->setText(path);
                        ui->grpSavePath->show();
                    }else{
                        ui->grpSavePath->hide();
                    }
                }
            }
            handled = true;
        }

        //DL - Lazy programmer reusing VideoRehabService widget!
        if (service_key == "RobotTeleOperationService") {
            // Main widget = QWebEngine
            m_serviceWidget = new VideoRehabWidget(m_comManager, this);
            setMainWidget(m_serviceWidget);
            m_serviceToolsWidget = new VideoRehabToolsWidget(m_comManager, m_serviceWidget, this);
            setToolsWidget(m_serviceToolsWidget);
            handled = true;
        }

        if (!handled){
            GlobalMessageBox msg_box;
            msg_box.showWarning(tr("Service non-supporté"), tr("Le service \"") + service_key + tr("\" n'est pas gérée par cette version du logiciel.\n\nVeuillez vérifier si une mise à jour existe ou contribuez au développement du logiciel!"));
        }else{
            // Connect signals between tools and main widget
            connect(m_serviceWidget, &BaseServiceWidget::widgetIsReady, this, &InSessionWidget::setReadyState);
        }

        break;
    }
    default:
        GlobalMessageBox msg_box;
        msg_box.showWarning(tr("Catégorie de séance non-supportée"), tr("La catégorie de séance \"") + TeraSessionCategory::getSessionTypeCategoryName(category) + tr("\" n'est pas gérée par cette version du logiciel.\n\nVeuillez vérifier si une mise à jour existe ou contribuez au développement du logiciel!"));
    }
}

void InSessionWidget::updateUI()
{
    if (m_session){
        ui->lblSessionName->setText(m_session->getName());
        if (!m_sessionTimer.isActive()){ // Session just started
            // Update session timer initial value
            int current_duration = m_session->getFieldValue("session_duration").toInt()-1; // -1 since we are going to increment it to display it with the timer's slot
            m_sessionDuration = QTime(0,0).addSecs(current_duration);

            // Display initial value
            sessionTimer();

            // Start timer
            m_sessionTimer.start();

            // Hide manager frame
            ui->btnInSessionInfos->setChecked(false);
        }
        bool user_is_creator = m_session->getFieldValue("id_creator_user").toInt() == m_comManager->getCurrentUser().getId();
        ui->btnEndSession->setVisible(user_is_creator);
        ui->btnLeaveSession->setVisible(!user_is_creator);

        if (m_serviceWidget){
            m_serviceWidget->setSession(m_session);
        }
    }
}

void InSessionWidget::setMainWidget(QWidget *wdg)
{
    // Check for layout
    if (!ui->widgetMain->layout()){
        // No existing layout - create one
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        ui->widgetMain->setLayout(layout);
    }

    ui->widgetMain->layout()->addWidget(wdg);

}

void InSessionWidget::setToolsWidget(QWidget *wdg)
{
    // Check for layout
    if (!ui->widgetTools->layout()){
        // No existing layout - create one
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        ui->widgetTools->setLayout(layout);
    }

    ui->widgetTools->layout()->addWidget(wdg);
}

TeraSessionCategory::SessionTypeCategories InSessionWidget::getSessionTypeCategory()
{
    return static_cast<TeraSessionCategory::SessionTypeCategories>(m_sessionType.getFieldValue("session_type_category").toInt());
}

void InSessionWidget::queryLists()
{
    // Query available participants, devices and users list
    QUrlQuery args;
    if (m_projectId>0)
        args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_projectId));
    args.addQueryItem(WEB_QUERY_ENABLED, "1");
    args.addQueryItem(WEB_QUERY_LIST, "1");
    // args.addQueryItem(WEB_QUERY_WITH_STATUS, "1");
    m_comManager->doGet(WEB_USERINFO_PATH, args);
    m_comManager->doGet(WEB_PARTICIPANTINFO_PATH, args);
    m_comManager->doGet(WEB_DEVICEINFO_PATH, args);


}

void InSessionWidget::showNotification(const NotificationWindow::NotificationType notification_type, const QString &text, const QString &iconPath)
{
    emit requestShowNotification(notification_type, text, iconPath, QString(), 300, 75, 5000);
}


void InSessionWidget::on_btnLeaveSession_clicked()
{
    GlobalMessageBox msg_box;
    if (msg_box.showYesNo(tr("Quitter la séance?"), tr("Désirez-vous quitter la séance?")) == QMessageBox::Yes){
        if (m_session){
            m_comManager->leaveSession(m_session->getId());
        }else{
            LOG_CRITICAL("No session! Can't leave it!", "InSessionWidget::on_btnLeaveSession_clicked");
        }
    }
}

void InSessionWidget::on_btnDefaultPath_clicked()
{
    QStringList documents_path = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    QString download_path = "";
    if (!documents_path.empty())
        download_path = documents_path.first();
    download_path += "/OpenTeraPlus/downloads/";
    // Save path as default
    TeraSettings::setUserSetting(m_comManager->getCurrentUser().getUuid(), SETTINGS_DATA_SAVEPATH, download_path);

    ui->txtSavePath->setText(download_path);

    if (m_serviceWidget)
        m_serviceWidget->setDataSavePath();
}


void InSessionWidget::on_btnBrowseSavePath_clicked()
{
    QFileDialog diag;
    QString full_path = diag.getExistingDirectory(this, tr("Répertoire où les données seront enregistrées"));

    if (!full_path.isEmpty()){
        TeraSettings::setUserSetting(m_comManager->getCurrentUser().getUuid(), SETTINGS_DATA_SAVEPATH, full_path);

        ui->txtSavePath->setText(full_path);

        if (m_serviceWidget)
            m_serviceWidget->setDataSavePath();
    }

}

