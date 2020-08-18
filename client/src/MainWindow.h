#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPropertyAnimation>
#include <QMovie>
#include <QDialog>

#include "editors/UserWidget.h"
#include "BaseDialog.h"
#include "ConfigWidget.h"

#include "DownloadProgressDialog.h"
#include "NotificationWindow.h"

#include "ComManager.h"
#include "Message.h"
#include "GlobalEventLogger.h"
#include "GlobalEvent.h"
#include "DownloadedFile.h"
#include "TeraSessionCategory.h"
#include "InSessionWidget.h"

// Protobuf
#include "UserEvent.pb.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT


public:

    explicit MainWindow(ComManager* com_manager, QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void logout_request();

private slots:
    void updateCurrentUser();
    void processGenericDataReply(TeraDataTypes item_data_type, QList<TeraData> datas);

    void com_socketError(QAbstractSocket::SocketError error, QString error_msg);
    void com_networkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code);
    void com_waitingForReply(bool waiting);
    void com_postReplyOK(QString path);
    void com_deleteResultsOK(QString path, int id);
    void com_posting(QString path, QString data);
    void com_querying(QString path);
    void com_deleting(QString path);

    void com_downloadProgress(DownloadedFile* file);
    void com_downloadCompleted(DownloadedFile* file);

    void com_sessionStarted(TeraData session_type, int id_session);
    void com_sessionStartRequested(TeraData session_type);
    void com_sessionStopped(int id_session);

    void ws_userEvent(UserEvent event);
    void ws_participantEvent(ParticipantEvent event);

    bool hasWaitingMessage();
    void showNextMessage();
    void notificationCompleted(NotificationWindow* notify);

    void addGlobalEvent(GlobalEvent event);

    void editorDialogFinished();
    void dataDisplayRequested(TeraDataTypes data_type, int data_id);
    void dataDeleteRequested(TeraDataTypes data_type, int data_id);   
    void dataEditorCancelled();

    void on_btnCloseMessage_clicked();
    void on_btnLogout_clicked();
    void on_btnEditUser_clicked();
    void on_btnConfig_clicked();

    void on_btnLog_toggled(bool checked);

    void on_tableHistory_itemDoubleClicked(QTableWidgetItem *item);

private:
    void connectSignals();
    void initUi();
    void showDataEditor(const TeraDataTypes &data_type, const TeraData *data);
    void setInSession(bool in_session, const TeraData *session_type, const int& id_session);

    // Online users and participants management
    void updateOnlineUser(const QString &user_uuid, const bool &online, const QString &user_name = QString());

    // Messages and notifications
    void addMessage(Message::MessageType msg_type, QString msg);
    void addMessage(Message &msg);
    void addNotification(const NotificationWindow::NotificationType notification_type, const QString& text, const QString& iconPath = QString());

    // Events
    QIcon getGlobalEventIcon(GlobalEvent &global_event);

    Ui::MainWindow *ui;

    ComManager*             m_comManager;
    BaseDialog*             m_diag_editor;
    DataEditorWidget*       m_data_editor;
    InSessionWidget*        m_inSessionWidget;
    DownloadProgressDialog* m_download_dialog;
    TeraDataTypes           m_waiting_for_data_type;
    TeraDataTypes           m_currentDataType;
    int                     m_currentDataId;

    // Message & notification system
    QList<Message>              m_messages;
    QList<NotificationWindow*>  m_notifications;
    Message                     m_currentMessage;
    QTimer                      m_msgTimer;

    // UI items
    QMovie*         m_loadingIcon;

    // Data management
    QHash<QString, QListWidgetItem*> m_onlineUsers; // User UUID mapping of online users
    QHash<QString, QListWidgetItem*> m_onlineParticipants; // Participant UUID mapping of online participants


};

#endif // MAINWINDOW_H
