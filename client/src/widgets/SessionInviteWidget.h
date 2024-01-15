#ifndef SESSIONINVITEWIDGET_H
#define SESSIONINVITEWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTreeWidgetItem>

#include "TeraData.h"
#include "GlobalMessageBox.h"
#include "managers/ComManager.h"

#define MAX_INVITEES_IN_SESSION 6

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
    void setCurrentSessionUuid(const QString &session_uuid);

    void addParticipantsToSession(const QList<TeraData>& participants, const QList<int>& required_ids = QList<int>());
    void addUsersToSession(const QList<TeraData>& users, const QList<int>& required_ids = QList<int>());
    void addDevicesToSession(const QList<TeraData>& devices, const QList<int>& required_ids = QList<int>());

    void addUserToSession(const QString& user_uuid);
    void addParticipantToSession(const QString& participant_uuid);
    void addDeviceToSession(const QString& device_uuid);

    void removeUserFromSession(const QString& user_uuid);
    void removeParticipantFromSession(const QString& participant_uuid);
    void removeDeviceFromSession(const QString& device_uuid);

    void selectDefaultFilter();
    void autoSelectFilters();
    void selectFilterParticipant();

    void showAvailableInvitees(const bool &show, const bool &hide_button=false);
    void showOnlineFilter(const bool &show);

    void setAvailableParticipants(const QList<TeraData> &participants);
    void setAvailableUsers(const QList<TeraData> &users);
    void setAvailableDevices(const QList<TeraData> &devices);

    bool hasAvailableParticipants() const;
    bool hasAvailableDevices() const;
    bool hasAvailableUsers() const;

    void addRequiredUser(const int& required_id);
    void addRequiredUser(const QString& required_uuid);
    void addRequiredParticipant(const int& required_id);
    void addRequiredParticipant(const QString& required_uuid);
    void addRequiredDevice(const int& required_id);

    void setConfirmOnRemove(const bool &confirm);

    void setEditable(const bool &editable);
    bool isEditable() const;

    QList<TeraData> getParticipantsInSession();
    QList<TeraData> getUsersInSession();
    QList<TeraData> getDevicesInSession();

    QStringList getParticipantsUuidsInSession();
    QStringList getUsersUuidsInSession();
    QStringList getDevicesUuidsInSession();

    QList<int> getParticipantsIdsInSession();
    QList<int> getUsersIdsInSession();
    QList<int> getDevicesIdsInSession();

    quint8 getInviteesCount();
    quint8 getParticipantsInSessionCount();
    quint8 getUsersInSessionCount();
    quint8 getDevicesInSessionCount();

    TeraData* getUserFromUuid(const QString& uuid);
    TeraData* getParticipantFromUuid(const QString& uuid);
    TeraData* getDeviceFromUuid(const QString& uuid);

private slots:
    void on_btnManageInvitees_clicked();
    void updateFilters();

    void ws_userEvent(UserEvent event);
    void ws_participantEvent(ParticipantEvent event);
    void ws_deviceEvent(DeviceEvent event);
    void ws_joinSessionEvent(JoinSessionEvent event);
    void ws_leaveSessionEvent(LeaveSessionEvent event);

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
    QString m_currentSessionUuid;

    bool m_searching;
    bool m_confirmRemove;
    bool m_editable;

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

    QStringList m_unknownUserUuidsInSession;
    QStringList m_unknownDeviceUuidsInSession;
    QStringList m_unknownParticipantUuidsInSession;

    void connectSignals();

    void updateItem(const TeraData &item);
    void refreshInSessionTreeWidget();

    void setSearching(const bool &search);

 signals:
    void newInvitees(QStringList user_uuids, QStringList participant_uuids, QStringList device_uuids);
    void removedInvitees(QStringList user_uuids, QStringList participant_uuids, QStringList device_uuids);

    void removedUser(QString user_uuid);
    void removedParticipant(QString participant_uuid);
    void removedDevice(QString device_uuid);

};

#endif // SESSIONINVITEWIDGET_H
