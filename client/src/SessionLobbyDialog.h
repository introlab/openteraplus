#ifndef SESSIONLOBBYDIALOG_H
#define SESSIONLOBBYDIALOG_H

#include <QDialog>
#include "ComManager.h"

#include "services/VideoRehabService/VideoRehabSetupWidget.h"

namespace Ui {
class SessionLobbyDialog;
}

class SessionLobbyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SessionLobbyDialog(ComManager* comManager, TeraData &session_type, int id_project, QWidget *parent = nullptr);
    ~SessionLobbyDialog();

    void addParticipantsToSession(const QList<TeraData>& participants, const QList<int>& required_ids = QList<int>());
    void addUsersToSession(const QList<TeraData>& users, const QList<int>& required_ids = QList<int>());
    void addDevicesToSession(const QList<TeraData>& devices, const QList<int>& required_ids = QList<int>());

private slots:
    void on_btnCancel_clicked();
    void on_btnStartSession_clicked();

    void processDevicesReply(QList<TeraData> devices);
    void processUsersReply(QList<TeraData> users);
    void processParticipantsReply(QList<TeraData> participants);

private:
    Ui::SessionLobbyDialog  *ui;
    ComManager*             m_comManager;
    TeraData                m_sessionType;
    int                     m_idProject;

    bool                    m_gotUsers;
    bool                    m_gotParticipants;
    bool                    m_gotDevices;

    void configureWidget();
    void setSetupWidget(QWidget *wdg);
    void connectSignals();

    void queryLists();
    void checkReady();

    TeraSessionCategory::SessionTypeCategories getSessionTypeCategory();

};

#endif // SESSIONLOBBYDIALOG_H
