#include "SessionInviteWidget.h"
#include "ui_SessionInviteWidget.h"

SessionInviteWidget::SessionInviteWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionInviteWidget)
{
    ui->setupUi(this);

    ui->frameSelector->hide();
}

SessionInviteWidget::~SessionInviteWidget()
{
    delete ui;
}

void SessionInviteWidget::addParticipantsToSession(const QList<TeraData> &participants, const QList<int> &required_ids)
{
    // Participants
    foreach(TeraData participant, participants){
        updateItem(participant, true);

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
        updateItem(user, true);
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
       updateItem(device, true);

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
    }
}

void SessionInviteWidget::setAvailableUsers(const QList<TeraData> &users)
{
    foreach(TeraData user, users){
        int id_user = user.getId();
        m_users[id_user] = user;
    }
}

void SessionInviteWidget::setAvailableDevices(const QList<TeraData> &devices)
{
    foreach(TeraData device, devices){
        int id_device = device.getId();
        m_devices[id_device] = device;
    }
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

void SessionInviteWidget::updateItem(const TeraData &item, const bool &invited)
{
    // Get pointers to correct structures depending on item type
    QHash<int, TeraData>*           item_data;
    QHash<int, QListWidgetItem*>*   item_availables;
    QHash<int, QTreeWidgetItem*>*   item_invitees;
    QTreeWidgetItem*                base_tree_item;

    if (item.getDataType() == TERADATA_PARTICIPANT){
        item_data = &m_participants;
        item_availables = &m_participantsItems;
        item_invitees = &m_participantsInSession;
        base_tree_item = ui->treeInvitees->topLevelItem(0);
    }else if(item.getDataType() == TERADATA_USER){
        item_data = &m_users;
        item_availables = &m_usersItems;
        item_invitees = &m_participantsInSession;
        base_tree_item = ui->treeInvitees->topLevelItem(1);
    }else if(item.getDataType() == TERADATA_DEVICE){
        item_data = &m_devices;
        item_availables = &m_devicesItems;
        item_invitees = &m_devicesInSession;
        base_tree_item = ui->treeInvitees->topLevelItem(2);
    }else{
        qWarning("SessionInviteWidget::updateItem - Wrong data type!!");
        return;
    }

    int id_item = item.getId();
    if (invited){
        // Item is in the invited list
        QTreeWidgetItem* tree_item = nullptr;
        if (item_invitees->contains(id_item)){
            tree_item = (*item_invitees)[id_item];
        }else{
            tree_item = new QTreeWidgetItem(base_tree_item);
            item_invitees->insert(id_item, tree_item);
        }

        // Update informations
        tree_item->setText(0, item.getName());
        tree_item->setIcon(0, QIcon(item.getIconStateFilename()));

        // Make sure item isn't in available list anymore
        if (item_availables->contains(id_item)){
            delete ui->lstInvitables->takeItem(ui->lstInvitables->row((*item_availables)[id_item]));
            item_availables->remove(id_item);
        }

    }else{
        // Item is in the available list
        QListWidgetItem* list_item = nullptr;
        if (item_availables->contains(id_item)){
            list_item = (*item_availables)[id_item];
        }else{
            list_item = new QListWidgetItem();
            item_availables->insert(id_item, list_item);
        }

        // Update informations
        list_item->setText(item.getName());
        list_item->setIcon(QIcon(item.getIconStateFilename()));

        // Make sure item isn't in invited list anymore
        if (item_invitees->contains(id_item)){
            base_tree_item->removeChild( (*item_invitees)[id_item]);
            delete (*item_invitees)[id_item];
            item_invitees->remove(id_item);
        }
    }

}
