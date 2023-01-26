#include "MainWindow.h"
#include <QNetworkReply>
#include <QDesktopWidget>
#include <QApplication>
#include <QSound>

#include "ui_MainWindow.h"

#include "editors/SiteWidget.h"
#include "editors/ProjectWidget.h"
#include "editors/GroupWidget.h"
#include "editors/ParticipantWidget.h"
#include "editors/UserSummaryWidget.h"
#include "editors/DeviceSummaryWidget.h"


MainWindow::MainWindow(ComManager *com_manager, const QString &current_server, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground);

    m_comManager = com_manager;
    m_diag_editor = nullptr;
    m_data_editor = nullptr;
    m_dashboard = nullptr;
    m_inSessionWidget = nullptr;
    m_download_dialog = nullptr;
    m_joinSession_dialog = nullptr;
    m_currentLanguage = m_comManager->getCurrentPreferences().getLanguage();
    m_waiting_for_data_type = TERADATA_NONE;
    m_currentDataType = TERADATA_NONE;
    m_currentDataId = -1;

    // Add current server name to window title
    if (!current_server.isEmpty())
        setWindowTitle(windowTitle() + " - " + current_server);

    // Initial UI state
    initUi();

    // Connect signals
    connectSignals();

    // Update user in case we already have it
    updateCurrentUser();

    // Show dashboard
    //showDashboard(true);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_loadingIcon;
    if (m_data_editor)
        m_data_editor->deleteLater();

    if (!m_notifications.isEmpty())
        qDeleteAll(m_notifications);

}

bool MainWindow::isInSession()
{
    return m_inSessionWidget != nullptr;
}

void MainWindow::connectSignals()
{
    connect(m_comManager, &ComManager::currentUserUpdated, this, &MainWindow::updateCurrentUser);
    connect(m_comManager, &ComManager::preferencesUpdated, this, &MainWindow::com_preferencesUpdated);

    connect(m_comManager, &ComManager::networkError, this, &MainWindow::com_networkError);
    connect(m_comManager, &ComManager::socketError, this, &MainWindow::com_socketError);
    connect(m_comManager, &ComManager::waitingForReply, this, &MainWindow::com_waitingForReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &MainWindow::com_postReplyOK);
    connect(m_comManager, &ComManager::dataReceived, this, &MainWindow::processGenericDataReply);
    connect(m_comManager, &ComManager::deleteResultsOK, this, &MainWindow::com_deleteResultsOK);
    connect(m_comManager, &ComManager::downloadProgress, this, &MainWindow::com_downloadProgress);
    connect(m_comManager, &ComManager::downloadCompleted, this, &MainWindow::com_downloadCompleted);

    connect(m_comManager, &ComManager::sessionStarted, this, &MainWindow::com_sessionStarted);
    connect(m_comManager, &ComManager::sessionStopped, this, &MainWindow::com_sessionStopped);
    connect(m_comManager, &ComManager::sessionError, this, &MainWindow::com_sessionError);

    connect(m_comManager, &ComManager::sessionStartRequested, this, &MainWindow::com_sessionStartRequested);
    connect(m_comManager, &ComManager::posting, this, &MainWindow::com_posting);
    connect(m_comManager, &ComManager::querying, this, &MainWindow::com_querying);
    connect(m_comManager, &ComManager::deleting, this, &MainWindow::com_deleting);

    connect(m_comManager->getWebSocketManager(), &WebSocketManager::userEventReceived, this, &MainWindow::ws_userEvent);
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::participantEventReceived, this, &MainWindow::ws_participantEvent);
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::joinSessionEventReceived, this, &MainWindow::ws_joinSessionEvent);

    connect(ui->projNavigator, &ProjectNavigator::dataDisplayRequest, this, &MainWindow::dataDisplayRequested);
    connect(ui->projNavigator, &ProjectNavigator::dataDeleteRequest, this, &MainWindow::dataDeleteRequested);
    connect(ui->projNavigator, &ProjectNavigator::currentSiteWasChanged, this, &MainWindow::currentSiteChanged);
    connect(ui->projNavigator, &ProjectNavigator::refreshButtonClicked, this, &MainWindow::dataRefreshRequested);

    connect(ui->onlineManager, &OnlineManagerWidget::dataDisplayRequest, this, &MainWindow::dataDisplayRequestedByUuid);
    connect(ui->onlineManager, &OnlineManagerWidget::totalCountUpdated, this, &MainWindow::updateOnlineCounts);

    connect(GlobalEventLogger::instance(), &GlobalEventLogger::newEventLogged, this, &MainWindow::addGlobalEvent);

    connect(ui->wdgMessages, &ResultMessageWidget::nextMessageShown, this, &MainWindow::nextMessageWasShown);
}

