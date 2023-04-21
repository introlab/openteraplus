#include "SessionInviteWidget.h"
#include "ui_SessionInviteWidget.h"

#include <QDebug>

SessionInviteWidget::SessionInviteWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionInviteWidget)
{
    ui->setupUi(this);

    m_searching = false;
    m_confirmRemove = false;
    m_editable = true;
    m_comManager = nullptr;
    m_currentSessionUuid.clear();

    ui->frameSelector->hide();

    connectSignals();
}

SessionInviteWidget::~SessionInviteWidget()
{
    if (ui)
        delete ui;
}

void SessionInviteWidget::setComManager(ComManager *comMan)
{
    m_comManager = comMan;

    connect(m_comManager->getWebSocketManager(), &WebSocketManager::userEventReceived, this, &SessionInviteWidget::ws_userEvent);
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::participantEventReceived, this, &SessionInviteWidget::ws_participantEvent);
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::deviceEventReceived, this, &SessionInviteWidget::ws_deviceEvent);

    connect(m_comManager->getWebSocketManager(), &WebSocketManager::joinSessionEventReceived, this, &SessionInviteWidget::ws_joinSessionEvent);
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::leaveSessionEventReceived, this, &SessionInviteWidget::ws_leaveSessionEvent);
}

void SessionInviteWidget::setCurrentSessionUuid(const QString &session_uuid)
{
    m_currentSessionUuid = session_uuid;
}

void SessionInviteWidget::addParticipantsToSession(const QList<TeraData> &participants, const QList<int> &required_ids)
{
    // Required ids
    foreach(int required_id, required_ids){
        if (!m_requiredParticipants.contains(required_id))
            m_requiredParticipants.append(required_id);
    }

    // Participants
    foreach(TeraData participant, participants){
        // Add in session - widget will be created and updated in updateItem
        int id_item = participant.getId();
        if (!m_participantsInSession.contains(id_item))
            m_participantsInSession[id_item] = nullptr; // Set to null = create a new ListWidgetItem*

        // Update item display
        updateItem(participant);
    }

}

void SessionInviteWidget::addUsersToSession(const QList<TeraData> &users, const QList<int> &required_ids)
{
    // Users
    foreach(TeraData user, users){
        // Add in session - widget will be created and updated in updateItem
        int id_item = user.getId();
        if (!m_usersInSession.contains(id_item))
            m_usersInSession[id_item] = nullptr; // Set to null = create a new ListWidgetItem*

        // Update item display
        updateItem(user);
    }

    // Required ids
    foreach(int required_id, required_ids){
        if (!m_requiredUsers.contains(required_id))
            m_requiredUsers.append(required_id);
    }
}

void SessionInviteWidget::addDevicesToSession(const QList<TeraData> &devices, const QList<int> &required_ids)
{
    // Devices
    foreach(TeraData device, devices){
        // Add in session - widget will be created and updated in updateItem
        int id_item = device.getId();
        if (!m_devicesInSession.contains(id_item))
            m_devicesInSession[id_item] = nullptr; // Set to null = create a new ListWidgetItem*

        // Update item display
       updateItem(device);

    }

    // Required ids
    foreach(int required_id, required_ids){
        if (!m_requiredDevices.contains(required_id))
            m_requiredDevices.append(required_id);
    }
}

void SessionInviteWidget::addUserToSession(const QString &user_uuid)
{
    TeraData* data = getUserFromUuid(user_uuid);

    if (!data){
        // We don't have the information for that user yet... put in queue for when we'll have it!
        LOG_WARNING("No information for user uuid " + user_uuid, "SessionInviteWidget::addUserToSession");
        m_unknownUserUuidsInSession.append(user_uuid);
    }else{
        // Check if already there
        int id_item = data->getId();
        if (!m_usersInSession.contains(id_item))
            m_usersInSession[id_item] = nullptr; // Set to null = create a new ListWidgetItem*

        // Update item display
       updateItem(m_users[data->getId()]);
    }
}

void SessionInviteWidget::addParticipantToSession(const QString &participant_uuid)
{
    TeraData* data = getParticipantFromUuid(participant_uuid);

    if (!data){
        // We don't have the information for that user yet... put in queue for when we'll have it!
        LOG_WARNING("No information for participant uuid " + participant_uuid, "SessionInviteWidget::addParticipantToSession");
        m_unknownParticipantUuidsInSession.append(participant_uuid);
    }else{
        // Check if already there
        int id_item = data->getId();
        if (!m_participantsInSession.contains(id_item))
            m_participantsInSession[id_item] = nullptr; // Set to null = create a new ListWidgetItem*

        // Update item display
       //updateItem(m_participants[data->getId()]);
    }
}

