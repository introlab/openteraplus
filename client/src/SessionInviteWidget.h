#ifndef SESSIONINVITEWIDGET_H
#define SESSIONINVITEWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTreeWidgetItem>

#include "TeraData.h"
#include "GlobalMessageBox.h"
#include "ComManager.h"

namespace Ui {
class SessionInviteWidget;
}

class SessionInviteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SessionInviteWidget(QWidget *parent = nullptr);
    ~SessionInviteWidget();

    void setComManager(ComManager* comMan);

    void addParticipantsToSession(const QList<TeraData>& participants, const QList<int>& required_ids = QList<int>());
    void addUsersToSession(const QList<TeraData>& users, const QList<int>& required_ids = QList<int>());
    void addDevicesToSession(const QList<TeraData>& devices, const QList<int>& required_ids = QList<int>());

    void selectDefaultFilter();

    void setAvailableParticipants(const QList<TeraData> &participants);
    void setAvailableUsers(const QList<TeraData> &users);
    void setAvailableDevices(const QList<TeraData> &devices);

    bool hasAvailableParticipants() const;
    bool hasAvailableDevices() const;
    bool hasAvailableUsers() const;

    void addRequiredUser(const int& required_id);
    void addRequiredParticipant(const int& required_id);
    void addRequiredDevice(const int& required_id);

    void setConfirmOnRemove(const bool &confirm);

    QList<TeraData> getParticipantsInSession();
    QList<TeraData> getUsersInSession();
    QList<TeraData> getDevicesInSession();

private slots:
    void on_btnManageInvitees_clicked();
    void updateFilters();

    void ws_userEvent(UserEvent event);
    void ws_participantEvent(ParticipantEvent event);
    void ws_deviceEvent(DeviceEvent event);

    void on_btnInvite_clicked();
    void on_lstInvitables_itemSelectionChanged();
    void on_lstInvitables_itemDoubleClicked(QListWidgetItem *item);
    void on_treeInvitees_itemSelectionChanged();
    void on_treeInvitees_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_btnRemove_clicked();
    void on_txtSearchInvitees_textChanged(const QString &arg1);

private:
    Ui::SessionInviteWidget *ui;

    ComManager* m_comManager;

    bool m_searching;
    bool m_confirmRemove;

    QHash<int, TeraData>    m_users; // id, data
    QHash<int, TeraData>    m_participants;
    QHash<int, TeraData>    m_devices;

    QHash<int, QListWidgetItem*>    m_usersItems;
    QHash<int, QListWidgetItem*>    m_participantsItems;
    QHash<int, QListWidgetItem*>    m_devicesItems;

    QList<int> m_requiredUsers;
    QList<int> m_requiredParticipants;
    QList<int> m_requiredDevices;

    QHash<int, QTreeWidgetItem*>    m_usersInSession;
    QHash<int, QTreeWidgetItem*>    m_participantsInSession;
    QHash<int, QTreeWidgetItem*>    m_devicesInSession;

    void connectSignals();

    void updateItem(const TeraData &item);
    void refreshInSessionTreeWidget();

    quint8 getInviteesCount();

    TeraData* getUserFromUuid(const QString& uuid);
    TeraData* getParticipantFromUuid(const QString& uuid);
    TeraData* getDeviceFromUuid(const QString& uuid);

    void setSearching(const bool &search);

 signals:
    void newInvitees(QStringList user_uuids, QStringList participant_uuids, QStringList device_uuids);
    void removedInvitees(QStringList user_uuids, QStringList participant_uuids, QStringList device_uuids);

};

#endif // SESSIONINVITEWIDGET_H
