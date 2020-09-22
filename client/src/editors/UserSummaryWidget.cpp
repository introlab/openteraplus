#include "UserSummaryWidget.h"
#include "ui_UserSummaryWidget.h"

#include <QLocale>

UserSummaryWidget::UserSummaryWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::UserSummaryWidget)
{

    m_diag_editor = nullptr;
    m_sessionLobby = nullptr;

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image
    setLimited(true);

    // Initialize user interface
    initUI();

    // Connect signals and slots
    connectSignals();

    // Query sessions types
    queryDataRequest(WEB_SESSIONTYPE_PATH);
}

UserSummaryWidget::~UserSummaryWidget()
{
    delete ui;

    if (m_sessionLobby)
        m_sessionLobby->deleteLater();
}

void UserSummaryWidget::connectSignals()
{
    connect(m_comManager, &ComManager::sessionTypesReceived, this, &UserSummaryWidget::processSessionTypesReply);

    connect(m_comManager->getWebSocketManager(), &WebSocketManager::userEventReceived, this, &UserSummaryWidget::ws_userEvent);

}

void UserSummaryWidget::updateControlsState()
{
    ui->btnEditUser->setVisible(!m_limited);
}

void UserSummaryWidget::updateFieldsValue()
{
    if (m_data){
        ui->lblTitle->setText(m_data->getName());
        QString email = m_data->getFieldValue("user_email").toString();
        if (email.isEmpty())
            email = tr("N/D");
        ui->lblEmailValue->setText(email);
        ui->lblFullNameValue->setText(m_data->getName());
        ui->lblLastOnlineValue->setText(m_data->getFieldValue("user_lastonline").toDateTime().toString("dd-MM-yyyy hh:mm:ss"));
        ui->chkEnabled->setChecked(m_data->isEnabled());

        // Status
        ui->icoOnline->setVisible(m_data->isEnabled());
        if (!m_data->hasStateField()){
            if (m_data->isEnabled()){
                m_data->setState("offline");
            }else{
                m_data->setState("");
            }
        }
        if (m_data->hasStateField()){
            ui->icoTitle->setPixmap(QPixmap(m_data->getIconStateFilename()));
            if (m_data->isOnline() || m_data->isBusy()){
                ui->icoOnline->setPixmap(QPixmap("://status/status_online.png"));
            }else{
                ui->icoOnline->setPixmap(QPixmap("://status/status_offline.png"));
            }
        }

        ui->frameNewSession->setVisible(m_data->isEnabled() && !m_data->isNew() &&
                                        m_data->getFieldValue("user_uuid") != m_comManager->getCurrentUser().getFieldValue("user_uuid"));
    }
}

void UserSummaryWidget::initUI()
{
    ui->btnEditUser->hide(); // For now
}

bool UserSummaryWidget::validateData()
{
    bool valid = false;

    return valid;
}

void UserSummaryWidget::saveData(bool signal)
{
    Q_UNUSED(signal)
    return; // Nothing to save here!
}

void UserSummaryWidget::processSessionTypesReply(QList<TeraData> session_types)
{
    ui->cmbSessionType->clear();

    for (TeraData st:session_types){
        if (!m_ids_session_types.contains(st.getId())){
            m_ids_session_types[st.getId()] = new TeraData(st);
            // New session ComboBox
            ui->cmbSessionType->addItem(st.getName(), st.getId());
        }else{
            *m_ids_session_types[st.getId()] = st;
        }
    }

}

void UserSummaryWidget::ws_userEvent(UserEvent event)
{
    if (!m_data)
        return;

    if (QString::fromStdString(event.user_uuid()) != m_data->getFieldValue("user_uuid").toString())
        return; // Not for us!

    if (event.type() == UserEvent_EventType_USER_CONNECTED || event.type() == UserEvent_EventType_USER_LEFT_SESSION){
        m_data->setState("online");
    }
    if (event.type() == UserEvent_EventType_USER_DISCONNECTED){
        m_data->setState("offline");
    }
    if (event.type() == UserEvent_EventType_USER_JOINED_SESSION){
        m_data->setState("busy");
    }
    updateFieldsValue();
}

void UserSummaryWidget::sessionLobbyStartSessionRequested()
{
    int id_session_type = ui->cmbSessionType->currentData().toInt();
    // Start session
    // TODO: Get real participant list from SessionLobbyDialog!
    m_comManager->startSession(*m_ids_session_types[id_session_type], QStringList(m_data->getFieldValue("participant_uuid").toString()), QStringList());

    m_sessionLobby->deleteLater();
    m_sessionLobby = nullptr;
}

void UserSummaryWidget::sessionLobbyStartSessionCancelled()
{
    if (m_sessionLobby){
        m_sessionLobby->deleteLater();
        m_sessionLobby = nullptr;
    }
}

void UserSummaryWidget::on_btnNewSession_clicked()
{
    if (ui->cmbSessionType->currentIndex() < 0)
        return;

    if (!m_data)
        return;

    int id_session_type = ui->cmbSessionType->currentData().toInt();

    if (m_sessionLobby)
        m_sessionLobby->deleteLater();

    m_sessionLobby = new SessionLobbyDialog(m_comManager, *m_ids_session_types[id_session_type], -1, this);

    // Add current users to session
    m_sessionLobby->addUsersToSession(QList<TeraData>() << m_comManager->getCurrentUser() << *m_data, QList<int>() << m_comManager->getCurrentUser().getId() << m_data->getId());

    connect(m_sessionLobby, &QDialog::accepted, this, &UserSummaryWidget::sessionLobbyStartSessionRequested);
    connect(m_sessionLobby, &QDialog::rejected, this, &UserSummaryWidget::sessionLobbyStartSessionCancelled);

    // Show Session Lobby
    m_sessionLobby->exec();
}
