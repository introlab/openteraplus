#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPropertyAnimation>
#include <QMovie>
#include <QDialog>
#include <QTextDocumentFragment>

#include "editors/UserWidget.h"
#include "dialogs/BaseDialog.h"
#include "widgets/ConfigWidget.h"
#include "widgets/DashboardWidget.h"

#include "dialogs/TransferProgressDialog.h"
#include "widgets/NotificationWindow.h"

#include "managers/ComManager.h"
#include "data/Message.h"
#include "data/GlobalEventLogger.h"
#include "data/GlobalEvent.h"
#include "data/DownloadingFile.h"
#include "TeraSessionCategory.h"
#include "widgets/InSessionWidget.h"
#include "dialogs/JoinSessionDialog.h"

#include "dialogs/AboutDialog.h"

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

    bool isInSession();

signals:
    void logout_request();

public slots:
    void ws_userEvent(opentera::protobuf::UserEvent event);
    void ws_participantEvent(opentera::protobuf::ParticipantEvent event);
    void ws_joinSessionEvent(opentera::protobuf::JoinSessionEvent event);

private slots:
    void updateCurrentUser();
    void updateOnlineCounts(int count);

    void processGenericDataReply(TeraDataTypes item_data_type, QList<TeraData> datas);

    void com_socketError(QAbstractSocket::SocketError error, QString error_msg);
    void com_networkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code);
    void com_waitingForReply(bool waiting);
    void com_postReplyOK(QString path);
    void com_deleteResultsOK(QString path, int id);
    void com_posting(QString path, QString data);
    void com_querying(QString path);
    void com_deleting(QString path);
    void com_downloadProgress(DownloadingFile* file);
    void com_downloadCompleted(DownloadingFile* file);

    void com_preferencesUpdated();

    void com_sessionStarted(TeraData session_type, int id_session);
    void com_sessionStartRequested(TeraData session_type);
    void com_sessionStopped(int id_session);
    void com_sessionError(QString error);

    void inSession_sessionEndedWithError();

    void nextMessageWasShown(Message current_message);
    void notificationCompleted(NotificationWindow* notify);
    void addNotification(const NotificationWindow::NotificationType notification_type, const QString& text, const QString& iconPath = QString(), const QString &soundPath = QString(), const int &width=400, const int &height=100, const int &duration=5000);

    void currentSiteChanged(QString site_name, int site_id);
    void dataRefreshRequested();

    void addGlobalEvent(GlobalEvent event);

    void editorDialogFinished();
    void joinSessionDialogFinished();
    void dataDisplayRequested(TeraDataTypes data_type, int data_id);
    void dataDisplayRequestedByUuid(TeraDataTypes data_type, QString data_uuid);
    void dataDeleteRequested(TeraDataTypes data_type, int data_id);   
    void dataEditorCancelled();

    void on_btnLogout_clicked();
    void on_btnEditUser_clicked();
    void on_btnConfig_clicked();

    void on_btnLog_toggled(bool checked);

    void on_tableHistory_itemDoubleClicked(QTableWidgetItem *item);

    void on_lblLogo_clicked();

private:
    void connectSignals();
    void initUi();
    void showDataEditor(const TeraDataTypes &data_type, const TeraData *data);
    void showDashboard(const bool &show);
    void setInSession(bool in_session, const TeraData *session_type, const int& id_session, int id_project=0);

    // Messages and notifications
    void addMessage(Message::MessageType msg_type, QString msg);
    void addMessage(Message &msg);


    // Events
    QIcon getGlobalEventIcon(GlobalEvent &global_event);
    void changeEvent(QEvent* event);
    QString m_currentLanguage;

    Ui::MainWindow *ui;

    ComManager*             m_comManager;
    BaseDialog*             m_diag_editor;
    DataEditorWidget*       m_data_editor;
    DashboardWidget*        m_dashboard;
    InSessionWidget*        m_inSessionWidget;
    TransferProgressDialog* m_download_dialog;
    JoinSessionDialog*      m_joinSession_dialog;
    TeraDataTypes           m_waiting_for_data_type;
    TeraDataTypes           m_currentDataType;
    int                     m_currentDataId;

    // Message & notification system
    QList<NotificationWindow*>  m_notifications;

    // UI items
    QMovie*         m_loadingIcon;


};

#endif // MAINWINDOW_H
