#include "OnlineManagerWidget.h"
#include "ui_OnlineManagerWidget.h"
#include "TeraSettings.h"
#include "GlobalMessageBox.h"

// Must be reset each time a site is changed.
// 1. Query online * using APIs
// 2. React to events using protobuf messages
//

OnlineManagerWidget::OnlineManagerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OnlineManagerWidget)
{

    m_comManager = nullptr;
    m_baseDevices = nullptr;
    m_baseUsers = nullptr;
    m_baseParticipants = nullptr;
    m_actionsMenu = nullptr;
    ui->setupUi(this);


}

OnlineManagerWidget::~OnlineManagerWidget()
{
    delete ui;
}

void OnlineManagerWidget::setComManager(ComManager *comMan)
{
    initUi();

    if (m_comManager)
        m_comManager->deleteLater();

    m_comManager = comMan;

    connectSignals();

    // Update filter buttons state
    bool filter = TeraSettings::getUserSetting(m_comManager->getCurrentUser().getUuid(), SETTINGS_UI_ONLINEFILTERDEVICES).toBool();
    ui->btnFilterDevices->setChecked(filter);
    filter = TeraSettings::getUserSetting(m_comManager->getCurrentUser().getUuid(), SETTINGS_UI_ONLINEFILTERUSERS).toBool();
    ui->btnFilterUsers->setChecked(filter);
    filter = TeraSettings::getUserSetting(m_comManager->getCurrentUser().getUuid(), SETTINGS_UI_ONLINEFILTERPARTICIPANTS).toBool();
    ui->btnFilterParticipants->setChecked(filter);

    // If all off, select at least users and participants
    if (!ui->btnFilterDevices->isChecked() && !ui->btnFilterParticipants->isChecked() && !ui->btnFilterUsers->isChecked()){
        ui->btnFilterParticipants->setChecked(true);
        ui->btnFilterUsers->setChecked(true);
    }

    // Update filtering state
    on_btnFilterParticipants_clicked();
    on_btnFilterUsers_clicked();
    on_btnFilterDevices_clicked();
    currentUserWasUpdated();
}

void OnlineManagerWidget::setCurrentSite(const QString &site_name, const int &site_id)
{
    m_siteName = site_name;
    m_siteId = site_id;

    refreshOnlines();
}

void OnlineManagerWidget::initUi()
{
    ui->treeOnline->clear();
    ui->treeOnline->setContextMenuPolicy(Qt::CustomContextMenu);

    m_baseParticipants = new QTreeWidgetItem();
    m_baseParticipants->setText(0, tr("Participants"));
    m_baseParticipants->setIcon(0, QIcon("://icons/patient.png"));
    m_baseParticipants->setForeground(0, Qt::cyan);
    ui->treeOnline->addTopLevelItem(m_baseParticipants);

    m_baseUsers = new QTreeWidgetItem();
    m_baseUsers->setText(0, tr("Utilisateurs"));
    m_baseUsers->setIcon(0, QIcon("://icons/software_user.png"));
    m_baseUsers->setForeground(0, Qt::cyan);
    ui->treeOnline->addTopLevelItem(m_baseUsers);

    m_baseDevices = new QTreeWidgetItem();
    m_baseDevices->setText(0, tr("Appareils"));
    m_baseDevices->setIcon(0, QIcon("://icons/device.png"));
    m_baseDevices->setForeground(0, Qt::cyan);
    ui->treeOnline->addTopLevelItem(m_baseDevices);

    ui->treeOnline->expandAll();

}

void OnlineManagerWidget::connectSignals()
{
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::userEventReceived, this, &OnlineManagerWidget::ws_userEvent);
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::participantEventReceived, this, &OnlineManagerWidget::ws_participantEvent);
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::deviceEventReceived, this, &OnlineManagerWidget::ws_deviceEvent);

    connect(m_comManager, &ComManager::onlineDevicesReceived, this, &OnlineManagerWidget::processOnlineDevices);
    connect(m_comManager, &ComManager::onlineParticipantsReceived, this, &OnlineManagerWidget::processOnlineParticipants);
    connect(m_comManager, &ComManager::onlineUsersReceived, this, &OnlineManagerWidget::processOnlineUsers);

    connect(m_comManager, &ComManager::currentUserUpdated, this, &OnlineManagerWidget::currentUserWasUpdated);
}