void MainWindow::initUi()
{
    ui->btnConfig->hide();

    // Setup messages
    ui->wdgMessages->hide();
    ui->frameCentralBottom->hide();

    // Setup event view
    ui->tableHistory->setColumnWidth(0, 20);
    ui->btnLog->hide(); // Disabled for now

    // Disable docker titles
    ui->dockerTop->setTitleBarWidget(new QWidget());
    ui->dockerLeft->setTitleBarWidget(new QWidget());
    ui->dockerRight->hide();

    // Setup loading icon animation
    m_loadingIcon = new QMovie("://status/loading.gif");
    ui->icoLoading->setMovie(m_loadingIcon);
    ui->icoLoading->hide();

    // Setup main menu
    ui->projNavigator->setComManager(m_comManager);
    ui->onlineManager->setComManager(m_comManager);
    ui->tabMainMenu->setCurrentIndex(0); // Select "Navigator" tab by default

    // Set version number
    ui->lblVersion->setText(QString(OPENTERAPLUS_VERSION));

    // Disable incomplete features
    ui->tabMainMenu->removeTab(2); // Search tab

}

void MainWindow::showDataEditor(const TeraDataTypes &data_type, const TeraData*data)
{
    if (m_data_editor){
        m_data_editor->disconnectSignals();
        m_data_editor->deleteLater();
        m_data_editor = nullptr;
    }

    if (data_type == TERADATA_NONE || data == nullptr){
        m_currentDataType = data_type;
        m_currentDataId = -1;
        showDashboard(true);
        return;
    }

    showDashboard(false);

    // Save values to display them again if needed
    m_currentDataType = data_type;
    m_currentDataId = data->getId();


    if (data_type == TERADATA_SITE){
        m_data_editor = new SiteWidget(m_comManager, data);
        if (!data->isNew())
            m_data_editor->setLimited(!(m_comManager->isCurrentUserSiteAdmin(data->getId())) && data->getId()>0);
    }

    if (data_type == TERADATA_PROJECT){
        m_data_editor = new ProjectWidget(m_comManager, data);
        if (!data->isNew())
            m_data_editor->setLimited(!(m_comManager->isCurrentUserProjectAdmin(data->getId())) && data->getId()>0);
    }

    if (data_type == TERADATA_GROUP){
        m_data_editor = new GroupWidget(m_comManager, data);
        bool limited = false;
        if (data->hasFieldName("id_project")){
            limited = !(m_comManager->isCurrentUserProjectAdmin(data->getFieldValue("id_project").toInt()));
        }
        if (!data->isNew())
            m_data_editor->setLimited(limited);
    }

    if (data_type == TERADATA_PARTICIPANT){
        m_data_editor = new ParticipantWidget(m_comManager, data);
        // Don't limit editing for project admins
        /*bool limited = true;
        if (data->hasFieldName("id_project")){
            limited = !(m_comManager->isCurrentUserProjectAdmin(data->getFieldValue("id_project").toInt())); //m_comManager->getCurrentUserProjectRole(data->getFieldValue("id_project").toInt()) != "admin";
        }
        if (data->getId()==0)
            limited = false;
        if (!data->isNew())
            m_data_editor->setLimited(limited);
        */

    }

    if (data_type == TERADATA_USER){
        int id_project = -1;
        if (ui->projNavigator->getCurrentItemType() == TERADATA_USER && ui->projNavigator->getCurrentItemId() == data->getId()){
            id_project = ui->projNavigator->getCurrentProjectId();
        }
        m_data_editor = new UserSummaryWidget(m_comManager, data, id_project);
    }

    if (data_type == TERADATA_DEVICE){
        int id_project = -1;
        if (ui->projNavigator->getCurrentItemType() == TERADATA_DEVICE && ui->projNavigator->getCurrentItemId() == data->getId()){
            id_project = ui->projNavigator->getCurrentProjectId();
        }
        m_data_editor = new DeviceSummaryWidget(m_comManager, data, id_project);
    }

    if (m_data_editor){
        ui->wdgMainTop->layout()->addWidget(m_data_editor);
        connect(m_data_editor, &DataEditorWidget::dataWasDeleted, this, &MainWindow::dataEditorCancelled);
        connect(m_data_editor, &DataEditorWidget::dataDisplayRequest, this, &MainWindow::dataDisplayRequested);
    }else{
        LOG_ERROR("Unhandled data editor: " + TeraData::getPathForDataType(data_type), "MainWindow::showDataEditor");
    }


}

void MainWindow::showDashboard(const bool &show)
{
    if (!show){
        if (m_dashboard){
            m_dashboard->deleteLater();
            m_dashboard = nullptr;
        }
    }else{
        // Remove any pending editor
        if (m_data_editor){
            m_data_editor->disconnectSignals();
            m_data_editor->deleteLater();
            m_data_editor = nullptr;
        }

        if (!m_dashboard){
            m_dashboard = new DashboardWidget(m_comManager, ui->projNavigator->getCurrentSiteId());
            connect(m_dashboard, &DashboardWidget::dataDisplayRequest, this, &MainWindow::dataDisplayRequested);
            ui->wdgMainTop->layout()->addWidget(m_dashboard);
        }else{
            m_dashboard->setCurrentSiteId(ui->projNavigator->getCurrentSiteId());
        }
    }

}

