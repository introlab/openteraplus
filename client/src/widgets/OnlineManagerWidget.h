#ifndef ONLINEMANAGERWIDGET_H
#define ONLINEMANAGERWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>

#include "managers/ComManager.h"

namespace Ui {
class OnlineManagerWidget;
}

class OnlineManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OnlineManagerWidget(QWidget *parent = nullptr);
    ~OnlineManagerWidget();

    void setComManager(ComManager* comMan);
    void setCurrentSite(const QString& site_name, const int& site_id);
    void refreshOnlines();

private:
    Ui::OnlineManagerWidget     *ui;
    ComManager                  *m_comManager;
    QString                     m_siteName;
    int                         m_siteId;

    // Base tree items
    QTreeWidgetItem*            m_baseUsers;
    QTreeWidgetItem*            m_baseParticipants;
    QTreeWidgetItem*            m_baseDevices;

    // Data management
    QHash<QString, QTreeWidgetItem*> m_onlineUsers;        // User UUID mapping of online users
    QHash<QString, QTreeWidgetItem*> m_onlineParticipants; // Participant UUID mapping of online participants
    QHash<QString, QTreeWidgetItem*> m_onlineDevices;      // Participant UUID mapping of online participants

    QHash<QString, TeraData>        m_onlineUsersData;     // UUID mapping to data of online users
    QHash<QString, TeraData>        m_onlineParticipantsData;     // UUID mapping to data of online users
    QHash<QString, TeraData>        m_onlineDevicesData;     // UUID mapping to data of online users

    void initUi();
    void connectSignals();

    void updateCounts();

    void updateOnlineUser(const TeraData *online_user);
    void updateOnlineParticipant(const TeraData *online_participant);
    void updateOnlineDevice(const TeraData *online_device);

    void createOnlineUser(const QString& uuid, const QString& name);
    void createOnlineParticipant(const QString& uuid, const QString& name);
    void createOnlineDevice(const QString& uuid, const QString& name);

private slots:
    void ws_userEvent(UserEvent event);
    void ws_participantEvent(ParticipantEvent event);
    void ws_deviceEvent(DeviceEvent event);

    void processOnlineUsers(QList<TeraData> users);
    void processOnlineParticipants(QList<TeraData> participants);
    void processOnlineDevices(QList<TeraData> devices);

    void on_treeOnline_itemClicked(QTreeWidgetItem *item, int column);

    void on_btnFilterParticipants_clicked();

    void on_btnFilterUsers_clicked();

    void on_btnFilterDevices_clicked();

signals:
    void dataDisplayRequest(TeraDataTypes data_type, QString data_uuid);
    void totalCountUpdated(int count);
};

#endif // ONLINEMANAGERWIDGET_H