void SessionInviteWidget::addDeviceToSession(const QString &device_uuid)
{
    TeraData* data = getDeviceFromUuid(device_uuid);

    if (!data){
        // We don't have the information for that user yet... put in queue for when we'll have it!
        LOG_WARNING("No information for device uuid " + device_uuid, "SessionInviteWidget::addDeviceToSession");
        m_unknownDeviceUuidsInSession.append(device_uuid);
    }else{
        // Check if already there
        int id_item = data->getId();
        if (!m_devicesInSession.contains(id_item))
            m_devicesInSession[id_item] = nullptr; // Set to null = create a new ListWidgetItem*

        // Update item display
       updateItem(m_devices[data->getId()]);
    }
}

void SessionInviteWidget::removeUserFromSession(const QString &user_uuid)
{
    TeraData* item_data = getUserFromUuid(user_uuid);

    if (!item_data){
        if (m_unknownUserUuidsInSession.contains(user_uuid))
            m_unknownUserUuidsInSession.removeAll(user_uuid);
        LOG_WARNING("Removed user " + user_uuid + " from session, but no information about it", "SessionInviteWidget::removeUserFromSession");
        return;
    }

    int id_item = item_data->getId();
    if (m_usersInSession.contains(id_item)){
        // Remove from ui
        QTreeWidgetItem* base_tree_item = ui->treeInvitees->topLevelItem(1);
        base_tree_item->removeChild( m_usersInSession[id_item]);
        delete m_usersInSession[id_item];
        m_usersInSession.remove(id_item);
    }

    // Update item to add it back to invitable list
    updateItem(*item_data);

    emit removedUser(user_uuid);
}

void SessionInviteWidget::removeParticipantFromSession(const QString &participant_uuid)
{
    TeraData* item_data = getParticipantFromUuid(participant_uuid);

    if (!item_data){
        if (m_unknownParticipantUuidsInSession.contains(participant_uuid))
            m_unknownParticipantUuidsInSession.removeAll(participant_uuid);
        LOG_WARNING("Removed participant " + participant_uuid + " from session, but no information about it", "SessionInviteWidget::removeParticipantFromSession");
        return;
    }

    int id_item = item_data->getId();
    if (m_participantsInSession.contains(id_item)){
        // Remove from ui
        QTreeWidgetItem* base_tree_item = ui->treeInvitees->topLevelItem(0);
        base_tree_item->removeChild( m_participantsInSession[id_item]);
        delete m_participantsInSession[id_item];
        m_participantsInSession.remove(id_item);
    }

    // Update item to add it back to invitable list
    updateItem(*item_data);

    emit removedParticipant(participant_uuid);
}

void SessionInviteWidget::removeDeviceFromSession(const QString &device_uuid)
{
    TeraData* item_data = getDeviceFromUuid(device_uuid);

    if (!item_data){
        if (m_unknownDeviceUuidsInSession.contains(device_uuid))
            m_unknownDeviceUuidsInSession.removeAll(device_uuid);
        LOG_WARNING("Removed device " + device_uuid + " from session, but no information about it", "SessionInviteWidget::removeDeviceFromSession");
        return;
    }

    int id_item = item_data->getId();
    if (m_devicesInSession.contains(id_item)){
        // Remove from ui
        QTreeWidgetItem* base_tree_item = ui->treeInvitees->topLevelItem(2);
        base_tree_item->removeChild( m_devicesInSession[id_item]);
        delete m_devicesInSession[id_item];
        m_devicesInSession.remove(id_item);
    }

    // Update item to add it back to invitable list
    updateItem(*item_data);

    emit removedDevice(device_uuid);
}

void SessionInviteWidget::setAvailableParticipants(const QList<TeraData> &participants)
{
    foreach(TeraData participant, participants){
        int id_participant = participant.getId();
        m_participants[id_participant] = participant;
        if (m_unknownParticipantUuidsInSession.contains(participant.getUuid())){
            m_participantsInSession[id_participant] = nullptr;
            m_unknownParticipantUuidsInSession.removeAll(participant.getUuid());
        }else{
            if (!m_participantsItems.contains(id_participant))
                m_participantsItems[id_participant] = nullptr;
        }
        updateItem(participant);
    }
    ui->btnParticipants->setVisible(!m_participants.isEmpty());
}

