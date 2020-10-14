#include "OnlineManagerWidget.h"
#include "ui_OnlineManagerWidget.h"

// Must be reset each time a site is changed.
// 1. Query online * using APIs
// 2. React to events using protobuf messages
//

OnlineManagerWidget::OnlineManagerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OnlineManagerWidget)
{
    m_comManager = nullptr;
    ui->setupUi(this);
}

OnlineManagerWidget::~OnlineManagerWidget()
{
    delete ui;
}

void OnlineManagerWidget::setComManager(ComManager *comMan)
{
    if (m_comManager)
        m_comManager->deleteLater();

    m_comManager = comMan;

    connectSignals();

    initUi();

}

void OnlineManagerWidget::setCurrentSite(const QString &site_name, const int &site_id)
{
    m_siteName = site_name;
    m_siteId = site_id;

    // Clear participant list since site was changed
    ui->lstOnlineDevices->clear();
    ui->lstOnlineParticipants->clear();

    refreshOnlines();
}

void OnlineManagerWidget::initUi()
{
    ui->lstOnlineDevices->hide();
    ui->lstOnlineUsers->hide();
    ui->btnShowOnlineParticipants->setChecked(true); // Expand participants by default
}

void OnlineManagerWidget::connectSignals()
{
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::userEventReceived, this, &OnlineManagerWidget::ws_userEvent);
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::participantEventReceived, this, &OnlineManagerWidget::ws_participantEvent);
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::deviceEventReceived, this, &OnlineManagerWidget::ws_deviceEvent);

    connect(m_comManager, &ComManager::onlineDevicesReceived, this, &OnlineManagerWidget::processOnlineDevices);
    connect(m_comManager, &ComManager::onlineParticipantsReceived, this, &OnlineManagerWidget::processOnlineParticipants);
    connect(m_comManager, &ComManager::onlineUsersReceived, this, &OnlineManagerWidget::processOnlineUsers);
}

void OnlineManagerWidget::refreshOnlines()
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_WITH_BUSY, "1");
    m_comManager->doQuery(WEB_ONLINEDEVICEINFO_PATH, args);
    m_comManager->doQuery(WEB_ONLINEPARTICIPANTINFO_PATH, args);
    m_comManager->doQuery(WEB_ONLINEUSERINFO_PATH, args);

}

void OnlineManagerWidget::updateCounts()
{
    ui->lblDevicesNum->setText(QString::number(ui->lstOnlineDevices->count()));
    ui->lblParticipantsNum->setText(QString::number(ui->lstOnlineParticipants->count()));
    ui->lblUsersNum->setText(QString::number(ui->lstOnlineUsers->count()));
}

void OnlineManagerWidget::updateOnlineUser(const TeraData &online_user)
{

    QString uuid = online_user.getUuid();

    QListWidgetItem* user_item = nullptr;

    if (m_onlineUsers.contains(uuid)){
        user_item = m_onlineUsers[uuid];
    }else{
        if (online_user.isOnline()/* || online_user.isBusy()*/){ // Not online and not busy = don't need to add!
            user_item = new QListWidgetItem(QIcon(online_user.getIconStateFilename()), online_user.getName());
            ui->lstOnlineUsers->addItem(user_item);
            m_onlineUsers[uuid] = user_item;
        }
    }

    if (user_item){
        if (!online_user.isOnline() /*&& !online_user.isBusy()*/){
            // We must remove that item
            delete user_item;
            m_onlineUsers.remove(uuid);
        }else{
            // Update name / icon if needed
            user_item->setText(online_user.getName());
            user_item->setIcon(QIcon(online_user.getIconStateFilename()));

            // Resort items
            ui->lstOnlineUsers->sortItems();
        }
    }
}

void OnlineManagerWidget::updateOnlineParticipant(const TeraData &online_participant)
{
    QString uuid = online_participant.getUuid();

    QListWidgetItem* participant_item = nullptr;

    if (m_onlineParticipants.contains(uuid)){
        participant_item = m_onlineParticipants[uuid];
    }else{
        if (online_participant.isOnline()/* || online_participant.isBusy()*/){ // Not online and not busy = don't need to add!
            participant_item = new QListWidgetItem(QIcon(online_participant.getIconStateFilename()), online_participant.getName());
            ui->lstOnlineParticipants->addItem(participant_item);
            m_onlineParticipants[uuid] = participant_item;
        }
    }

    if (participant_item){
        if (!online_participant.isOnline()/* && !online_participant.isBusy()*/){
            // We must remove that item
            delete participant_item;
            m_onlineParticipants.remove(uuid);
        }else{
            // Update name / icon if needed
            participant_item->setText(online_participant.getName());
            participant_item->setIcon(QIcon(online_participant.getIconStateFilename()));

            // Resort items
            ui->lstOnlineParticipants->sortItems();
        }
    }
}

void OnlineManagerWidget::updateOnlineDevice(const TeraData &online_device)
{
    QString uuid = online_device.getUuid();

    QListWidgetItem* device_item = nullptr;

    if (m_onlineDevices.contains(uuid)){
        device_item = m_onlineDevices[uuid];
    }else{
        if (online_device.isOnline()/* || online_device.isBusy()*/){ // Not online and not busy = don't need to add!
            device_item = new QListWidgetItem(QIcon(online_device.getIconStateFilename()), online_device.getName());
            ui->lstOnlineDevices->addItem(device_item);
            m_onlineDevices[uuid] = device_item;
        }
    }

    if (device_item){
        if (!online_device.isOnline()/* && !online_device.isBusy()*/){
            // We must remove that item
            delete device_item;
            m_onlineDevices.remove(uuid);
        }else{
            // Update name / icon if needed
            device_item->setText(online_device.getName());
            device_item->setIcon(QIcon(online_device.getIconStateFilename()));

            // Resort items
            ui->lstOnlineDevices->sortItems();
        }
    }
}