void MainWindow::setInSession(bool in_session, const TeraData *session_type, const int &id_session, int id_project)
{
    if (!in_session && !isInSession())
        return; // No current session, don't stop it!

    if (in_session && isInSession())
        return; // Already in a session, don't start another one

    if (m_data_editor){
        m_data_editor->disconnectSignals();
        ui->wdgMainTop->layout()->removeWidget(m_data_editor);
        m_data_editor->deleteLater();
        m_data_editor = nullptr;
    }

    if (m_dashboard){
        m_dashboard->deleteLater();
        m_dashboard = nullptr;
    }
    if (m_inSessionWidget){
        m_inSessionWidget->disconnectSignals();
        ui->wdgMainTop->layout()->removeWidget(m_inSessionWidget);
        //m_inSessionWidget->deleteLater();
        delete m_inSessionWidget;
        m_inSessionWidget = nullptr;
    }

    ui->dockerLeft->setVisible(!in_session);
    ui->btnLogout->setVisible(!in_session);
    ui->frameCentralBottom->setVisible(!in_session && ui->wdgMessages->hasMessagesWaiting());

    if (in_session){
        if (id_project == 0)
            id_project = ui->projNavigator->getCurrentProjectId();
        m_inSessionWidget = new InSessionWidget(m_comManager, session_type, id_session, id_project);
        connect(m_inSessionWidget, &InSessionWidget::sessionEndedWithError, this, &MainWindow::inSession_sessionEndedWithError);
        connect(m_inSessionWidget, &InSessionWidget::requestShowNotification, this, &MainWindow::addNotification);
        ui->wdgMainTop->layout()->addWidget(m_inSessionWidget);
    }else{
        // Loads back the "previous" data type
        dataDisplayRequested(m_currentDataType, m_currentDataId);
    }
}

QIcon MainWindow::getGlobalEventIcon(GlobalEvent &global_event)
{
    switch(global_event.getEventType()){
        case EVENT_ERROR:
            return QIcon("://icons/error.png");

        case EVENT_LOGIN:
        case EVENT_LOGOUT:
            return QIcon("://icons/user_info.png");

        case EVENT_NETWORK:
            return QIcon("://status/connecting.gif");

        case EVENT_WARNING:
            return QIcon("://icons/warning.png");

        case EVENT_USER_ONLINE:
        case EVENT_PART_ONLINE:
            return QIcon("://status/status_ok.png");

        case EVENT_USER_OFFLINE:
        case EVENT_PART_OFFLINE:
            return QIcon("://status/status_notok.png");

        case EVENT_DATA_NEW:
            return QIcon("://icons/new.png");
        case EVENT_DATA_EDIT:
            return QIcon("://icons/edit.png");
        case EVENT_DATA_DELETE:
            return QIcon("://icons/delete_old.png");
        case EVENT_DATA_QUERY:
            return QIcon("://icons/server.png");
    }

    return QIcon();
}

void MainWindow::nextMessageWasShown(Message current_message)
{
    m_loadingIcon->stop();
    if (current_message.getMessageType() == Message::MESSAGE_NONE){
        ui->wdgMessages->hide();
        //if (m_inSessionWidget){
            // Hide frame when no more message
            ui->frameCentralBottom->hide();
        //}
        return;
    }else{
        // We have a message displayed now
        ui->frameCentralBottom->show();
        ui->wdgMessages->show();
    }
}

void MainWindow::notificationCompleted(NotificationWindow *notify)
{
    m_notifications.removeAll(notify);
    notify->deleteLater();

    for(int i=0; i<m_notifications.count(); i++){
        m_notifications.at(i)->resetPosition(i+1);
    }
}

void MainWindow::currentSiteChanged(QString site_name, int site_id)
{
    ui->onlineManager->setCurrentSite(site_name, site_id);
    if (m_dashboard)
        m_dashboard->setCurrentSiteId(site_id);
    else{
        showDashboard(true);
    }
}

void MainWindow::dataRefreshRequested()
{
    ui->onlineManager->refreshOnlines();
}