void OnlineManagerWidget::refreshOnlines()
{
    // Clear online list
    qDeleteAll(m_onlineDevices);
    qDeleteAll(m_onlineUsers);
    qDeleteAll(m_onlineParticipants);

    m_onlineUsers.clear();
    m_onlineDevices.clear();
    m_onlineParticipants.clear();
    m_onlineUsersData.clear();
    m_onlineDevicesData.clear();
    m_onlineParticipantsData.clear();

    QUrlQuery args;
    m_comManager->doGet(WEB_ONLINEDEVICEINFO_PATH, args);
    m_comManager->doGet(WEB_ONLINEPARTICIPANTINFO_PATH, args);
    m_comManager->doGet(WEB_ONLINEUSERINFO_PATH, args);

}

void OnlineManagerWidget::updateCounts()
{
    ui->lblDevicesNum->setText(QString::number(m_onlineDevices.count()));
    ui->lblParticipantsNum->setText(QString::number(m_onlineParticipants.count()));
    ui->lblUsersNum->setText(QString::number(m_onlineUsers.count()));

    emit totalCountUpdated(m_onlineDevices.count() + m_onlineParticipants.count() + m_onlineUsers.count());
}

void OnlineManagerWidget::updateOnlineUser(const TeraData* online_user)
{

    QString uuid = online_user->getUuid();

    QTreeWidgetItem* user_item = nullptr;

    if (m_onlineUsers.contains(uuid)){
        user_item = m_onlineUsers[uuid];
    }else{
        if (online_user->isOnline()/* || online_user.isBusy()*/){ // Not online and not busy = don't need to add!
            user_item = new QTreeWidgetItem(m_baseUsers);
            user_item->setIcon(0, QIcon(online_user->getIconStateFilename()));
            user_item->setText(0, online_user->getName());
#ifdef QT_DEBUG
            user_item->setToolTip(0, uuid);
#endif
            m_onlineUsers[uuid] = user_item;
        }
    }

    if (user_item){
        if (!online_user->isOnline() && !online_user->isBusy()){
            // We must remove that item
            delete user_item;
            m_onlineUsers.remove(uuid);
            m_onlineUsersData.remove(uuid);
        }else{
            // Update name / icon if needed
            /*if (!online_user->getName().isEmpty())
                user_item->setText(online_user->getName());*/
            user_item->setIcon(0, QIcon(online_user->getIconStateFilename()));

            // Resort items
            //ui->lstOnline->sortItems();
            m_baseUsers->sortChildren(0, Qt::AscendingOrder);
        }
    }
}

void OnlineManagerWidget::updateOnlineParticipant(const TeraData *online_participant)
{
    QString uuid = online_participant->getUuid();

    QTreeWidgetItem* participant_item = nullptr;

    if (m_onlineParticipants.contains(uuid)){
        participant_item = m_onlineParticipants[uuid];
    }else{
        if (online_participant->isOnline()/* || online_participant.isBusy()*/){ // Not online and not busy = don't need to add!
            participant_item = new QTreeWidgetItem(m_baseParticipants);
            participant_item->setIcon(0, QIcon(online_participant->getIconStateFilename()));
            participant_item->setText(0, online_participant->getName());
#ifdef QT_DEBUG
            participant_item->setToolTip(0, uuid);
#endif
            m_onlineParticipants[uuid] = participant_item;
        }
    }

    if (participant_item){
        if (!online_participant->isOnline() && !online_participant->isBusy()){
            // We must remove that item
            delete participant_item;
            m_onlineParticipants.remove(uuid);
            m_onlineParticipantsData.remove(uuid);
        }else{
            // Update name / icon if needed
            /*if (!online_participant->getName().isEmpty())
                participant_item->setText(online_participant->getName());*/
            participant_item->setIcon(0, QIcon(online_participant->getIconStateFilename()));

            // Resort items
           // ui->lstOnline->sortItems();
            m_baseParticipants->sortChildren(0, Qt::AscendingOrder);
        }
    }
}