void OnlineManagerWidget::ws_userEvent(UserEvent event)
{
    TeraData user_data(TERADATA_ONLINE_USER);
    user_data.setName(QString::fromStdString(event.user_fullname()));
    user_data.setUuid(QString::fromStdString(event.user_uuid()));

    if (event.type() == UserEvent_EventType_USER_CONNECTED){
        user_data.setOnline(true);
        updateOnlineUser(user_data);
    }
    if (event.type() == UserEvent_EventType_USER_DISCONNECTED){
        user_data.setOnline(false);
        updateOnlineUser(user_data);
    }
    if (event.type() == UserEvent_EventType_USER_JOINED_SESSION){
        user_data.setBusy(true);
        updateOnlineUser(user_data);
    }
    if (event.type() == UserEvent_EventType_USER_LEFT_SESSION){
        user_data.setBusy(false);
    }
    updateCounts();

}

void OnlineManagerWidget::ws_participantEvent(ParticipantEvent event)
{
    TeraData participant_data(TERADATA_ONLINE_PARTICIPANT);
    participant_data.setName(QString::fromStdString(event.participant_name()));
    participant_data.setUuid(QString::fromStdString(event.participant_uuid()));

    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_CONNECTED){
        participant_data.setOnline(true);
        if (QString::fromStdString(event.participant_site_name()) == m_siteName)
            updateOnlineParticipant(participant_data);
    }

    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_DISCONNECTED){
        participant_data.setOnline(false);
        updateOnlineParticipant(participant_data);
    }

    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_JOINED_SESSION){
        participant_data.setBusy(true);
        updateOnlineParticipant(participant_data);
    }

    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_LEFT_SESSION){
        participant_data.setBusy(false);
        updateOnlineParticipant(participant_data);
    }

    updateCounts();
}

void OnlineManagerWidget::ws_deviceEvent(DeviceEvent event)
{
    TeraData device_data(TERADATA_ONLINE_DEVICE);
    device_data.setName(QString::fromStdString(event.device_name()));
    device_data.setUuid(QString::fromStdString(event.device_uuid()));

    if (event.type() == DeviceEvent_EventType_DEVICE_CONNECTED){
        device_data.setOnline(true);
        // TODO: Filter by site?
        updateOnlineDevice(device_data);
    }

    if (event.type() == DeviceEvent_EventType_DEVICE_DISCONNECTED){
        device_data.setOnline(false);
        updateOnlineDevice(device_data);
    }

    if (event.type() == DeviceEvent_EventType_DEVICE_JOINED_SESSION){
        device_data.setBusy(true);
        updateOnlineDevice(device_data);
    }
    if (event.type() == DeviceEvent_EventType_DEVICE_LEFT_SESSION){
        device_data.setBusy(false);
        updateOnlineDevice(device_data);
    }
    updateCounts();
}

void OnlineManagerWidget::processOnlineUsers(QList<TeraData> users)
{
    foreach(TeraData online_user, users){
        updateOnlineUser(online_user);
    }
    updateCounts();
}

void OnlineManagerWidget::processOnlineParticipants(QList<TeraData> participants)
{
    foreach(TeraData online_participant, participants){
        updateOnlineParticipant(online_participant);
    }
    updateCounts();
}

void OnlineManagerWidget::processOnlineDevices(QList<TeraData> devices)
{
    foreach(TeraData online_device, devices){
        updateOnlineDevice(online_device);
    }
    updateCounts();
}

void OnlineManagerWidget::on_btnShowOnlineParticipants_clicked()
{
    ui->lstOnlineParticipants->setVisible(ui->btnShowOnlineParticipants->isChecked());
}

void OnlineManagerWidget::on_btnShowOnlineUsers_clicked()
{
    ui->lstOnlineUsers->setVisible(ui->btnShowOnlineUsers->isChecked());
}

void OnlineManagerWidget::on_btnShowOnlineDevices_clicked()
{
    ui->lstOnlineDevices->setVisible(ui->btnShowOnlineDevices->isChecked());
}

void OnlineManagerWidget::on_lstOnlineParticipants_itemClicked(QListWidgetItem *item)
{
    ui->lstOnlineDevices->clearSelection();
    ui->lstOnlineUsers->clearSelection();

    QString uuid = m_onlineParticipants.key(item);
    emit dataDisplayRequest(TERADATA_PARTICIPANT, uuid);

}

void OnlineManagerWidget::on_lstOnlineUsers_itemClicked(QListWidgetItem *item)
{
    ui->lstOnlineDevices->clearSelection();
    ui->lstOnlineParticipants->clearSelection();

    QString uuid = m_onlineUsers.key(item);
    emit dataDisplayRequest(TERADATA_USER, uuid);
}

void OnlineManagerWidget::on_lstOnlineDevices_itemClicked(QListWidgetItem *item)
{
    ui->lstOnlineParticipants->clearSelection();
    ui->lstOnlineUsers->clearSelection();

    QString uuid = m_onlineDevices.key(item);
    emit dataDisplayRequest(TERADATA_DEVICE, uuid);
}
