#include "JoinSessionDialog.h"
#include "ui_JoinSessionDialog.h"

JoinSessionDialog::JoinSessionDialog(ComManager *comMan, opentera::protobuf::JoinSessionEvent event, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JoinSessionDialog),
    m_comManager(comMan),
    m_event(event)
{
    ui->setupUi(this);

    m_reply = JoinSessionReplyEvent_ReplyType::JoinSessionReplyEvent_ReplyType_REPLY_UNKNOWN;

    connectSignals();

    ui->frameButtons->setDisabled(true); // Disable buttons until we get all required data

    // Query session information for session uuid in event
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_SESSION_UUID, QString::fromStdString(event.session_uuid()));
    //args.addQueryItem(WEB_QUERY_LIST, "1");
    m_comManager->doQuery(WEB_SESSIONINFO_PATH, args);

    initUi();
}

JoinSessionReplyEvent_ReplyType JoinSessionDialog::getSelectedReply()
{
    return m_reply;
}

JoinSessionEvent* JoinSessionDialog::getEvent()
{
    return &m_event;
}

TeraData *JoinSessionDialog::getSessionType()
{
    return &m_sessionType;
}

TeraData *JoinSessionDialog::getSession()
{
    return &m_session;
}

int JoinSessionDialog::getSessionId()
{
    if (m_session.getDataType() != TERADATA_NONE)
        return m_session.getId();

    return -1;
}

JoinSessionDialog::~JoinSessionDialog()
{
    delete ui;
}

void JoinSessionDialog::initUi()
{
    ui->lblInviteText->setText("<b><font color=cyan>" + QString::fromStdString(m_event.session_creator_name()) + "</font></b> " + tr("vous invite à rejoindre une séance."));
    ui->lblInviteMsg->setVisible(!m_event.join_msg().empty());
    if (!m_event.join_msg().empty()){
        ui->lblInviteMsg->setText(tr("L'invitation comporte le message suivant:<br><i>") + QString::fromStdString(m_event.join_msg()) + "</i>");
    }

    QSound::play("://sounds/notify_invite.wav");
}

void JoinSessionDialog::connectSignals()
{
    connect(m_comManager, &ComManager::sessionsReceived, this, &JoinSessionDialog::processSessionsReply);
    connect(m_comManager, &ComManager::sessionTypesReceived, this, &JoinSessionDialog::processSessionsTypesReply);
}

void JoinSessionDialog::on_btnJoinSession_clicked()
{
    m_reply = JoinSessionReplyEvent_ReplyType::JoinSessionReplyEvent_ReplyType_REPLY_ACCEPTED;
    accept();
}

void JoinSessionDialog::on_btnDenySession_clicked()
{
    m_reply = JoinSessionReplyEvent_ReplyType::JoinSessionReplyEvent_ReplyType_REPLY_DENIED;
    accept();
}

void JoinSessionDialog::on_btnBusySession_clicked()
{
     m_reply = JoinSessionReplyEvent_ReplyType::JoinSessionReplyEvent_ReplyType_REPLY_BUSY;
     accept();
}

void JoinSessionDialog::processSessionsReply(QList<TeraData> sessions)
{
    if (m_session.getDataType() != TERADATA_NONE)
        // Already got a session
        return;

    foreach(TeraData session, sessions){
        if (session.getUuid() == QString::fromStdString(m_event.session_uuid())){
            m_session = session;
            // Query session type
            QUrlQuery args;
            args.addQueryItem(WEB_QUERY_ID_SESSION_TYPE, m_session.getFieldValue("id_session_type").toString());
            m_comManager->doQuery(WEB_SESSIONTYPE_PATH, args);
            return;
        }
    }

}

void JoinSessionDialog::processSessionsTypesReply(QList<TeraData> session_types)
{
    if (m_sessionType.getDataType() != TERADATA_NONE)
        // Already got a session type
        return;

    if (m_session.getDataType() == TERADATA_NONE)
        // No session
        return;

    foreach(TeraData session_type, session_types){
        if (session_type.getId() == m_session.getFieldValue("id_session_type").toInt()){
            ui->frameButtons->setEnabled(true);
            m_sessionType = session_type;
            return;
        }
    }



}
