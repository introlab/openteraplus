#ifndef JOINSESSIONDIALOG_H
#define JOINSESSIONDIALOG_H

#include <QDialog>
#include <QSoundEffect>
#include "managers/ComManager.h"

// Protobuf
#include "JoinSessionEvent.pb.h"
#include "JoinSessionReplyEvent.pb.h"

using namespace opentera::protobuf;

namespace Ui {
class JoinSessionDialog;
}

class JoinSessionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JoinSessionDialog(ComManager* comMan, JoinSessionEvent event, QWidget *parent = nullptr);

    JoinSessionReplyEvent_ReplyType getSelectedReply();
    JoinSessionEvent *getEvent();
    TeraData *getSessionType();
    TeraData* getSession();

    int getSessionId();

    virtual ~JoinSessionDialog() override;

private slots:
    void on_btnJoinSession_clicked();
    void on_btnDenySession_clicked();
    void on_btnBusySession_clicked();

    void processSessionsReply(QList<TeraData> sessions);
    void processSessionsTypesReply(QList<TeraData> session_types);

private:
    Ui::JoinSessionDialog *ui;

    ComManager* m_comManager;

    JoinSessionEvent                m_event;
    JoinSessionReplyEvent_ReplyType m_reply;

    TeraData m_session;
    TeraData m_sessionType;

    QSoundEffect m_soundPlayer;

    void initUi();
    void connectSignals();
};

#endif // JOINSESSIONDIALOG_H
