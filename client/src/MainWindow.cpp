#include "MainWindow.h"
#include <QNetworkReply>
#include <QDesktopWidget>
#include <QApplication>

#include "ui_MainWindow.h"

#include "editors/SiteWidget.h"
#include "editors/ProjectWidget.h"
#include "editors/GroupWidget.h"
#include "editors/ParticipantWidget.h"


MainWindow::MainWindow(ComManager *com_manager, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground);

    m_comManager = com_manager;
    m_diag_editor = nullptr;
    m_data_editor = nullptr;
    m_inSessionWidget = nullptr;
    m_download_dialog = nullptr;
    m_waiting_for_data_type = TERADATA_NONE;
    m_currentDataType = TERADATA_NONE;
    m_currentDataId = -1;
    m_currentMessage.setMessage(Message::MESSAGE_NONE,"");

    // Initial UI state
    initUi();

    // Connect signals
    connectSignals();

    // Update user in case we already have it
    updateCurrentUser();

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

void MainWindow::connectSignals()
{
    connect(m_comManager, &ComManager::currentUserUpdated, this, &MainWindow::updateCurrentUser);
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
    connect(m_comManager, &ComManager::sessionStartRequested, this, &MainWindow::com_sessionStartRequested);
    connect(m_comManager, &ComManager::posting, this, &MainWindow::com_posting);
    connect(m_comManager, &ComManager::querying, this, &MainWindow::com_querying);
    connect(m_comManager, &ComManager::deleting, this, &MainWindow::com_deleting);

    connect(m_comManager->getWebSocketManager(), &WebSocketManager::userEventReceived, this, &MainWindow::ws_userEvent);
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::participantEventReceived, this, &MainWindow::ws_participantEvent);

    connect(&m_msgTimer, &QTimer::timeout, this, &MainWindow::showNextMessage);

    connect(ui->projNavigator, &ProjectNavigator::dataDisplayRequest, this, &MainWindow::dataDisplayRequested);
    connect(ui->projNavigator, &ProjectNavigator::dataDeleteRequest, this, &MainWindow::dataDeleteRequested);

    connect(GlobalEventLogger::instance(), &GlobalEventLogger::newEventLogged, this, &MainWindow::addGlobalEvent);
}

void MainWindow::initUi()
{
    // Setup messages
    ui->frameMessages->hide();
    m_msgTimer.setSingleShot(true);
    m_msgTimer.setInterval(8000);

    // Setup event view
    ui->tableHistory->setColumnWidth(0, 20);

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

    // Set version number
    ui->lblVersion->setText(QString(TERAPLUS_VERSION));

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
        return;
    }

    // Save values to display them again if needed
    m_currentDataType = data_type;
    m_currentDataId = data->getId();


    if (data_type == TERADATA_SITE){
        m_data_editor = new SiteWidget(m_comManager, data);
        m_data_editor->setLimited(!(m_comManager->isCurrentUserSiteAdmin(data->getId())) && data->getId()>0);
    }

    if (data_type == TERADATA_PROJECT){
        m_data_editor = new ProjectWidget(m_comManager, data);
        m_data_editor->setLimited(!(m_comManager->isCurrentUserProjectAdmin(data->getId())) && data->getId()>0);
    }

    if (data_type == TERADATA_GROUP){
        m_data_editor = new GroupWidget(m_comManager, data);
        bool limited = false;
        if (data->hasFieldName("id_project")){
            limited = !(m_comManager->isCurrentUserProjectAdmin(data->getFieldValue("id_project").toInt()));
        }
        m_data_editor->setLimited(limited);
    }

    if (data_type == TERADATA_PARTICIPANT){
        m_data_editor = new ParticipantWidget(m_comManager, data);
        bool limited = true;
        if (data->hasFieldName("id_project")){
            limited = !(m_comManager->isCurrentUserProjectAdmin(data->getFieldValue("id_project").toInt())); //m_comManager->getCurrentUserProjectRole(data->getFieldValue("id_project").toInt()) != "admin";
        }
        if (data->getId()==0)
            limited = false;
        m_data_editor->setLimited(limited);

    }

    if (m_data_editor){
        ui->wdgMainTop->layout()->addWidget(m_data_editor);
        connect(m_data_editor, &DataEditorWidget::dataWasDeleted, this, &MainWindow::dataEditorCancelled);
    }else{
        LOG_ERROR("Unhandled data editor: " + TeraData::getPathForDataType(data_type), "MainWindow::showDataEditor");
    }


}