void OnlineManagerWidget::updateOnlineDevice(const TeraData *online_device)
{
    QString uuid = online_device->getUuid();

    QTreeWidgetItem* device_item = nullptr;

    if (m_onlineDevices.contains(uuid)){
        device_item = m_onlineDevices[uuid];
    }else{
        if (online_device->isOnline()/* || online_device.isBusy()*/){ // Not online and not busy = don't need to add!
            device_item = new QTreeWidgetItem(m_baseDevices);
            device_item->setIcon(0,QIcon(online_device->getIconStateFilename()));
            device_item->setText(0, online_device->getName());
#ifdef QT_DEBUG
            device_item->setToolTip(0, uuid);
#endif
            //ui->lstOnline->addItem(device_item);
            m_onlineDevices[uuid] = device_item;
        }
    }

    if (device_item){
        if (!online_device->isOnline() && !online_device->isBusy()){
            // We must remove that item
            delete device_item;
            m_onlineDevices.remove(uuid);
            m_onlineDevicesData.remove(uuid);
        }else{
            // Update name / icon if needed
            /*if (!online_device->getName().isEmpty())
                device_item->setText(online_device->getName());*/
            device_item->setIcon(0, QIcon(online_device->getIconStateFilename()));

            // Resort items
           // ui->lstOnline->sortItems();
            m_baseDevices->sortChildren(0, Qt::AscendingOrder);
        }
    }
}

void OnlineManagerWidget::createOnlineUser(const QString &uuid, const QString &name)
{
    TeraData new_data(TERADATA_ONLINE_USER);
    new_data.setName(name);
    new_data.setUuid(uuid);
    m_onlineUsersData[uuid] = new_data;
}

void OnlineManagerWidget::createOnlineParticipant(const QString& uuid, const QString& name)
{
    TeraData new_data(TERADATA_ONLINE_PARTICIPANT);
    new_data.setName(name);
    new_data.setUuid(uuid);
    m_onlineParticipantsData[uuid] = new_data;
}

void OnlineManagerWidget::createOnlineDevice(const QString &uuid, const QString &name)
{
    TeraData new_data(TERADATA_ONLINE_DEVICE);
    new_data.setName(name);
    new_data.setUuid(uuid);
    m_onlineDevicesData[uuid] = new_data;
}

void OnlineManagerWidget::ws_userEvent(UserEvent event)
{
    QString uuid = QString::fromStdString(event.user_uuid());

    if (!m_onlineUsersData.contains(uuid)){
       createOnlineUser(uuid, QString::fromStdString(event.user_fullname()));
    }
    TeraData* user_data = &m_onlineUsersData[uuid];

    if (event.type() == UserEvent_EventType_USER_CONNECTED){
        user_data->setOnline(true);
        updateOnlineUser(user_data);
    }
    if (event.type() == UserEvent_EventType_USER_DISCONNECTED){
        user_data->setOnline(false);
        updateOnlineUser(user_data);
    }
    if (event.type() == UserEvent_EventType_USER_JOINED_SESSION){
        user_data->setBusy(true);
        updateOnlineUser(user_data);
    }
    if (event.type() == UserEvent_EventType_USER_LEFT_SESSION){
        user_data->setBusy(false);
        updateOnlineUser(user_data);
    }
    updateCounts();

}

void OnlineManagerWidget::ws_participantEvent(ParticipantEvent event)
{

    QString uuid = QString::fromStdString(event.participant_uuid());

    /*if (QString::fromStdString(event.participant_site_name()) != m_siteName)
        LOG_DEBUG("Ignoring participant " + QString::fromStdString(event.participant_name()) + " - not for the current site.", "OnlineManagerWidget::ws_participantEvent");
        return;*/

    if (!m_onlineParticipantsData.contains(uuid)){
       createOnlineParticipant(uuid, QString::fromStdString(event.participant_name()));
    }
    TeraData* participant_data = &m_onlineParticipantsData[uuid];

    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_CONNECTED){
        participant_data->setOnline(true);
        updateOnlineParticipant(participant_data);
    }

    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_DISCONNECTED){
        participant_data->setOnline(false);
        updateOnlineParticipant(participant_data);
    }

    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_JOINED_SESSION){
        participant_data->setBusy(true);
        updateOnlineParticipant(participant_data);
    }

    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_LEFT_SESSION){
        participant_data->setBusy(false);
        updateOnlineParticipant(participant_data);
    }

    updateCounts();
}