void SessionInviteWidget::setAvailableUsers(const QList<TeraData> &users)
{
    foreach(TeraData user, users){
        int id_user = user.getId();
        m_users[id_user] = user;
        if (m_unknownUserUuidsInSession.contains(user.getUuid())){
            m_usersInSession[id_user] = nullptr;
            m_unknownUserUuidsInSession.removeAll(user.getUuid());
        }else{
            if (!m_usersItems.contains(id_user))
                m_usersItems[id_user] = nullptr;
        }

        updateItem(user);
    }
    ui->btnUsers->setVisible(!m_users.isEmpty());
}

void SessionInviteWidget::setAvailableDevices(const QList<TeraData> &devices)
{
    foreach(TeraData device, devices){
        int id_device = device.getId();
        m_devices[id_device] = device;
        if (m_unknownDeviceUuidsInSession.contains(device.getUuid())){
            m_devicesInSession[id_device] = nullptr;
            m_unknownDeviceUuidsInSession.removeAll(device.getUuid());
        }else{
            if (!m_devicesItems.contains(id_device))
                m_devicesItems[id_device] = nullptr;
        }
        updateItem(device);
    }
    ui->btnDevices->setVisible(!m_devices.isEmpty());
}

bool SessionInviteWidget::hasAvailableParticipants() const
{
    return !m_participants.isEmpty();
}

bool SessionInviteWidget::hasAvailableDevices() const
{
    return !m_devices.isEmpty();
}

bool SessionInviteWidget::hasAvailableUsers() const
{
    return !m_users.isEmpty();
}

void SessionInviteWidget::addRequiredUser(const int &required_id)
{
    if (!m_requiredUsers.contains(required_id))
        m_requiredUsers.append(required_id);

    if (!m_users.contains(required_id)){
        LOG_WARNING("Tried to add user id " + QString::number(required_id) + " to required list, but no data for it!", "SessionInviteWidget::addRequiredUser");
    }else{
        if (!m_usersInSession.contains(required_id)){
            m_usersInSession[required_id] = nullptr; // Will create new TreeWidgetItem
        }
        updateItem(m_users[required_id]);
    }
}

void SessionInviteWidget::addRequiredUser(const QString &required_uuid)
{
    // Find id for UUID
    foreach(TeraData data, m_users){
        if (data.getUuid() == required_uuid){
            addRequiredUser(data.getId());
            return;
        }
    }
}

void SessionInviteWidget::addRequiredParticipant(const int &required_id)
{
    if (!m_requiredParticipants.contains(required_id))
        m_requiredParticipants.append(required_id);

    if (!m_participants.contains(required_id)){
        LOG_WARNING("Tried to add participant id " + QString::number(required_id) + " to required list, but no data for it!", "SessionInviteWidget::addRequiredParticipant");
    }else{
        if (!m_participantsInSession.contains(required_id)){
            m_participantsInSession[required_id] = nullptr; // Will create new TreeWidgetItem
        }
        //updateItem(m_participants[required_id]);
    }

}

void SessionInviteWidget::addRequiredParticipant(const QString &required_uuid)
{
    // Find id for UUID
    foreach(TeraData data, m_participants){
        if (data.getUuid() == required_uuid){
            addRequiredParticipant(data.getId());
            return;
        }
    }
}

void SessionInviteWidget::addRequiredDevice(const int &required_id)
{
    if (!m_requiredDevices.contains(required_id))
        m_requiredDevices.append(required_id);

    if (!m_devices.contains(required_id)){
        LOG_WARNING("Tried to add device id " + QString::number(required_id) + " to required list, but no data for it!", "SessionInviteWidget::addRequiredDevice");
    }else{
        if (!m_devicesInSession.contains(required_id)){
            m_devicesInSession[required_id] = nullptr; // Will create new TreeWidgetItem
        }
        updateItem(m_devices[required_id]);
    }
}

void SessionInviteWidget::setConfirmOnRemove(const bool &confirm)
{
    m_confirmRemove = confirm;
}