void MainWindow::addGlobalEvent(GlobalEvent event)
{
    // Keep max 50 events in the log
    if (ui->tableHistory->rowCount() >= 50){
        //ui->tableHistory->removeRow(ui->tableHistory->rowCount()-1);
        ui->tableHistory->removeRow(0);
        ui->tableHistory->setRowCount(49);
    }

    // Get correct event icon depending on type
    QIcon event_icon = getGlobalEventIcon(event);

    // Create new table items and add to event table
    QTableWidgetItem* icon_item = new QTableWidgetItem(event_icon, "");
    QTableWidgetItem* time_item = new QTableWidgetItem(QTime::currentTime().toString("hh:mm:ss"));
    QTableWidgetItem* desc_item = new QTableWidgetItem(event.getEventText());//new QTableWidgetItem(QTextDocumentFragment::fromHtml(event.getEventText()).toPlainText());
    desc_item->setData(Qt::UserRole, event.getEventText());
    //ui->tableHistory->insertRow(0);
    ui->tableHistory->insertRow(ui->tableHistory->rowCount());
    int current_row = ui->tableHistory->rowCount()-1; //0
    ui->tableHistory->setItem(current_row, 0, icon_item);
    ui->tableHistory->setItem(current_row, 1, time_item);
    ui->tableHistory->setItem(current_row, 2, desc_item);


}

void MainWindow::editorDialogFinished()
{
    m_diag_editor->deleteLater();
    m_diag_editor = nullptr;

    // Enable selection in the project manager
    ui->projNavigator->setOnHold(false);
}

void MainWindow::joinSessionDialogFinished()
{
    if (m_joinSession_dialog->result() == JoinSessionDialog::Accepted){
        m_comManager->sendJoinSessionReply(QString::fromStdString(m_joinSession_dialog->getEvent()->session_uuid()),
                                           m_joinSession_dialog->getSelectedReply());

        if (m_joinSession_dialog->getSelectedReply() == JoinSessionReplyEvent_ReplyType::JoinSessionReplyEvent_ReplyType_REPLY_ACCEPTED){
            // Join session
            TeraData* session = m_joinSession_dialog->getSession();
            int id_project = -1;
            // Find id project of the first participant in the project, if there
            if (session->hasFieldName("session_participants")){
                QVariantList participants = session->getFieldValue("session_participants").toList();
                if (participants.count() > 0){
                    id_project = participants.first().toHash()["id_project"].toInt();
                }
            }
            setInSession(true, m_joinSession_dialog->getSessionType(), m_joinSession_dialog->getSessionId(), id_project);
            m_inSessionWidget->setPendingEvent(m_joinSession_dialog->getEvent());
            m_comManager->joinSession(*m_joinSession_dialog->getSessionType(), m_joinSession_dialog->getSessionId());
        }
    }else{
        // Rejected dialog (because closed manually, for example) always generate a refuse reply.
        m_comManager->sendJoinSessionReply(QString::fromStdString(m_joinSession_dialog->getEvent()->session_uuid()),
                                           JoinSessionReplyEvent_ReplyType::JoinSessionReplyEvent_ReplyType_REPLY_TIMEOUT);
    }

    m_joinSession_dialog->deleteLater();
    m_joinSession_dialog = nullptr;
}

void MainWindow::dataDisplayRequested(TeraDataTypes data_type, int data_id)
{
    if (data_type == TERADATA_NONE){
        // Clear data display
        showDataEditor(TERADATA_NONE, nullptr);
        return;
    }

    if (data_id == 0){
        ui->projNavigator->setEnabled(false);
        TeraData* new_data = new TeraData(data_type);
        new_data->setId(0);

        // Set default values for new data
        if (data_type == TERADATA_PROJECT)
            new_data->setFieldValue("id_site", ui->projNavigator->getCurrentSiteId());

        if (data_type == TERADATA_GROUP){
            new_data->setFieldValue("id_project", ui->projNavigator->getCurrentProjectId());
            new_data->setFieldValue("id_site", ui->projNavigator->getCurrentSiteId());
        }

        if (data_type == TERADATA_PARTICIPANT){
            if (ui->projNavigator->getCurrentGroupId() > 0)
                new_data->setFieldValue("id_participant_group", ui->projNavigator->getCurrentGroupId());
            if (ui->projNavigator->getCurrentProjectId() > 0)
                new_data->setFieldValue("id_project", ui->projNavigator->getCurrentProjectId());
        }

        showDataEditor(data_type, new_data);
        return;
    }else{
        // Try to select in project navigator
        if (data_type == TERADATA_PROJECT || data_type == TERADATA_PARTICIPANT || data_type == TERADATA_GROUP)
            ui->projNavigator->selectItem(data_type, data_id);
    }

    // Set flag to wait for that specific data type
    if (m_waiting_for_data_type != TERADATA_NONE)
        LOG_WARNING("Request for new data for editor, but still waiting on previous one!", "MainWindow::dataDisplayRequested");
    m_waiting_for_data_type = data_type;

    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID, QString::number(data_id));
    if (data_type == TERADATA_USER || data_type == TERADATA_DEVICE){
        // Also query for status
        query.addQueryItem(WEB_QUERY_WITH_STATUS, "1");
    }
    m_comManager->doGet(TeraData::getPathForDataType(data_type), query);

}

