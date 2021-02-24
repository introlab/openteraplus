#ifndef SESSIONLOBBYDIALOG_H
#define SESSIONLOBBYDIALOG_H

#include <QDialog>
#include "ComManager.h"

#include "services/BaseServiceSetupWidget.h"
#include "services/VideoRehabService/VideoRehabSetupWidget.h"

namespace Ui {
class SessionLobbyDialog;
}

class SessionLobbyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SessionLobbyDialog(ComManager* comManager, TeraData &session_type, int id_project, int id_session, QWidget *parent = nullptr);
    ~SessionLobbyDialog();

    void addParticipantsToSession(const QList<TeraData>& participants, const QList<int>& required_ids = QList<int>());
    void addUsersToSession(const QList<TeraData>& users, const QList<int>& required_ids = QList<int>());
    void addDevicesToSession(const QList<TeraData>& devices, const QList<int>& required_ids = QList<int>());

    QStringList getSessionParticipantsUuids();
    QStringList getSessionUsersUuids();
    QStringList getSessionDevicesUuids();
    QJsonDocument getSessionConfig();

    int getIdSession() const;
    int getIdSessionType() const;

private slots:
    void on_btnCancel_clicked();
    void on_btnStartSession_clicked();

    void processDevicesReply(QList<TeraData> devices);
    void processUsersReply(QList<TeraData> users);
    void processParticipantsReply(QList<TeraData> participants);
    void processSessionsReply(QList<TeraData> sessions);

private:
    Ui::SessionLobbyDialog  *ui;
    ComManager*             m_comManager;
    TeraData                m_sessionType;
    BaseServiceSetupWidget* m_setupWdg;
    int                     m_idProject;
    int                     m_idSession;
    QJsonDocument           m_sessionConfig;

    bool                    m_gotUsers;
    bool                    m_gotParticipants;
    bool                    m_gotDevices;
    bool                    m_gotSession;

    void configureWidget();
    void setSetupWidget(QWidget *wdg);
    void removeSetupWidget();
    void connectSignals();

    void queryLists();
    void checkReady();

    TeraSessionCategory::SessionTypeCategories getSessionTypeCategory();

};

#endif // SESSIONLOBBYDIALOG_H