void SessionInviteWidget::setEditable(const bool &editable)
{
    m_editable = editable;

    ui->btnManageInvitees->setVisible(m_editable);
    ui->btnRemove->setVisible(m_editable);
}

bool SessionInviteWidget::isEditable() const
{
    return m_editable;
}

QList<TeraData> SessionInviteWidget::getParticipantsInSession()
{
    QList<TeraData> rval;
    /*foreach(int id, m_participantsInSession.keys()){
        rval.append(m_participants[id]);
    }*/
    foreach(QTreeWidgetItem* item, m_participantsInSession){
        rval.append(m_participants[m_participantsInSession.key(item)]);
    }
    return rval;
}

QList<TeraData> SessionInviteWidget::getUsersInSession()
{
    QList<TeraData> rval;
    /*foreach(int id, m_usersInSession.keys()){
        rval.append(m_users[id]);
    }*/
    foreach(QTreeWidgetItem* item, m_usersInSession){
        rval.append(m_users[m_usersInSession.key(item)]);
    }
    return rval;
}

QList<TeraData> SessionInviteWidget::getDevicesInSession()
{
    QList<TeraData> rval;
    /*foreach(int id, m_devicesInSession.keys()){
        rval.append(m_devices[id]);
    }*/
    foreach(QTreeWidgetItem* item, m_devicesInSession){
            rval.append(m_devices[m_devicesInSession.key(item)]);
        }
    return rval;
}

QStringList SessionInviteWidget::getParticipantsUuidsInSession()
{
    QStringList uuids;

    foreach(TeraData part, getParticipantsInSession()){
        uuids.append(part.getUuid());
    }
    return uuids;
}

QStringList SessionInviteWidget::getUsersUuidsInSession()
{
    QStringList uuids;

    foreach(TeraData part, getUsersInSession()){
        uuids.append(part.getUuid());
    }
    return uuids;
}

QStringList SessionInviteWidget::getDevicesUuidsInSession()
{
    QStringList uuids;

    foreach(TeraData part, getDevicesInSession()){
        uuids.append(part.getUuid());
    }
    return uuids;
}

QList<int> SessionInviteWidget::getParticipantsIdsInSession()
{
    return m_participantsInSession.keys();
}

QList<int> SessionInviteWidget::getUsersIdsInSession()
{
    return m_usersInSession.keys();
}

QList<int> SessionInviteWidget::getDevicesIdsInSession()
{
    return m_devicesInSession.keys();
}

void SessionInviteWidget::on_btnManageInvitees_clicked()
{
    ui->frameSelector->setVisible(ui->btnManageInvitees->isChecked());

    if (ui->btnManageInvitees->isChecked()){
        ui->btnManageInvitees->setIcon(QIcon("://controls/branch_opened.png"));
    }else{
        ui->btnManageInvitees->setIcon(QIcon("://controls/branch_closed.png"));
    }
}

void SessionInviteWidget::updateFilters()
{
    // TOIMPROVE: Better way to do this with a QListView instead of a QListWidget?
    for (int i=0; i<ui->lstInvitables->count(); i++){
        QListWidgetItem* item = ui->lstInvitables->item(i);
        TeraData* item_data = nullptr;
        bool hidden = false;
        if (m_usersItems.key(item) > 0){
            if (!ui->btnUsers->isChecked())
                hidden = true;
            else{
                item_data = &m_users[m_usersItems.key(item)];
            }
        }
        if (m_devicesItems.key(item) > 0){
            if (!ui->btnDevices->isChecked())
                hidden = true;
            else{
                item_data = &m_devices[m_devicesItems.key(item)];
            }
        }
        if (m_participantsItems.key(item) > 0){
            if (!ui->btnParticipants->isChecked())
                hidden = true;
            else{
                item_data = &m_participants[m_participantsItems.key(item)];
            }
        }

        if (item_data){
            if (!item_data->isOnline() && ui->btnOnline->isChecked()){
                hidden = true;
            }
        }

        if (m_searching){
            if (!item->text().contains(ui->txtSearchInvitees->text(), Qt::CaseInsensitive)){
                hidden = true;
            }
        }
        item->setHidden(hidden);
    }
}