void MainWindow::dataDisplayRequestedByUuid(TeraDataTypes data_type, QString data_uuid)
{
    // Try to select in project navigator
    if (data_type == TERADATA_PROJECT || data_type == TERADATA_PARTICIPANT || data_type == TERADATA_GROUP)
        ui->projNavigator->selectItemByUuid(data_type, data_uuid);

    // Request to display a specific item by uuid.
    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_UUID, data_uuid);
    if (/*data_type == TERADATA_PARTICIPANT ||*/ data_type == TERADATA_DEVICE || data_type == TERADATA_USER){
        // Also query for status
        query.addQueryItem(WEB_QUERY_WITH_STATUS, "1");
    }
    m_comManager->doGet(TeraData::getPathForDataType(data_type), query);

    // Set flag to wait for that specific data type
    if (m_waiting_for_data_type != TERADATA_NONE)
        LOG_WARNING("Request for new data for editor, but still waiting on previous one!", "MainWindow::dataDisplayRequested");
    m_waiting_for_data_type = data_type;

}

void MainWindow::dataDeleteRequested(TeraDataTypes data_type, int data_id)
{
    /*if (m_waiting_for_data_type != TERADATA_NONE)
        LOG_WARNING("Request to delete, but still waiting on previous result!", "MainWindow::dataDeleteRequested");
    m_waiting_for_data_type = data_type;*/

    m_comManager->doDelete(TeraData::getPathForDataType(data_type), data_id);
}

void MainWindow::dataEditorCancelled()
{
    if (ui->projNavigator->hasCurrentItem()){
        ui->projNavigator->refreshCurrentItem();
    }else{
        showDataEditor(TERADATA_NONE, nullptr);
    }

    ui->projNavigator->setEnabled(true);
}

void MainWindow::updateCurrentUser()
{
    if (m_comManager->getCurrentUser().hasFieldName("user_name")){
        // Ok, we have a user to update.
        ui->lblUser->setText(m_comManager->getCurrentUser().getName());
        ui->btnConfig->setVisible(m_comManager->getCurrentUser().getFieldValue("user_superadmin").toBool());
    }
}

void MainWindow::updateOnlineCounts(int count)
{
    int index = ui->tabMainMenu->indexOf(ui->tabOnline);
    ui->tabMainMenu->setTabText(index, tr("En ligne (") + QString::number(count) + ")");
}

void MainWindow::processGenericDataReply(TeraDataTypes item_data_type, QList<TeraData> datas)
{
    if (datas.isEmpty())
        return;

    if (m_data_editor){
        if (m_data_editor->getData()->getDataType() == item_data_type && m_data_editor->getData()->getId()==0){
            // New item that was saved?
            if (m_data_editor->getData()->getName() == datas.first().getName()){
                if (item_data_type == TERADATA_PROJECT){
                    // New project - update access!
                    m_comManager->doUpdateCurrentUser();
                }
                // Try to update project navigator with new created item
                if (!ui->projNavigator->selectItemByName(item_data_type, datas.first().getName())){
                    // Not found in project navigator - close data editor
                    showDataEditor(TERADATA_NONE, nullptr);
                }
                ui->projNavigator->setEnabled(true);
            }
        }
    }

    if (m_waiting_for_data_type != item_data_type)
        return;

    m_waiting_for_data_type = TERADATA_NONE;

    // Show editor
    showDataEditor(item_data_type, &datas.first());

}


void MainWindow::com_socketError(QAbstractSocket::SocketError error, QString error_msg)
{
    Q_UNUSED(error)
    addMessage(Message::MESSAGE_ERROR, error_msg);
}

void MainWindow::com_networkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code)
{
    if (error_msg.endsWith('\n'))
        error_msg = error_msg.left(error_msg.length()-1);

    if (status_code > 0)
        addMessage(Message::MESSAGE_ERROR, tr("Erreur HTTP ") + QString::number(status_code) + ": " + error_msg);
    else
        addMessage(Message::MESSAGE_ERROR, tr("Erreur ") + QString::number(error) + ": " + error_msg);

    if (op == QNetworkAccessManager::DeleteOperation){
        // Also show a message box for that error
        GlobalMessageBox msg;
        msg.showError(tr("Suppression impossible"), error_msg);
    }
}

void MainWindow::com_waitingForReply(bool waiting)
{
    /*if (waiting){
        if (!hasWaitingMessage())
            addMessage(Message::MESSAGE_WORKING, "");
     }else{
        if (m_currentMessage.getMessageType()==Message::MESSAGE_WORKING)
            showNextMessage();
    }*/
    ui->icoLoading->setVisible(waiting);
    if (waiting)
        m_loadingIcon->start();
    else {
        m_loadingIcon->stop();
    }
    ui->btnEditUser->setEnabled(!waiting);
    ui->btnConfig->setEnabled(!waiting);
    ui->projNavigator->setEnabled(!waiting);
}

