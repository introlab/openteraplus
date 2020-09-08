#ifndef SESSIONINVITEWIDGET_H
#define SESSIONINVITEWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include "TeraData.h"

namespace Ui {
class SessionInviteWidget;
}

class SessionInviteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SessionInviteWidget(QWidget *parent = nullptr);
    ~SessionInviteWidget();

    void addParticipantsToSession(const QList<TeraData>& participants, const QList<int>& required_ids = QList<int>());
    void addUsersToSession(const QList<TeraData>& users, const QList<int>& required_ids = QList<int>());
    void addDevicesToSession(const QList<TeraData>& devices, const QList<int>& required_ids = QList<int>());

    void setAvailableParticipants(const QList<TeraData> &participants);
    void setAvailableUsers(const QList<TeraData> &users);
    void setAvailableDevices(const QList<TeraData> &devices);

private slots:
    void on_btnManageInvitees_clicked();

private:
    Ui::SessionInviteWidget *ui;

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

    void updateItem(const TeraData &item, const bool &invited);

};

#endif // SESSIONINVITEWIDGET_H