void SessionInviteWidget::selectDefaultFilter()
{
    // Remove invalid combination
    if (ui->btnDevices->isChecked() && m_devices.isEmpty())
        ui->btnDevices->setChecked(false);
    if (ui->btnParticipants->isChecked() && m_participants.isEmpty())
        ui->btnParticipants->setChecked(false);
    if (ui->btnUsers->isChecked() && m_users.isEmpty())
        ui->btnUsers->setChecked(false);

    if (ui->btnDevices->isChecked() || ui->btnParticipants->isChecked() || ui->btnUsers->isChecked())
        return; // At least one selected, no need to.

    if (!m_participants.isEmpty())
        ui->btnParticipants->setChecked(true);
    else if(!m_users.isEmpty())
        ui->btnUsers->setChecked(true);
    else if(!m_devices.isEmpty())
        ui->btnDevices->setChecked(true);

    updateFilters();
}

void SessionInviteWidget::autoSelectFilters()
{
    ui->btnParticipants->setChecked(!m_participants.isEmpty());
    ui->btnUsers->setChecked(!m_users.isEmpty());
    ui->btnDevices->setChecked(!m_devices.isEmpty());

    updateFilters();
}

void SessionInviteWidget::selectFilterParticipant()
{
    ui->btnParticipants->setChecked(true);
    updateFilters();
}

void SessionInviteWidget::showAvailableInvitees(const bool &show, const bool &hide_button)
{
    ui->btnManageInvitees->setChecked(show);
    on_btnManageInvitees_clicked();

    ui->btnManageInvitees->setHidden(hide_button);
}

void SessionInviteWidget::showOnlineFilter(const bool &show)
{
    ui->btnOnline->setVisible(show);
}

void SessionInviteWidget::ws_userEvent(UserEvent event)
{
    QString uuid = QString::fromStdString(event.user_uuid());

    TeraData* data = getUserFromUuid(uuid);

    if (!data)
        return; // Item not found, we don't have to manage it here!

    if (event.type() == UserEvent_EventType_USER_CONNECTED){
        data->setOnline(true);
    }
    if (event.type() == UserEvent_EventType_USER_DISCONNECTED){
        data->setOnline(false);
    }
    if (event.type() == UserEvent_EventType_USER_JOINED_SESSION){
        data->setBusy(true);
    }
    if (event.type() == UserEvent_EventType_USER_LEFT_SESSION){
        data->setBusy(false);
    }

    updateItem(*data);

}

void SessionInviteWidget::ws_participantEvent(ParticipantEvent event)
{
    QString uuid = QString::fromStdString(event.participant_uuid());

    TeraData* data = getParticipantFromUuid(uuid);

    if (!data)
        return; // Item not found, we don't have to manage it here!

    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_CONNECTED){
        data->setOnline(true);
    }
    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_DISCONNECTED){
        data->setOnline(false);
    }
    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_JOINED_SESSION){
        data->setBusy(true);
    }
    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_LEFT_SESSION){
        data->setBusy(false);
    }

    updateItem(*data);
}

void SessionInviteWidget::ws_deviceEvent(DeviceEvent event)
{
    QString uuid = QString::fromStdString(event.device_uuid());

    TeraData* data = getDeviceFromUuid(uuid);

    if (!data)
        return; // Item not found, we don't have to manage it here!

    if (event.type() == DeviceEvent_EventType_DEVICE_CONNECTED){
        data->setOnline(true);
    }
    if (event.type() == DeviceEvent_EventType_DEVICE_DISCONNECTED){
        data->setOnline(false);
    }
    if (event.type() == DeviceEvent_EventType_DEVICE_JOINED_SESSION){
        data->setBusy(true);
    }
    if (event.type() == DeviceEvent_EventType_DEVICE_LEFT_SESSION){
        data->setBusy(false);
    }

    updateItem(*data);
}

void SessionInviteWidget::ws_joinSessionEvent(JoinSessionEvent event)
{
    if (m_currentSessionUuid.isEmpty()){
        LOG_WARNING("Received JoinSessionEvent, but no session uuid was setted beforehand - ignoring event", "SessionInviteWidget::ws_joinSessionEvent");
        return;
    }

    if (m_currentSessionUuid != QString::fromStdString(event.session_uuid()))
        return; // Not for us!

    // Add invitees to list if not already there
    for (int i=0; i<event.session_users_size(); i++){
        addUserToSession(QString::fromStdString(event.session_users(i)));
    }
    for (int i=0; i<event.session_participants_size(); i++){
        addParticipantToSession(QString::fromStdString(event.session_participants(i)));
    }
    for (int i=0; i<event.session_devices_size(); i++){
        addDeviceToSession(QString::fromStdString(event.session_devices(i)));
    }
}