void MainWindow::com_postReplyOK(QString path)
{
    Q_UNUSED(path);
    addMessage(Message::MESSAGE_OK, tr("Données sauvegardées."));

}

void MainWindow::com_deleteResultsOK(QString path, int id)
{
    if (m_data_editor){
        TeraData* current_data = m_data_editor->getData();
        if (current_data){
            if (current_data->getDataType() == TeraData::getDataTypeFromPath(path) &&
                    current_data->getId() == id && path != WEB_PARTICIPANTINFO_PATH && path != WEB_GROUPINFO_PATH){
                // Show dashboard as current item was deleted and not a participant or a participant group
                showDashboard(true);
            }
        }
    }

    ui->projNavigator->removeItem(TeraData::getDataTypeFromPath(path), id);

    addMessage(Message::MESSAGE_OK, tr("Données supprimées."));
}

void MainWindow::com_posting(QString path, QString data)
{
    Q_UNUSED(data)
    if (path != WEB_SESSIONMANAGER_PATH){
        QString data_type = TeraData::getDataTypeNameText(TeraData::getDataTypeFromPath(path));
        if (!data_type.isEmpty()){
            GlobalEvent event(EVENT_DATA_EDIT, data_type + tr(" - mise à jour..."));
            addGlobalEvent(event);
        }
    }
}

void MainWindow::com_querying(QString path)
{
    if (path != WEB_FORMS_PATH && path != WEB_LOGOUT_PATH && path != WEB_REFRESH_TOKEN_PATH && path != WEB_DISCONNECT_PATH){
        QString data_type = TeraData::getDataTypeNameText(TeraData::getDataTypeFromPath(path));
        if (!data_type.isEmpty()){
            GlobalEvent event(EVENT_DATA_QUERY, tr("Récupération de ") + data_type + "...");
            //addGlobalEvent(event);
        }
    }
}

void MainWindow::com_deleting(QString path)
{
    QString data_type = TeraData::getDataTypeNameText(TeraData::getDataTypeFromPath(path));
    if (!data_type.isEmpty()){
        GlobalEvent event(EVENT_DATA_DELETE, data_type + tr(" - suppression..."));
        addGlobalEvent(event);
    }
}

void MainWindow::com_downloadProgress(DownloadingFile *file)
{
    if (!m_download_dialog){
        // New download request - create dialog and add file
        m_download_dialog = new TransferProgressDialog(this);
        m_download_dialog->show();
    }
    m_download_dialog->updateTransferringFile(file);
}

void MainWindow::com_downloadCompleted(DownloadingFile *file)
{
    if (m_download_dialog){
        if (m_download_dialog->transferFileCompleted(file)){
            // If we are here, no more downloads are pending. Close download dialog.
            m_download_dialog->close();
            m_download_dialog->deleteLater();
            m_download_dialog = nullptr;
        }
    }
}

void MainWindow::com_preferencesUpdated()
{
    if (m_currentLanguage != m_comManager->getCurrentPreferences().getLanguage()){ // Filter initial language change
        GlobalMessageBox msg;
        if (msg.showYesNo(tr("Changement de langue"), tr("La langue a été modifiée.\nSouhaitez-vous vous déconnecter pour appliquer les changements?")) == QMessageBox::Yes){
            emit logout_request();
        }
        m_currentLanguage = m_comManager->getCurrentPreferences().getLanguage();
    }
}

void MainWindow::com_sessionStarted(TeraData session_type, int id_session)
{
    if (!m_inSessionWidget){
        // Loads the in-session widget since none loaded yet!
        setInSession(true, &session_type, id_session);
    }else{
        // Update session id in InSessionWidget
        m_inSessionWidget->setSessionId(id_session);
    }
}

void MainWindow::com_sessionStartRequested(TeraData session_type)
{
    // Loads the in-session widget
    setInSession(true, &session_type, -1);
}

void MainWindow::com_sessionStopped(int id_session)
{
    setInSession(false, nullptr, id_session);
}

void MainWindow::com_sessionError(QString error)
{
    setInSession(false, nullptr, -1);
    GlobalMessageBox msg;
    msg.showError(tr("Erreur de séance"), tr("Une erreur est survenue:\n") + error + tr("\n\nLa séance ne peut pas continuer."));
}