void OnlineManagerWidget::ws_deviceEvent(DeviceEvent event)
{

    QString uuid = QString::fromStdString(event.device_uuid());

    if (!m_onlineDevicesData.contains(uuid)){
       createOnlineDevice(uuid, QString::fromStdString(event.device_name()));
    }
    TeraData* device_data = &m_onlineDevicesData[uuid];

    if (event.type() == DeviceEvent_EventType_DEVICE_CONNECTED){
        device_data->setOnline(true);
        // TODO: Filter by site?
        updateOnlineDevice(device_data);
    }

    if (event.type() == DeviceEvent_EventType_DEVICE_DISCONNECTED){
        device_data->setOnline(false);
        updateOnlineDevice(device_data);
    }

    if (event.type() == DeviceEvent_EventType_DEVICE_JOINED_SESSION){
        device_data->setBusy(true);
        updateOnlineDevice(device_data);
    }
    if (event.type() == DeviceEvent_EventType_DEVICE_LEFT_SESSION){
        device_data->setBusy(false);
        updateOnlineDevice(device_data);
    }
    updateCounts();
}

void OnlineManagerWidget::processOnlineUsers(QList<TeraData> users)
{
    foreach(TeraData online_user, users){
        if (!m_onlineUsersData.contains(online_user.getUuid())){
           createOnlineUser(online_user.getUuid(), online_user.getName());
        }
        TeraData* user_data = &m_onlineUsersData[online_user.getUuid()];
        user_data->setOnline(online_user.isOnline());
        user_data->setBusy(online_user.isBusy());
        updateOnlineUser(user_data);
    }
    updateCounts();
}

void OnlineManagerWidget::processOnlineParticipants(QList<TeraData> participants)
{
    foreach(TeraData online_participant, participants){
        if (!m_onlineParticipantsData.contains(online_participant.getUuid())){
           createOnlineParticipant(online_participant.getUuid(), online_participant.getName());
        }
        TeraData* part_data = &m_onlineParticipantsData[online_participant.getUuid()];
        part_data->setOnline(online_participant.isOnline());
        part_data->setBusy(online_participant.isBusy());
        updateOnlineParticipant(part_data);
    }
    updateCounts();
}

void OnlineManagerWidget::processOnlineDevices(QList<TeraData> devices)
{
    foreach(TeraData online_device, devices){
        if (!m_onlineDevicesData.contains(online_device.getUuid())){
           createOnlineDevice(online_device.getUuid(), online_device.getName());
        }
        TeraData* device_data = &m_onlineDevicesData[online_device.getUuid()];
        device_data->setOnline(online_device.isOnline());
        device_data->setBusy(online_device.isBusy());
        updateOnlineDevice(device_data);
    }
    updateCounts();
}

void OnlineManagerWidget::currentUserWasUpdated()
{
    if (m_comManager->isCurrentUserSuperAdmin()){
        if (!m_actionsMenu){
            m_actionsMenu = new QMenu(this);

            QAction* disconnectAction = new QAction(QIcon(":/icons/delete_old.png"), tr("Déconnecter"), m_actionsMenu);
            connect(disconnectAction, &QAction::triggered, this, &OnlineManagerWidget::disconnectItemRequested);
            m_actionsMenu->addAction(disconnectAction);

        }
    }
}