void MainWindow::setInSession(bool in_session, const TeraData *session_type, const int &id_session)
{
    if (m_data_editor){
        m_data_editor->disconnectSignals();
        ui->wdgMainTop->layout()->removeWidget(m_data_editor);
        m_data_editor->deleteLater();
        m_data_editor = nullptr;
    }
    if (m_inSessionWidget){
        m_inSessionWidget->disconnectSignals();
        ui->wdgMainTop->layout()->removeWidget(m_inSessionWidget);
        m_inSessionWidget->deleteLater();
        m_inSessionWidget = nullptr;
    }

    ui->dockerLeft->setVisible(!in_session);
    ui->btnLogout->setVisible(!in_session);

    if (in_session){
        m_inSessionWidget = new InSessionWidget(m_comManager, session_type, id_session);
        ui->wdgMainTop->layout()->addWidget(m_inSessionWidget);
    }else{

        // Loads back the "previous" data type
        dataDisplayRequested(m_currentDataType, m_currentDataId);
    }
}

void MainWindow::updateOnlineUser(const QString &user_uuid, const bool &online,  const QString &user_name)
{
    QListWidgetItem* user_item = nullptr;

    if (m_onlineUsers.contains(user_uuid)){
        user_item = m_onlineUsers[user_uuid];
    }else{
        // If offline, we don't need to anything since it's there !
        if (online){
            user_item = new QListWidgetItem(QIcon("://icons/software_user_online.png"), user_name);
            ui->lstOnlineUsers->addItem(user_item);
            m_onlineUsers[user_uuid] = user_item;
        }
    }

    if (user_item){
        if (!online){
            // We must remove that item
            delete user_item;
            m_onlineUsers.remove(user_uuid);
        }else{
            // Update name if needed
            user_item->setText(user_name);

            // Resort items
            ui->lstOnlineUsers->sortItems();
        }
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

void MainWindow::showNextMessage()
{
    m_loadingIcon->stop();
    ui->frameMessages->hide();
    m_msgTimer.stop();
    if (m_messages.isEmpty()){
        m_currentMessage.setMessage(Message::MESSAGE_NONE,"");
        return;
    }

    m_currentMessage = m_messages.takeFirst();

    QString background_color = "rgba(128,128,128,50%)";
    QString icon_path = "";

    switch(m_currentMessage.getMessageType()){
    case Message::MESSAGE_OK:
        background_color = "rgba(0,200,0,50%)";
        ui->icoMessage->setPixmap(QPixmap("://icons/ok.png"));
        break;
    case Message::MESSAGE_ERROR:
        background_color = "rgba(200,0,0,50%)";
        ui->icoMessage->setPixmap(QPixmap("://icons/error.png"));
        break;
    case Message::MESSAGE_WARNING:
        background_color = "rgba(255,85,0,50%)";
        ui->icoMessage->setPixmap(QPixmap("://icons/warning.png"));
        break;
    case Message::MESSAGE_WORKING:
        background_color = "rgba(128,128,128,50%)";
        ui->icoMessage->setMovie(m_loadingIcon);
        m_loadingIcon->start();
        break;
    default:
        break;
    }
    ui->frameMessages->setStyleSheet("QWidget#frameMessages{background-color: " + background_color + ";}");
    ui->lblMessage->setText(m_currentMessage.getMessageText());
    if (m_currentMessage.getMessageType() != Message::MESSAGE_ERROR && m_currentMessage.getMessageType()!=Message::MESSAGE_NONE)
        m_msgTimer.start();

    QPropertyAnimation *animate = new QPropertyAnimation(ui->frameMessages,"windowOpacity",this);
    animate->setDuration(1000);
    animate->setStartValue(0.0);
    animate->setKeyValueAt(0.1, 0.8);
    animate->setKeyValueAt(0.9, 0.8);
    animate->setEndValue(0.0);
    animate->start(QAbstractAnimation::DeleteWhenStopped);
    ui->frameMessages->show();
}

void MainWindow::notificationCompleted(NotificationWindow *notify)
{
    m_notifications.removeAll(notify);
    notify->deleteLater();

    for(int i=0; i<m_notifications.count(); i++){
        m_notifications.at(i)->resetPosition(i+1);
    }
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
    QTableWidgetItem* desc_item = new QTableWidgetItem(event.getEventText());
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
        }

        if (data_type == TERADATA_PARTICIPANT){
            if (ui->projNavigator->getCurrentGroupId() > 0)
                new_data->setFieldValue("id_participant_group", ui->projNavigator->getCurrentGroupId());
            if (ui->projNavigator->getCurrentProjectId() > 0)
                new_data->setFieldValue("id_project", ui->projNavigator->getCurrentProjectId());
        }

        showDataEditor(data_type, new_data);
        return;
    }

    // Set flag to wait for that specific data type
    if (m_waiting_for_data_type != TERADATA_NONE)
        LOG_WARNING("Request for new data for editor, but still waiting on previous one!", "MainWindow::dataDisplayRequested");
    m_waiting_for_data_type = data_type;

    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID, QString::number(data_id));
    m_comManager->doQuery(TeraData::getPathForDataType(data_type), query);

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
    showDataEditor(TERADATA_NONE, nullptr);
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
                // Yes, it is - close data editor
                showDataEditor(TERADATA_NONE, nullptr);
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
    ui->projNavigator->removeItem(TeraData::getDataTypeFromPath(path), id);
}