void MainWindow::ws_userEvent(UserEvent event)
{

    if (event.type() == UserEvent_EventType_USER_CONNECTED){
        // Don't do anything for current user!
        if (event.user_uuid() == m_comManager->getCurrentUser().getUuid().toStdString())
            return;
        QString msg_text = "<font color=yellow>" + QString::fromStdString(event.user_fullname()) + "</font>" + tr(" est en ligne.");
        addNotification(NotificationWindow::TYPE_MESSAGE, msg_text, "://icons/software_user_online.png", "://sounds/notify_online.wav");

        // Add a trace in events also
        GlobalEvent g_event(EVENT_LOGIN, msg_text);
        addGlobalEvent(g_event);

        // Update online users list
        //updateOnlineUser(QString::fromStdString(event.user_uuid()), true, QString::fromStdString(event.user_fullname()));
    }

    if (event.type() == UserEvent_EventType_USER_DISCONNECTED){
        QString msg_text = "<font color=yellow>" + QString::fromStdString(event.user_fullname()) + "</font>" +  tr(" est hors-ligne.");
        addNotification(NotificationWindow::TYPE_MESSAGE, msg_text, "://icons/software_user_offline.png", "://sounds/notify_offline.wav");

        GlobalEvent g_event(EVENT_LOGOUT, msg_text);
        addGlobalEvent(g_event);

        // Update online users list
        //updateOnlineUser(QString::fromStdString(event.user_uuid()), false);
    }
}

void MainWindow::ws_participantEvent(ParticipantEvent event)
{
    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_CONNECTED){
        // Is in the current site?
        if (QString::fromStdString(event.participant_site_name()) == ui->projNavigator->getCurrentSiteName()){
            QString msg_text = "<u>" + QString::fromStdString(event.participant_project_name()) + "</u><br/>";
            msg_text += "<font color=yellow>" + QString::fromStdString(event.participant_name()) + "</font>" + tr(" est en ligne.");
            addNotification(NotificationWindow::TYPE_MESSAGE, msg_text, "://icons/patient_online.png", "://sounds/notify_online.wav");
            // Add a trace in events also
            GlobalEvent g_event(EVENT_LOGIN, msg_text);
            addGlobalEvent(g_event);

            // Update online users list
            //updateOnlineParticipant(QString::fromStdString(event.participant_uuid()), true, QString::fromStdString(event.participant_name()));
        }
    }

    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_DISCONNECTED){
        // Is in the current site?
        if (QString::fromStdString(event.participant_site_name()) == ui->projNavigator->getCurrentSiteName()){
            QString msg_text = "<u>" + QString::fromStdString(event.participant_project_name()) + "</u><br/>";
            msg_text += "<font color=yellow>" + QString::fromStdString(event.participant_name()) + "</font>" + tr(" est hors-ligne.");
            addNotification(NotificationWindow::TYPE_MESSAGE, msg_text, "://icons/patient.png", "://sounds/notify_offline.wav");
            // Add a trace in events also
            GlobalEvent g_event(EVENT_LOGOUT, msg_text);
            addGlobalEvent(g_event);

            // Update online participants list
            //updateOnlineParticipant(QString::fromStdString(event.participant_uuid()), false);
        }
    }
}

void MainWindow::ws_joinSessionEvent(JoinSessionEvent event)
{
    if (isInSession()){
        // If we are in a session, the InSession Widget will handle that event for us.
        return;
    }

    // Don't display join session event for session we created!
    if (QString::fromStdString(event.session_creator_name()) == m_comManager->getCurrentUser().getName()){
        return;
    }

    // Ensure we are in the invitees list
    for (int i=0; i<event.session_users_size(); i++){
        if (QString::fromStdString(event.session_users(i)) == m_comManager->getCurrentUser().getUuid()){
            // Display JoinSession dialog
            if (m_joinSession_dialog)
                m_joinSession_dialog->deleteLater();

            m_joinSession_dialog = new JoinSessionDialog(m_comManager, event, this);
            connect(m_joinSession_dialog, &JoinSessionDialog::finished, this, &MainWindow::joinSessionDialogFinished);
            m_joinSession_dialog->open();
            return;
        }
    }
}

void MainWindow::inSession_sessionEndedWithError()
{
    com_sessionError(tr("Erreur de serveur."));
}

void MainWindow::on_btnLogout_clicked()
{
    GlobalMessageBox msg;

    if (msg.showYesNo(tr("Déconnexion"), tr("Vous serez déconnecté du logiciel. Toute donnée non enregistrée sera perdue.") + "\n\n" + tr("Souhaitez-vous continuer?")) != QMessageBox::Yes){
        return;
    }

    // Add logged action
    GlobalEvent logout_event(EVENT_LOGOUT, tr("Déconnexion"));
    GlobalEventLogger::instance()->logEvent(logout_event);

    emit logout_request();
}

void MainWindow::addMessage(Message::MessageType msg_type, QString msg)
{
    Message message(msg_type, msg);
    addMessage(message);
}

void MainWindow::addMessage(Message &msg)
{
    ui->wdgMessages->addMessage(msg);

    // Create event for error
    if (msg.getMessageType() == Message::MESSAGE_ERROR){
        GlobalEvent error_event(EVENT_ERROR, msg.getMessageText());
        addGlobalEvent(error_event);
    }

    // Create event for warning
    if (msg.getMessageType() == Message::MESSAGE_WARNING){
        GlobalEvent warning_event(EVENT_WARNING, msg.getMessageText());
        addGlobalEvent(warning_event);
    }

    /*if (ui->wdgMessages->isHidden())
        ui->wdgMessages->showNextMessage();*/
}