void OnlineManagerWidget::on_treeOnline_itemClicked(QTreeWidgetItem *item, int column)
{
    if (!item || item == m_baseDevices || item == m_baseParticipants || item == m_baseUsers){
        return;
    }

    item->setSelected(false);
    QString uuid;

    uuid = m_onlineDevices.key(item, QString());
    if (!uuid.isEmpty()){
        emit dataDisplayRequest(TERADATA_DEVICE, uuid);
        return;
    }

    uuid = m_onlineUsers.key(item, QString());
    if (!uuid.isEmpty()){
        emit dataDisplayRequest(TERADATA_USER, uuid);
        return;
    }

    uuid = m_onlineParticipants.key(item, QString());
    if (!uuid.isEmpty()){
        emit dataDisplayRequest(TERADATA_PARTICIPANT, uuid);
        return;
    }
}


void OnlineManagerWidget::on_btnFilterParticipants_clicked()
{
    m_baseParticipants->setHidden(!ui->btnFilterParticipants->isChecked());
    // Save new setting
    if (m_comManager)
        TeraSettings::setUserSetting(m_comManager->getCurrentUser().getUuid(), SETTINGS_UI_ONLINEFILTERPARTICIPANTS, ui->btnFilterParticipants->isChecked());
}


void OnlineManagerWidget::on_btnFilterUsers_clicked()
{
    m_baseUsers->setHidden(!ui->btnFilterUsers->isChecked());
    // Save new setting
    if (m_comManager)
        TeraSettings::setUserSetting(m_comManager->getCurrentUser().getUuid(), SETTINGS_UI_ONLINEFILTERUSERS, ui->btnFilterUsers->isChecked());
}


void OnlineManagerWidget::on_btnFilterDevices_clicked()
{
    m_baseDevices->setHidden(!ui->btnFilterDevices->isChecked());
    // Save new setting
    if (m_comManager)
        TeraSettings::setUserSetting(m_comManager->getCurrentUser().getUuid(), SETTINGS_UI_ONLINEFILTERDEVICES, ui->btnFilterDevices->isChecked());
}

void OnlineManagerWidget::on_treeOnline_customContextMenuRequested(const QPoint &pos)
{
    if (!m_actionsMenu)
        return;

    QTreeWidgetItem* pointed_item = ui->treeOnline->itemAt(pos);
    if (!pointed_item){
        return;
    }

    if (!pointed_item->parent())
        return; // Top level item = category = no action here.

    // Get item uuid
    QString uuid;
    if (!m_onlineDevices.key(pointed_item).isEmpty()){
        uuid = m_onlineDevices.key(pointed_item);
    }
    if (!m_onlineUsers.key(pointed_item).isEmpty()){
        uuid = m_onlineUsers.key(pointed_item);
    }
    if (!m_onlineParticipants.key(pointed_item).isEmpty()){
        uuid = m_onlineParticipants.key(pointed_item);
    }

    if (uuid == m_comManager->getCurrentUser().getUuid())
        return; // Ourselves - can't disconnect!!

    m_actionsMenu->setProperty("uuid", uuid);

    m_actionsMenu->exec(ui->treeOnline->mapToGlobal(pos));
}

void OnlineManagerWidget::disconnectItemRequested()
{
    QString uuid = m_actionsMenu->property("uuid").toString();
    QUrlQuery args;
    QString name;

    if (m_onlineDevices.contains(uuid)){
        name = m_onlineDevices[uuid]->text(0);
        args.addQueryItem(WEB_QUERY_UUID_DEVICE, uuid);
    }
    if (m_onlineParticipants.contains(uuid)){
        name = m_onlineParticipants[uuid]->text(0);
        args.addQueryItem(WEB_QUERY_UUID_PARTICIPANT, uuid);
    }
    if (m_onlineUsers.contains(uuid)){
        name = m_onlineUsers[uuid]->text(0);
        args.addQueryItem(WEB_QUERY_UUID_USER, uuid);
    }

    GlobalMessageBox msg;
    if (msg.showYesNo(tr("Déconnecter?"), tr("Êtes-vous sûrs de vouloir déconnecter") + " " + name + "?") == GlobalMessageBox::Yes){
        m_comManager->doGet(WEB_DISCONNECT_PATH, args);
    }
}

