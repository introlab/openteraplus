#include "SessionLobbyDialog.h"
#include "ui_SessionLobbyDialog.h"

SessionLobbyDialog::SessionLobbyDialog(ComManager* comManager, TeraData &session_type, int id_project, int id_session, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SessionLobbyDialog),
    m_comManager(comManager),
    m_sessionType(session_type),
    m_idProject(id_project),
    m_idSession(id_session)
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

QStringList SessionLobbyDialog::getSessionParticipantsUuids()
{
    QStringList uuids;
    QList<TeraData> participants = ui->wdgSessionInvite->getParticipantsInSession();

    foreach(TeraData part, participants){
        uuids.append(part.getUuid());
    }
    return uuids;
}

QStringList SessionLobbyDialog::getSessionUsersUuids()
{
    QStringList uuids;
    QList<TeraData> users = ui->wdgSessionInvite->getUsersInSession();

    foreach(TeraData user, users){
        uuids.append(user.getUuid());
    }
    return uuids;
}

QStringList SessionLobbyDialog::getSessionDevicesUuids()
{
    QStringList uuids;
    QList<TeraData> devices = ui->wdgSessionInvite->getDevicesInSession();

    foreach(TeraData device, devices){
        uuids.append(device.getUuid());
    }
    return uuids;
}

int SessionLobbyDialog::getIdSession() const
{
    return m_idSession;
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
    connect(m_comManager, &ComManager::sessionsReceived, this, &SessionLobbyDialog::processSessionsReply);

}

void SessionLobbyDialog::queryLists()
{
    QUrlQuery args;
    if (m_idProject>0)
        args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_idProject));
    args.addQueryItem(WEB_QUERY_LIST, "1");
    args.addQueryItem(WEB_QUERY_ENABLED, "1");
    args.addQueryItem(WEB_QUERY_WITH_STATUS, "1");
    m_comManager->doQuery(WEB_USERINFO_PATH, args);
    m_gotUsers = false;
    m_comManager->doQuery(WEB_PARTICIPANTINFO_PATH, args);
    m_gotParticipants = false;
    m_comManager->doQuery(WEB_DEVICEINFO_PATH, args);
    m_gotDevices = false;
    if (m_idSession>0){
        m_gotSession = false;
    }else{
        m_gotSession = true;
    }

    // Disable form until we got everything we need!
    checkReady();
}

void SessionLobbyDialog::checkReady()
{
    setEnabled(m_gotDevices && m_gotParticipants && m_gotUsers && m_gotSession);

    if (m_gotDevices && m_gotParticipants && m_gotUsers && !m_gotSession){
        // Query for session information to add invitees
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(m_idSession));
        m_comManager->doQuery(WEB_SESSIONINFO_PATH, args);
    }
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

void SessionLobbyDialog::processSessionsReply(QList<TeraData> sessions)
{
    foreach(TeraData session, sessions){
        if (session.getId() == m_idSession){
            // Get participants, users and devices list, and mark them as "required" and invited
            QVariantList item_list;

            if (session.hasFieldName("session_participants")){
                item_list = session.getFieldValue("session_participants").toList();

                for(QVariant session_part:item_list){
                    QVariantMap part_info = session_part.toMap();
                    ui->wdgSessionInvite->addRequiredParticipant(part_info["id_participant"].toInt());
                }
            }

            if (session.hasFieldName("session_users")){
                item_list = session.getFieldValue("session_users").toList();

                for(QVariant session_user:item_list){
                    QVariantMap user_info = session_user.toMap();
                    ui->wdgSessionInvite->addRequiredUser(user_info["id_user"].toInt());
                }
            }

            if (session.hasFieldName("session_devices")){
                item_list = session.getFieldValue("session_devices").toList();

                for(QVariant session_device:item_list){
                    QVariantMap device_info = session_device.toMap();
                    ui->wdgSessionInvite->addRequiredDevice(device_info["id_device"].toInt());
                }
            }

        }
    }

    m_gotSession = true;
    checkReady();
}