void SessionInviteWidget::ws_leaveSessionEvent(LeaveSessionEvent event)
{
    if (m_currentSessionUuid.isEmpty()){
        LOG_WARNING("Received LeaveSessionEvent, but no session uuid was setted beforehand - ignoring event", "SessionInviteWidget::ws_leaveSessionEvent");
        return;
    }

    if (m_currentSessionUuid != QString::fromStdString(event.session_uuid()))
        return; // Not for us!

    // Remove all leaving invitees
    for (int i=0; i<event.leaving_users_size(); i++){
        removeUserFromSession(QString::fromStdString(event.leaving_users(i)));
    }
    for (int i=0; i<event.leaving_participants_size(); i++){
        removeParticipantFromSession(QString::fromStdString(event.leaving_participants(i)));
    }
    for (int i=0; i<event.leaving_devices_size(); i++){
        removeDeviceFromSession(QString::fromStdString(event.leaving_devices(i)));
    }
}

void SessionInviteWidget::connectSignals()
{
    connect(ui->btnDevices, &QToolButton::clicked, this, &SessionInviteWidget::updateFilters);
    connect(ui->btnParticipants, &QToolButton::clicked, this, &SessionInviteWidget::updateFilters);
    connect(ui->btnUsers, &QToolButton::clicked, this, &SessionInviteWidget::updateFilters);
    connect(ui->btnOnline, &QToolButton::clicked, this, &SessionInviteWidget::updateFilters);
}

void SessionInviteWidget::updateItem(const TeraData &item)
{
    // Get pointers to correct structures depending on item type
    //QHash<int, TeraData>*           item_data;
    QHash<int, QListWidgetItem*>*   item_availables;
    QHash<int, QTreeWidgetItem*>*   item_invitees;
    QTreeWidgetItem*                base_tree_item;
    QList<int>*                     item_required;

    if (item.getDataType() == TERADATA_PARTICIPANT){
        //item_data = &m_participants;
        item_availables = &m_participantsItems;
        item_invitees = &m_participantsInSession;
        item_required = &m_requiredParticipants;
        base_tree_item = ui->treeInvitees->topLevelItem(0);
    }else if(item.getDataType() == TERADATA_USER){
        //item_data = &m_users;
        item_availables = &m_usersItems;
        item_invitees = &m_usersInSession;
        item_required = &m_requiredUsers;
        base_tree_item = ui->treeInvitees->topLevelItem(1);
    }else if(item.getDataType() == TERADATA_DEVICE){
        //item_data = &m_devices;
        item_availables = &m_devicesItems;
        item_invitees = &m_devicesInSession;
        item_required = &m_requiredDevices;
        base_tree_item = ui->treeInvitees->topLevelItem(2);
    }else{
        qWarning("SessionInviteWidget::updateItem - Wrong data type!!");
        return;
    }

    int id_item = item.getId();
    // Check in which list the item is
    bool invited = item_invitees->contains(id_item) || item_required->contains(id_item);

    // qDebug() << item.getName() << " - " << item.getDataTypeName(item.getDataType()) << " ID " << id_item << ": " << invited;

    if (invited){
        // Item is in the invited list
        QTreeWidgetItem* tree_item = nullptr;
        if (item_invitees->contains(id_item) && (*item_invitees)[id_item] != nullptr){
            tree_item = (*item_invitees)[id_item];
        }else{
            tree_item = new QTreeWidgetItem(base_tree_item);
            item_invitees->insert(id_item, tree_item);
        }

        // Update informations
        tree_item->setText(0, item.getName()/* + " [" + QString::number(id_item) + "]"*/);
        tree_item->setIcon(0, QIcon(item.getIconStateFilename()));

        tree_item->setDisabled(item_required->contains(id_item)); // Required, so can't be removed!

        // Check if super admin, and hide if required
        if(item.getDataType() == TERADATA_USER){
            if (item.hasFieldName("user_superadmin"))
                if (item.getFieldValue("user_superadmin").toBool() == true && !m_comManager->isCurrentUserSuperAdmin()){
                    tree_item->setHidden(true);
                }
        }

        // Make sure item isn't in available list anymore
        if (item_availables->contains(id_item)){
            delete ui->lstInvitables->takeItem(ui->lstInvitables->row((*item_availables)[id_item]));
            item_availables->remove(id_item);
        }

    }else{
        // Item is in the available list
        QListWidgetItem* list_item = nullptr;
        if (item_availables->contains(id_item) && (*item_availables)[id_item] != nullptr){
            list_item = (*item_availables)[id_item];
        }else{
            list_item = new QListWidgetItem();
            item_availables->insert(id_item, list_item);
        }

        // Update informations
        list_item->setText(item.getName());
        list_item->setIcon(QIcon(item.getIconStateFilename()));
        ui->lstInvitables->addItem(list_item);

        // Check if super admin, and hide if required
        if(item.getDataType() == TERADATA_USER){
            if (item.hasFieldName("user_superadmin"))
                if (item.getFieldValue("user_superadmin").toBool() == true && !m_comManager->isCurrentUserSuperAdmin()){
                    list_item->setHidden(true);
                }
        }

        // Make sure item isn't in invited list anymore
        if (item_invitees->contains(id_item)){
            base_tree_item->removeChild( (*item_invitees)[id_item]);
            delete (*item_invitees)[id_item];
            item_invitees->remove(id_item);
        }
    }

    // Refresh filters for available items
    updateFilters();

    // Refresh in session widget (hide empty items and expand)
    refreshInSessionTreeWidget();

}

