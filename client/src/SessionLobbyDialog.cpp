#include "SessionLobbyDialog.h"
#include "ui_SessionLobbyDialog.h"

SessionLobbyDialog::SessionLobbyDialog(ComManager* comManager, TeraData &session_type, int id_project, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SessionLobbyDialog),
    m_comManager(comManager),
    m_sessionType(session_type),
    m_idProject(id_project)
{
    ui->setupUi(this);

    ui->lblTitle->setText(m_sessionType.getFieldValue("session_type_name").toString());

    connectSignals();
    configureWidget();

    // Query users, participants and devices for this project
    queryLists();
}

SessionLobbyDialog::~SessionLobbyDialog()
{
    delete ui;
}

void SessionLobbyDialog::addParticipantsToSession(const QList<TeraData> &participants, const QList<int>& required_ids)
{
    ui->wdgSessionInvite->addParticipantsToSession(participants, required_ids);
}

void SessionLobbyDialog::addUsersToSession(const QList<TeraData> &users, const QList<int>& required_ids)
{
    ui->wdgSessionInvite->addUsersToSession(users, required_ids);
}

void SessionLobbyDialog::addDevicesToSession(const QList<TeraData> &devices, const QList<int> &required_ids)
{
    ui->wdgSessionInvite->addDevicesToSession(devices, required_ids);
}

void SessionLobbyDialog::setSetupWidget(QWidget *wdg)
{

    // Check for layout
    if (!ui->wdgSessionConfig->layout()){
        // No existing layout - create one
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        ui->wdgSessionConfig->setLayout(layout);
    }

    ui->wdgSessionConfig->layout()->addWidget(wdg);
}

void SessionLobbyDialog::connectSignals()
{

    connect(m_comManager, &ComManager::usersReceived, this, &SessionLobbyDialog::processUsersReply);
    connect(m_comManager, &ComManager::participantsReceived, this, &SessionLobbyDialog::processParticipantsReply);
    connect(m_comManager, &ComManager::devicesReceived, this, &SessionLobbyDialog::processDevicesReply);

}

void SessionLobbyDialog::queryLists()
{
    QUrlQuery args;
    if (m_idProject>0)
        args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_idProject));
    args.addQueryItem(WEB_QUERY_LIST, "1");
    m_comManager->doQuery(WEB_USERINFO_PATH, args);
    m_gotUsers = false;
    m_comManager->doQuery(WEB_PARTICIPANTINFO_PATH, args);
    m_gotParticipants = false;
    m_comManager->doQuery(WEB_DEVICEINFO_PATH, args);
    m_gotDevices = false;

    // Disable form until we got everything we need!
    checkReady();
}

void SessionLobbyDialog::checkReady()
{
    setEnabled(m_gotDevices && m_gotParticipants && m_gotUsers);
    if (isEnabled()){
        ui->wdgSessionInvite->selectDefaultFilter();
    }
}

void SessionLobbyDialog::configureWidget()
{
    // Session Invite Widget
    ui->wdgSessionInvite->setComManager(m_comManager);

    // Get session type category
    TeraSessionCategory::SessionTypeCategories category = getSessionTypeCategory();

    switch(category){
    case TeraSessionCategory::SESSION_TYPE_SERVICE:{
        // Get service key to load the proper ui
        QString service_key = m_sessionType.getFieldValue("session_type_service_key").toString();
        bool handled = false;
        if (service_key == "VideoRehabService"){
            setSetupWidget(new VideoRehabSetupWidget(m_comManager, this));
            handled = true;
        }

        if (!handled){
            ui->wdgSessionConfig->hide();
        }

        break;
    }
    default:
       ui->wdgSessionConfig->hide();
    }
}

TeraSessionCategory::SessionTypeCategories SessionLobbyDialog::getSessionTypeCategory()
{
    return static_cast<TeraSessionCategory::SessionTypeCategories>(m_sessionType.getFieldValue("session_type_category").toInt());
}

void SessionLobbyDialog::on_btnCancel_clicked()
{
    reject();
}

void SessionLobbyDialog::on_btnStartSession_clicked()
{
    accept();
}

void SessionLobbyDialog::processDevicesReply(QList<TeraData> devices)
{
    ui->wdgSessionInvite->setAvailableDevices(devices);
    m_gotDevices = true;
    checkReady();
}

void SessionLobbyDialog::processUsersReply(QList<TeraData> users)
{
    ui->wdgSessionInvite->setAvailableUsers(users);
    m_gotUsers = true;
    checkReady();
}

void SessionLobbyDialog::processParticipantsReply(QList<TeraData> participants)
{
    ui->wdgSessionInvite->setAvailableParticipants(participants);
    m_gotParticipants = true;
    checkReady();
}
