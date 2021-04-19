#ifndef INSESSIONWIDGET_H
#define INSESSIONWIDGET_H

#include <QWidget>
#include "managers/ComManager.h"
#include "TeraData.h"
#include "GlobalMessageBox.h"
#include "dialogs/StartSessionDialog.h"
#include "Logger.h"
#include "widgets/NotificationWindow.h"

#include "services/BaseServiceWidget.h"

#include "services/VideoRehabService/VideoRehabWidget.h"
#include "services/VideoRehabService/VideoRehabToolsWidget.h"

#include "JoinSessionReplyEvent.pb.h"

namespace Ui {
class InSessionWidget;
}

class InSessionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InSessionWidget(ComManager *comMan, const TeraData* session_type, const int id_session, const int id_project, JoinSessionEvent *initial_event = nullptr, QWidget *parent = nullptr);
    ~InSessionWidget();

    void disconnectSignals();
    void setSessionId(int session_id);
    void setPendingEvent(JoinSessionEvent* event);


private slots:
    void on_btnEndSession_clicked();
    void on_btnInSessionInfos_toggled(bool checked);

    void processSessionsReply(QList<TeraData> sessions);
    void processDevicesReply(QList<TeraData> devices);
    void processUsersReply(QList<TeraData> users);
    void processParticipantsReply(QList<TeraData> participants);

    void ws_JoinSessionEvent(JoinSessionEvent event);
    void ws_JoinSessionReplyEvent(JoinSessionReplyEvent event);
    void ws_StopSessionEvent(StopSessionEvent event);

    void showEvent(QShowEvent *event) override;

    void showStartSessionDiag(const QString& msg);
    void startSessionDiagTimeout();
    void startSessionDiagAccepted();
    void startSessionDiagRejected();

    void newSessionInviteesRequested(QStringList user_uuids, QStringList participant_uuids, QStringList device_uuids);
    void removeSessionInviteesRequested(QStringList user_uuids, QStringList participant_uuids, QStringList device_uuids);

    void sessionTimer();

    void on_btnLeaveSession_clicked();

private:
    void connectSignals();
    void initUI();
    void updateUI();
    void setMainWidget(QWidget* wdg);
    void setToolsWidget(QWidget* wdg);
    TeraSessionCategory::SessionTypeCategories getSessionTypeCategory();

    void queryLists();

    void showNotification(const NotificationWindow::NotificationType notification_type, const QString& text, const QString& iconPath);

    Ui::InSessionWidget *ui;

    QTimer              m_sessionTimer;
    QTime               m_sessionDuration;

    ComManager*             m_comManager;
    TeraData                m_sessionType;
    TeraData*               m_session;
    BaseServiceWidget*      m_serviceWidget;
    BaseServiceToolsWidget* m_serviceToolsWidget;
    StartSessionDialog*     m_startDiag;

    JoinSessionEvent*       m_pendingEvent;

    int                     m_projectId;

signals:
    void sessionEndedWithError();
    void requestShowNotification(const NotificationWindow::NotificationType notification_type, const QString& text, const QString& iconPath, const QString &soundPath, const int &width, const int &height, const int &duration);
};

#endif // INSESSIONWIDGET_H