void MainWindow::addNotification(const NotificationWindow::NotificationType notification_type, const QString &text, const QString &iconPath, const QString &soundPath, const int &width, const int &height, const int &duration)
{
    // Notification window
    NotificationWindow* notify = new NotificationWindow(this, notification_type, m_notifications.count()+1, width, height, duration);
    notify->setNotificationText(text);
    notify->setNotificationIcon(iconPath);
    m_notifications.append(notify);

    connect(notify, &NotificationWindow::notificationFinished, this, &MainWindow::notificationCompleted);
    connect(notify, &NotificationWindow::notificationClosed, this, &MainWindow::notificationCompleted);

    if (m_comManager->getCurrentPreferences().isNotifySounds() && !soundPath.isEmpty()){
        if (!m_inSessionWidget) // Don't play sounds when in session!
            QSound::play(soundPath);
    }
}

void MainWindow::on_btnEditUser_clicked()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }

    // Hold all selection from happening in the project manager
    ui->projNavigator->setOnHold(true);

    m_diag_editor = new BaseDialog(this);
    //m_diag_editor->setMinimumHeight(600);
    m_diag_editor->setMinimumSize(this->width()/2, 2*this->height()/3);
    UserWidget* user_editor = new UserWidget(m_comManager, &(m_comManager->getCurrentUser()), nullptr);

    m_diag_editor->setCentralWidget(user_editor);

    user_editor->setLimited(true);
    connect(user_editor, &UserWidget::dataWasChanged, m_diag_editor, &QDialog::accept);
    connect(m_diag_editor, &QDialog::finished, this, &MainWindow::editorDialogFinished);

    m_diag_editor->setWindowTitle(tr("Votre compte"));

    m_diag_editor->open();
}

void MainWindow::on_btnConfig_clicked()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }

    // Hold all selection from happening in the project manager
    ui->projNavigator->setOnHold(true);

    m_diag_editor = new BaseDialog(this);
    ConfigWidget* config_editor = new ConfigWidget(m_comManager, nullptr);
    m_diag_editor->setCentralWidget(config_editor);

    m_diag_editor->setFixedSize(size().width()-50, size().height()-150);
    m_diag_editor->move(this->x()+25, this->y()+75);

    connect(m_diag_editor, &QDialog::finished, this, &MainWindow::editorDialogFinished);
    connect(config_editor, &ConfigWidget::closeRequest, m_diag_editor, &QDialog::accept);

    m_diag_editor->setWindowTitle(tr("Configuration Globale"));

    m_diag_editor->open();

}

void MainWindow::on_btnLog_toggled(bool checked)
{
    ui->dockerRight->setVisible(checked);
}

void MainWindow::on_tableHistory_itemDoubleClicked(QTableWidgetItem *item)
{
    // Display more info on that item
    GlobalMessageBox msg_box;
    int row = item->row();
    QString date_str = ui->tableHistory->item(row,1)->text();
    QString event_str = ui->tableHistory->item(row,2)->data(Qt::UserRole).toString();
    QIcon icon = ui->tableHistory->item(row,0)->icon();

    msg_box.showInfo(tr("Détails"), date_str + " - " + event_str, &icon);

}

void MainWindow::changeEvent(QEvent* event)
{
    if (event) {
        switch(event->type()) {

        case QEvent::LanguageChange:{
            /*if (m_initialLanguageSetted){ // Filter initial language change
                GlobalMessageBox msg;
                if (msg.showYesNo(tr("Changement de langue"), tr("La langue a été modifiée.\nSouhaitez-vous vous déconnecter pour appliquer les changements?")) == QMessageBox::Yes){
                    emit logout_request();
                }

            }else{
                m_initialLanguageSetted = true;
            }*/

            //qDebug() << "MainWindow::changeEvent - LanguageChange";
        }
            break;

            // this event is send, if the system, language changes
        case QEvent::LocaleChange:
            break;

        default:
            break;
        }

    }

    //Base class
    QMainWindow::changeEvent(event);

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // About to close... check if we have something in progress that prevents it
    if (m_inSessionWidget){
        if (!m_inSessionWidget->sessionCanBeEnded()){
            GlobalMessageBox msg;
            msg.showWarning(tr("Séance en cours"), tr("La séance en cours empêche la fermeture du logiciel.") + "\n\n" + tr("Veuillez la terminer avant de poursuivre."));
            event->ignore();
            return;
        }
    }
    event->accept();
}


void MainWindow::on_lblLogo_clicked()
{
    AboutDialog about(m_comManager->getServerUrl(), this);

    about.setFixedSize(size().width()-50, size().height()-150);
    about.move(this->x()+25, this->y()+75);

    about.exec();
}