void SessionInviteWidget::refreshInSessionTreeWidget()
{
    // Hide empty top level items
    for(int i=0; i<ui->treeInvitees->topLevelItemCount(); i++){
        if (ui->treeInvitees->topLevelItem(i)->childCount() == 0){
           ui->treeInvitees->topLevelItem(i)->setHidden(true);
        }else{
           ui->treeInvitees->topLevelItem(i)->setHidden(false);
        }
    }

    // Always expand items
    ui->treeInvitees->expandAll();

    // Update count label
    ui->lblCount->setText(QString::number(getInviteesCount()) + " / " + QString::number(MAX_INVITEES_IN_SESSION));

}

quint8 SessionInviteWidget::getInviteesCount()
{
    return m_usersInSession.count() + m_participantsInSession.count() + m_devicesInSession.count();
}

quint8 SessionInviteWidget::getParticipantsInSessionCount()
{
    return m_participantsInSession.count();
}

quint8 SessionInviteWidget::getUsersInSessionCount()
{
    return m_usersInSession.count();
}

quint8 SessionInviteWidget::getDevicesInSessionCount()
{
    return m_devicesInSession.count();
}

TeraData *SessionInviteWidget::getUserFromUuid(const QString &uuid)
{
    TeraData* data = nullptr;

    foreach(TeraData user, m_users){
        if (user.getUuid() == uuid){
            data = &m_users[m_users.key(user)];
        }
    }
    /*for (int i=0; i<m_users.values().count(); i++){
        if (m_users.values().at(i).getUuid() == uuid){
            data = &m_users[m_users.keys().at(i)];
            break;
        }
    }*/

    return data;
}

TeraData *SessionInviteWidget::getParticipantFromUuid(const QString &uuid)
{
    TeraData* data = nullptr;

    foreach(TeraData part, m_participants){
        if (part.getUuid() == uuid){
            data = &m_participants[m_participants.key(part)];
        }
    }
    /*for (int i=0; i<m_participants.values().count(); i++){
        if (m_participants.values().at(i).getUuid() == uuid){
            data = &m_participants[m_participants.keys().at(i)];
            break;
        }
    }*/

    return data;
}

TeraData *SessionInviteWidget::getDeviceFromUuid(const QString &uuid)
{
    TeraData* data = nullptr;

    foreach(TeraData device, m_devices){
        if (device.getUuid() == uuid){
            data = &m_devices[m_devices.key(device)];
        }
    }
    /*for (int i=0; i<m_devices.values().count(); i++){
        if (m_devices.values().at(i).getUuid() == uuid){
            data = &m_devices[m_devices.keys().at(i)];
            break;
        }
    }*/

    return data;
}

void SessionInviteWidget::setSearching(const bool &search)
{

    if (search){
        m_searching = true;
        //ui->txtSearchInvitees->setStyleSheet("color:white;");
        //ui->btnClear->setVisible(true);
    }else{
        m_searching=false;
        //ui->txtSearchInvitees->setStyleSheet("color:rgba(255,255,255,50%);");
        //ui->btnClear->setVisible(false);
    }

}