void MainWindow::com_posting(QString path, QString data)
{
    Q_UNUSED(data)

    QString data_type = TeraData::getDataTypeNameText(TeraData::getDataTypeFromPath(path));
    if (!data_type.isEmpty()){
        GlobalEvent event(EVENT_DATA_EDIT, data_type + tr(" - mise à jour..."));
        addGlobalEvent(event);
    }
}

void MainWindow::com_querying(QString path)
{
    if (path != WEB_FORMS_PATH){
        QString data_type = TeraData::getDataTypeNameText(TeraData::getDataTypeFromPath(path));
        if (!data_type.isEmpty()){
            GlobalEvent event(EVENT_DATA_QUERY, tr("Récupération de ") + data_type + "...");
            addGlobalEvent(event);
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

void MainWindow::com_downloadProgress(DownloadedFile *file)
{
    if (!m_download_dialog){
        // New download request - create dialog and add file
        m_download_dialog = new DownloadProgressDialog(this);
        m_download_dialog->show();
    }
    m_download_dialog->updateDownloadedFile(file);
}

void MainWindow::com_downloadCompleted(DownloadedFile *file)
{
    if (m_download_dialog){
        if (m_download_dialog->downloadFileCompleted(file)){
            // If we are here, no more downloads are pending. Close download dialog.
            m_download_dialog->close();
            m_download_dialog->deleteLater();
            m_download_dialog = nullptr;
        }
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

void MainWindow::ws_userEvent(UserEvent event)
{
    // TODO: Don't do anything for current user!
    if (event.type() == UserEvent_EventType_USER_CONNECTED){
        QString msg_text = "<font color=yellow>" + QString::fromStdString(event.user_fullname()) + "</font>" + tr(" est en ligne.");
        addNotification(NotificationWindow::TYPE_MESSAGE, msg_text, "://icons/software_user_online.png");

        // Add a trace in events also
        GlobalEvent g_event(EVENT_LOGIN, msg_text);
        addGlobalEvent(g_event);

        // Update online users list
        updateOnlineUser(QString::fromStdString(event.user_uuid()), true, QString::fromStdString(event.user_fullname()));
    }

    if (event.type() == UserEvent_EventType_USER_DISCONNECTED){
        QString msg_text = "<font color=yellow>" + QString::fromStdString(event.user_fullname()) + "</font>" +  tr(" est hors-ligne.");
        addNotification(NotificationWindow::TYPE_MESSAGE, msg_text, "://icons/software_user_offline.png");

        GlobalEvent g_event(EVENT_LOGOUT, msg_text);
        addGlobalEvent(g_event);

        // Update online users list
        updateOnlineUser(QString::fromStdString(event.user_uuid()), false);
    }
}

void MainWindow::ws_participantEvent(ParticipantEvent event)
{
    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_CONNECTED){
        // Is in the current site?
        if (QString::fromStdString(event.participant_site_name()) == ui->projNavigator->getCurrentSiteName()){
            QString msg_text = "<u>" + QString::fromStdString(event.participant_project_name()) + "</u><br/>";
            msg_text += "<font color=yellow>" + QString::fromStdString(event.participant_name()) + "</font>" + tr(" est en ligne.");
            addNotification(NotificationWindow::TYPE_MESSAGE, msg_text, "://icons/patient_online.png");
            // Add a trace in events also
            GlobalEvent event(EVENT_LOGIN, msg_text);
            addGlobalEvent(event);
        }
    }

    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_DISCONNECTED){
        // Is in the current site?
        if (QString::fromStdString(event.participant_site_name()) == ui->projNavigator->getCurrentSiteName()){
            QString msg_text = "<font color=cyan><u>" + QString::fromStdString(event.participant_project_name()) + "</u></font><br/>";
            msg_text += "<font color=yellow>" + QString::fromStdString(event.participant_name()) + "</font>" + tr(" est hors-ligne.");
            addNotification(NotificationWindow::TYPE_MESSAGE, msg_text, "://icons/patient.png");
            // Add a trace in events also
            GlobalEvent event(EVENT_LOGOUT, msg_text);
            addGlobalEvent(event);
        }
    }
}

void MainWindow::on_btnLogout_clicked()
{
    //qDebug() << "on_btnLogout_clicked()";

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
    m_messages.append(msg);

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

    if (ui->frameMessages->isHidden())
        showNextMessage();
}

void MainWindow::addNotification(const NotificationWindow::NotificationType notification_type, const QString &text, const QString &iconPath)
{
    // Notification window
    NotificationWindow* notify = new NotificationWindow(this, notification_type, m_notifications.count()+1);
    notify->setNotificationText(text);
    notify->setNotificationIcon(iconPath);
    m_notifications.append(notify);

    connect(notify, &NotificationWindow::notificationFinished, this, &MainWindow::notificationCompleted);
    connect(notify, &NotificationWindow::notificationClosed, this, &MainWindow::notificationCompleted);
}

bool MainWindow::hasWaitingMessage()
{
    for (Message msg:m_messages){
        if (msg.getMessageType()==Message::MESSAGE_WORKING)
            return true;
    }
    return false;
}

void MainWindow::on_btnCloseMessage_clicked()
{
    showNextMessage();
}

void MainWindow::on_btnEditUser_clicked()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }

    // Hold all selection from happening in the project manager
    ui->projNavigator->setOnHold(true);

    m_diag_editor = new BaseDialog(this);
    UserWidget* user_editor = new UserWidget(m_comManager, &(m_comManager->getCurrentUser()), nullptr);

    m_diag_editor->setCentralWidget(user_editor);

    user_editor->setLimited(true);
    connect(user_editor, &UserWidget::closeRequest, m_diag_editor, &QDialog::accept);
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
    QString event_str = ui->tableHistory->item(row,2)->text();
    QIcon icon = ui->tableHistory->item(row,0)->icon();

    msg_box.showInfo(tr("Détails"), date_str + " - " + event_str, &icon);

}

void MainWindow::changeEvent(QEvent* event)
{
    if (event) {
        switch(event->type()) {

        case QEvent::LanguageChange:
            if (m_currentDataId >= 0){ // Filter initial language change
                GlobalMessageBox msg;
                if (msg.showYesNo(tr("Changement de langue"), tr("La langue a été modifiée.\nSouhaitez-vous vous déconnecter pour appliquer les changements?")) == QMessageBox::Yes){
                    emit logout_request();
                }

            }
            //qDebug() << "MainWindow::changeEvent - LanguageChange";
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
