#include "SessionInviteWidget.h"
#include "ui_SessionInviteWidget.h"

#include <QDebug>

SessionInviteWidget::SessionInviteWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionInviteWidget)
{
    ui->setupUi(this);

    m_searching = false;
    m_comManager = nullptr;

    ui->frameSelector->hide();

    connectSignals();
}

SessionInviteWidget::~SessionInviteWidget()
{
    delete ui;
}

void SessionInviteWidget::setComManager(ComManager *comMan)
{
    m_comManager = comMan;

    connect(m_comManager->getWebSocketManager(), &WebSocketManager::userEventReceived, this, &SessionInviteWidget::ws_userEvent);
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::participantEventReceived, this, &SessionInviteWidget::ws_participantEvent);
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::deviceEventReceived, this, &SessionInviteWidget::ws_deviceEvent);
}

void SessionInviteWidget::addParticipantsToSession(const QList<TeraData> &participants, const QList<int> &required_ids)
{
    // Participants
    foreach(TeraData participant, participants){
        // Add in session - widget will be created and updated in updateItem
        int id_item = participant.getId();
        if (!m_participantsInSession.contains(id_item))
            m_participantsInSession[id_item] = nullptr; // Set to null = create a new ListWidgetItem*

        // Update item display
        updateItem(participant);
    }

    // Required ids
    foreach(int required_id, required_ids){
        if (!m_requiredParticipants.contains(required_id))
            m_requiredParticipants.append(required_id);
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

void SessionInviteWidget::setAvailableParticipants(const QList<TeraData> &participants)
{
    foreach(TeraData participant, participants){
        int id_participant = participant.getId();
        m_participants[id_participant] = participant;
        if (!m_participantsItems.contains(id_participant))
            m_participantsItems[id_participant] = nullptr;
        updateItem(participant);
    }
    ui->btnParticipants->setVisible(!m_participants.isEmpty());
}

void SessionInviteWidget::setAvailableUsers(const QList<TeraData> &users)
{
    foreach(TeraData user, users){
        int id_user = user.getId();
        m_users[id_user] = user;
        if (!m_usersItems.contains(id_user))
            m_usersItems[id_user] = nullptr;
        updateItem(user);
    }
    ui->btnUsers->setVisible(!m_users.isEmpty());
}

void SessionInviteWidget::setAvailableDevices(const QList<TeraData> &devices)
{
    foreach(TeraData device, devices){
        int id_device = device.getId();
        m_devices[id_device] = device;
        if (!m_devicesItems.contains(id_device))
            m_devicesItems[id_device] = nullptr;
        updateItem(device);
    }
    ui->btnDevices->setVisible(!m_devices.isEmpty());
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
        updateItem(m_participants[required_id]);
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

QList<TeraData> SessionInviteWidget::getParticipantsInSession()
{
    QList<TeraData> rval;
    foreach(int id, m_participantsInSession.keys()){
        rval.append(m_participants[id]);
    }
    return rval;
}

QList<TeraData> SessionInviteWidget::getUsersInSession()
{
    QList<TeraData> rval;
    foreach(int id, m_usersInSession.keys()){
        rval.append(m_users[id]);
    }
    return rval;
}

QList<TeraData> SessionInviteWidget::getDevicesInSession()
{
    QList<TeraData> rval;
    foreach(int id, m_devicesInSession.keys()){
        rval.append(m_devices[id]);
    }
    return rval;
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
        if (m_usersItems.values().contains(item)){
            if (!ui->btnUsers->isChecked())
                hidden = true;
            else{
                item_data = &m_users[m_usersItems.key(item)];
            }
        }
        if (m_devicesItems.values().contains(item)){
            if (!ui->btnDevices->isChecked())
                hidden = true;
            else{
                item_data = &m_devices[m_devicesItems.key(item)];
            }
        }
        if (m_participantsItems.values().contains(item)){
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

void SessionInviteWidget::ws_userEvent(UserEvent event)
{
    QString uuid = QString::fromStdString(event.user_uuid());

    TeraData* data = getUserFromUuid(uuid);

    if (!data)
        return; // Item not found, we don't have to manage it here!

    if (event.type() == UserEvent_EventType_USER_CONNECTED || event.type() == UserEvent_EventType_USER_LEFT_SESSION){
        data->setState("online");
    }
    if (event.type() == UserEvent_EventType_USER_DISCONNECTED){
        data->setState("offline");
    }
    if (event.type() == UserEvent_EventType_USER_JOINED_SESSION){
        data->setState("busy");
    }

    updateItem(*data);

}

void SessionInviteWidget::ws_participantEvent(ParticipantEvent event)
{
    QString uuid = QString::fromStdString(event.participant_uuid());

    TeraData* data = getParticipantFromUuid(uuid);

    if (!data)
        return; // Item not found, we don't have to manage it here!

    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_CONNECTED || event.type() == ParticipantEvent_EventType_PARTICIPANT_LEFT_SESSION){
        data->setState("online");
    }
    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_DISCONNECTED){
        data->setState("offline");
    }
    if (event.type() == ParticipantEvent_EventType_PARTICIPANT_JOINED_SESSION){
        data->setState("busy");
    }

    updateItem(*data);
}

void SessionInviteWidget::ws_deviceEvent(DeviceEvent event)
{
    QString uuid = QString::fromStdString(event.device_uuid());

    TeraData* data = getDeviceFromUuid(uuid);

    if (!data)
        return; // Item not found, we don't have to manage it here!

    if (event.type() == DeviceEvent_EventType_DEVICE_CONNECTED || event.type() == DeviceEvent_EventType_DEVICE_LEFT_SESSION){
        data->setState("online");
    }
    if (event.type() == DeviceEvent_EventType_DEVICE_DISCONNECTED){
        data->setState("offline");
    }
    if (event.type() == DeviceEvent_EventType_DEVICE_JOINED_SESSION){
        data->setState("busy");
    }

    updateItem(*data);
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
    QHash<int, TeraData>*           item_data;
    QHash<int, QListWidgetItem*>*   item_availables;
    QHash<int, QTreeWidgetItem*>*   item_invitees;
    QTreeWidgetItem*                base_tree_item;
    QList<int>*                     item_required;

    if (item.getDataType() == TERADATA_PARTICIPANT){
        item_data = &m_participants;
        item_availables = &m_participantsItems;
        item_invitees = &m_participantsInSession;
        item_required = &m_requiredParticipants;
        base_tree_item = ui->treeInvitees->topLevelItem(0);
    }else if(item.getDataType() == TERADATA_USER){
        item_data = &m_users;
        item_availables = &m_usersItems;
        item_invitees = &m_usersInSession;
        item_required = &m_requiredUsers;
        base_tree_item = ui->treeInvitees->topLevelItem(1);
    }else if(item.getDataType() == TERADATA_DEVICE){
        item_data = &m_devices;
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
    bool invited = item_invitees->contains(id_item);

    //qDebug() << item.getName() << " - " << item.getDataTypeName(item.getDataType()) << " ID " << id_item << ": " << invited;

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
        tree_item->setText(0, item.getName());
        tree_item->setIcon(0, QIcon(item.getIconStateFilename()));

        tree_item->setDisabled(item_required->contains(id_item)); // Required, so can't be removed!

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


}

quint8 SessionInviteWidget::getInviteesCount()
{
    return m_usersInSession.count() + m_participantsInSession.count() + m_devicesInSession.count();
}

TeraData *SessionInviteWidget::getUserFromUuid(const QString &uuid)
{
    TeraData* data = nullptr;

    for (int i=0; i<m_users.values().count(); i++){
        if (m_users.values().at(i).getUuid() == uuid){
            data = &m_users[m_users.keys().at(i)];
            break;
        }
    }

    return data;
}

TeraData *SessionInviteWidget::getParticipantFromUuid(const QString &uuid)
{
    TeraData* data = nullptr;

    for (int i=0; i<m_participants.values().count(); i++){
        if (m_participants.values().at(i).getUuid() == uuid){
            data = &m_participants[m_participants.keys().at(i)];
            break;
        }
    }

    return data;
}

TeraData *SessionInviteWidget::getDeviceFromUuid(const QString &uuid)
{
    TeraData* data = nullptr;

    for (int i=0; i<m_devices.values().count(); i++){
        if (m_devices.values().at(i).getUuid() == uuid){
            data = &m_devices[m_devices.keys().at(i)];
            break;
        }
    }

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
    if (getInviteesCount() + ui->lstInvitables->selectedItems().count() > 5){
        // 5: 1 clinician and 4 others maximum
        GlobalMessageBox msgbox;
        msgbox.showError(tr("Nombre d'invités atteint"), tr("Impossible d'ajouter ces invités à la séance: le nombre maximal de participants (5) serait dépassé"));
        return;
    }

    // Add all items to the session
    foreach(QListWidgetItem* item, ui->lstInvitables->selectedItems()){
        TeraData* item_data = nullptr;
        if (m_usersItems.values().contains(item)){
            item_data = &(m_users[m_usersItems.key(item)]);
            m_usersInSession[item_data->getId()] = nullptr;
        }
        if (m_devicesItems.values().contains(item)){
            item_data = &(m_devices[m_devicesItems.key(item)]);
            m_devicesInSession[item_data->getId()] = nullptr;
        }
        if (m_participantsItems.values().contains(item)){
            item_data = &(m_participants[m_participantsItems.key(item)]);
            m_participantsInSession[item_data->getId()] = nullptr;
        }
        updateItem(*item_data);
    }
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
    on_btnRemove_clicked();
}

void SessionInviteWidget::on_btnRemove_clicked()
{
    // Remove all selected item from the session
    foreach(QTreeWidgetItem* item, ui->treeInvitees->selectedItems()){
        TeraData* item_data = nullptr;
        if (m_usersInSession.values().contains(item)){
            item_data = &(m_users[m_usersInSession.key(item)]);
            m_usersInSession.remove(item_data->getId());
        }
        if (m_devicesInSession.values().contains(item)){
            item_data = &(m_devices[m_devicesInSession.key(item)]);
            m_devicesInSession.remove(item_data->getId());
        }
        if (m_participantsInSession.values().contains(item)){
            item_data = &(m_participants[m_participantsInSession.key(item)]);
            m_participantsInSession.remove(item_data->getId());
        }

        // Remove from view
        item->parent()->removeChild(item);
        delete item;

        // Update item to add it back to invitable list
        updateItem(*item_data);
    }
}

void SessionInviteWidget::on_txtSearchInvitees_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    // Check if search field is empty
    setSearching(ui->txtSearchInvitees->text().count()>0);
    updateFilters();
}