void SessionInviteWidget::on_btnInvite_clicked()
{
    // Check if we are under the allowed maximum number of invitees in a session
    if (getInviteesCount() + ui->lstInvitables->selectedItems().count() > MAX_INVITEES_IN_SESSION){
        GlobalMessageBox msgbox;
        msgbox.showError(tr("Nombre maximal d'invités atteint"), tr("Impossible d'ajouter ces invités à la séance: le nombre maximal de participants (") + QString::number(MAX_INVITEES_IN_SESSION) + ") " + tr("serait dépassé"));
        return;
    }

    // Add all items to the session
    QStringList invited_users;
    QStringList invited_participants;
    QStringList invited_devices;

    foreach(QListWidgetItem* item, ui->lstInvitables->selectedItems()){
        TeraData* item_data = nullptr;
        if (m_usersItems.key(item) > 0){
            item_data = &(m_users[m_usersItems.key(item)]);
            m_usersInSession[item_data->getId()] = nullptr;

            invited_users.append(item_data->getUuid());
        }
        if (m_devicesItems.key(item) > 0){
            item_data = &(m_devices[m_devicesItems.key(item)]);
            m_devicesInSession[item_data->getId()] = nullptr;

            invited_devices.append(item_data->getUuid());
        }
        if (m_participantsItems.key(item) > 0){
            item_data = &(m_participants[m_participantsItems.key(item)]);
            m_participantsInSession[item_data->getId()] = nullptr;

            invited_participants.append(item_data->getUuid());
        }
        if (item_data)
            updateItem(*item_data);
    }

    // Signals
    emit newInvitees(invited_users, invited_participants, invited_devices);
}

void SessionInviteWidget::on_lstInvitables_itemSelectionChanged()
{
    ui->btnInvite->setEnabled(ui->lstInvitables->selectedItems().count()>0);
}

void SessionInviteWidget::on_lstInvitables_itemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item)
    on_btnInvite_clicked();
}

void SessionInviteWidget::on_treeInvitees_itemSelectionChanged()
{
    foreach(QTreeWidgetItem* item, ui->treeInvitees->selectedItems()){
        // Don't enable remove button on toplevel items (categories)
        if (item->parent()){
            ui->btnRemove->setEnabled(true);
            return;
        }
    }
    ui->btnRemove->setEnabled(false);
}

void SessionInviteWidget::on_treeInvitees_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(item)
    Q_UNUSED(column)
    if (!item->isDisabled())
        on_btnRemove_clicked();
}

void SessionInviteWidget::on_btnRemove_clicked()
{
    if (!m_editable)
        return;

    if (m_confirmRemove){
        GlobalMessageBox msg;
        if (msg.showYesNo(tr("Confirmation"), tr("Êtes-vous sûrs de vouloir retirer les invités sélectionnés de la séance?")) != QMessageBox::Yes){
            return;
        }
    }

    // Remove all selected item from the session
    QStringList removed_users;
    QStringList removed_devices;
    QStringList removed_participants;
    foreach(QTreeWidgetItem* item, ui->treeInvitees->selectedItems()){
        TeraData* item_data = nullptr;
        if (m_usersInSession.key(item) > 0){
            item_data = &(m_users[m_usersInSession.key(item)]);
            m_usersInSession.remove(item_data->getId());
            removed_users.append(item_data->getUuid());
        }
        if (m_devicesInSession.key(item) > 0){
            item_data = &(m_devices[m_devicesInSession.key(item)]);
            m_devicesInSession.remove(item_data->getId());
            removed_devices.append(item_data->getUuid());
        }
        if (m_participantsInSession.key(item) > 0){
            item_data = &(m_participants[m_participantsInSession.key(item)]);
            m_participantsInSession.remove(item_data->getId());
            removed_participants.append(item_data->getUuid());
        }

        // Remove from view
        item->parent()->removeChild(item);
        delete item;

        // Update item to add it back to invitable list
        if (item_data)
            updateItem(*item_data);
    }

    // Emit signals
    emit removedInvitees(removed_users, removed_participants, removed_devices);
}

void SessionInviteWidget::on_txtSearchInvitees_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    // Check if search field is empty
    setSearching(ui->txtSearchInvitees->text().size()>0);
    updateFilters();
}
