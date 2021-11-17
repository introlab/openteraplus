#include "DashboardWidget.h"
#include "ui_DashboardWidget.h"

DashboardWidget::DashboardWidget(ComManager *comMan, int id_site, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DashboardWidget),
    m_comManager(comMan)
{
    ui->setupUi(this);

    m_sessionLobby = nullptr;
    ui->tableUpcomingSessions->hide();

    connectSignals();

    setCurrentSiteId(id_site);

    // Query available sessions types for the current user
    m_comManager->doQuery(WEB_SESSIONTYPE_PATH);

}

DashboardWidget::~DashboardWidget()
{
    qDeleteAll(m_sessions);
    qDeleteAll(m_session_types);
    delete ui;
}

void DashboardWidget::setCurrentSiteId(const int &id_site)
{
    if (id_site != m_siteId){
        m_siteId = id_site;
        refreshData();
    }
}

void DashboardWidget::on_btnUpcomingSessions_clicked()
{
    ui->frameUpcomingSessions->setVisible(ui->btnUpcomingSessions->isChecked());
    if (!ui->btnUpcomingSessions->isChecked())
        ui->icoUpcomingSessions->setPixmap(QPixmap(":/controls/branch_closed.png"));
    else
        ui->icoUpcomingSessions->setPixmap(QPixmap(":/controls/branch_opened.png"));

    updateUiSpacing();
}


void DashboardWidget::on_btnRecentParticipants_clicked()
{
    ui->frameRecent->setVisible(ui->btnRecentParticipants->isChecked());
    if (!ui->btnRecentParticipants->isChecked())
        ui->icoRecentParticipants->setPixmap(QPixmap(":/controls/branch_closed.png"));
    else
        ui->icoRecentParticipants->setPixmap(QPixmap(":/controls/branch_opened.png"));

    updateUiSpacing();
}


void DashboardWidget::on_btnAttention_clicked()
{
    ui->frameAttention->setVisible(ui->btnAttention->isChecked());
    if (!ui->btnAttention->isChecked())
        ui->icoAttention->setPixmap(QPixmap(":/controls/branch_closed.png"));
    else
        ui->icoAttention->setPixmap(QPixmap(":/controls/branch_opened.png"));

    updateUiSpacing();
}

void DashboardWidget::on_icoUpcomingSessions_clicked()
{
    ui->btnUpcomingSessions->setChecked(!ui->btnUpcomingSessions->isChecked());
    on_btnUpcomingSessions_clicked();
}

void DashboardWidget::on_icoRecentParticipants_clicked()
{
    ui->btnRecentParticipants->setChecked(!ui->btnRecentParticipants->isChecked());
    on_btnRecentParticipants_clicked();
}

void DashboardWidget::on_icoAttention_clicked()
{
    ui->btnAttention->setChecked(!ui->btnAttention->isChecked());
    on_btnAttention_clicked();
}

void DashboardWidget::processSessionsReply(QList<TeraData> sessions)
{
    ui->tableUpcomingSessions->clearContents();
    ui->tableUpcomingSessions->setRowCount(0);
    m_listSessions_items.clear();
    qDeleteAll(m_sessions);
    m_sessions.clear();

    QTableWidgetItem* name_item;
    TableDateWidgetItem* date_item;
    QTableWidgetItem* user_item;
    QTableWidgetItem* project_item;
    QTableWidgetItem* participants_item;
    QTableWidgetItem* type_item;

    for(const TeraData &session: sessions){
        int id_session = session.getId();
        if (std::find(m_listSessions_items.cbegin(), m_listSessions_items.cend(), id_session) != m_listSessions_items.cend()){
            // Already there, get items
           name_item = m_listSessions_items.key(id_session);
           date_item = dynamic_cast<TableDateWidgetItem*>(ui->tableUpcomingSessions->item(name_item->row(), 0));
           type_item = ui->tableUpcomingSessions->item(name_item->row(), 2);
           user_item = ui->tableUpcomingSessions->item(name_item->row(), 3);
           project_item = ui->tableUpcomingSessions->item(name_item->row(), 4);
           participants_item = ui->tableUpcomingSessions->item(name_item->row(), 5);
        }else{
            ui->tableUpcomingSessions->setRowCount(ui->tableUpcomingSessions->rowCount()+1);
            int current_row = ui->tableUpcomingSessions->rowCount()-1;
            name_item = new QTableWidgetItem("");
            name_item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ui->tableUpcomingSessions->setItem(current_row, 1, name_item);
            date_item = new TableDateWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_SESSION)),"");
            date_item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ui->tableUpcomingSessions->setItem(current_row, 0, date_item);
            type_item = new QTableWidgetItem("");
            type_item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ui->tableUpcomingSessions->setItem(current_row, 2, type_item);
            user_item = new QTableWidgetItem("");
            user_item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ui->tableUpcomingSessions->setItem(current_row, 3, user_item);
            project_item = new QTableWidgetItem("");
            project_item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ui->tableUpcomingSessions->setItem(current_row, 4, project_item);
            participants_item = new QTableWidgetItem("");
            ui->tableUpcomingSessions->setItem(current_row, 5, participants_item);

            m_listSessions_items[name_item] = id_session;
        }

        m_sessions[id_session] = new TeraData(session);

        // Update values
        name_item->setText(session.getName());
        QDateTime session_date = session.getFieldValue("session_start_datetime").toDateTime().toLocalTime();
        date_item->setText(session_date.toString("dd-MM-yyyy hh:mm:ss"));

        // Type
        int id_session_type = session.getFieldValue("id_session_type").toInt();
        if (m_session_types.contains(id_session_type)){
            type_item->setText(m_session_types[id_session_type]->getName());
        }else{
            type_item->setText(tr("Inconnu"));
        }

        // Creator
        if (session.hasFieldName("session_creator_user"))
            user_item->setText(session.getFieldValue("session_creator_user").toString());
        else if(session.hasFieldName("session_creator_device"))
            user_item->setText(tr("Appareil: ") + session.getFieldValue("session_creator_device").toString());
        else if(session.hasFieldName("session_creator_participant"))
            user_item->setText(tr("Participant: ") + session.getFieldValue("session_creator_participant").toString());
        else if(session.hasFieldName("session_creator_service"))
            user_item->setText(tr("Service: ") + session.getFieldValue("session_creator_service").toString());
        else {
            user_item->setText(tr("Inconnu"));
        }

        // Invitees
        QStringList invitees;
        QString project_name;
        if (session.hasFieldName("session_participants")){
            QVariantList participants = session.getFieldValue("session_participants").toList();
            for(const QVariant &participant: qAsConst(participants)){
                QVariantHash part_data = participant.toHash();
                if (project_name.isEmpty() && part_data.contains("project_name")){

                    project_name = part_data["project_name"].toString();
                    m_sessions[id_session]->setFieldValue("project_name", project_name);
                    if (part_data.contains("id_project")){
                        m_sessions[id_session]->setFieldValue("id_project", part_data["id_project"]);
                    }
                }
                invitees.append(part_data["participant_name"].toString());
            }
        }
        if (session.hasFieldName("session_users")){
            QVariantList users = session.getFieldValue("session_users").toList();
            for(const QVariant &user: qAsConst(users)){
                QVariantHash user_data = user.toHash();
                invitees.append(user_data["user_name"].toString());
            }
        }
        if (session.hasFieldName("session_devices")){
            QVariantList devices = session.getFieldValue("session_devices").toList();
            for(const QVariant &device: qAsConst(devices)){
                QVariantHash device_data = device.toHash();
                invitees.append(device_data["device_name"].toString());
            }
        }

        project_item->setText(project_name);
        QString invitees_str;
        for (const QString &invitee: invitees)
            invitees_str += invitee + ", ";
        invitees_str = invitees_str.left(invitees_str.length()-2);
        participants_item->setText(invitees_str);


    }

    //ui->tableUpcomingSessions->resizeColumnsToContents();

    ui->lblNoUpcomingSessions->setVisible(m_sessions.isEmpty());
    ui->tableUpcomingSessions->setVisible(!m_sessions.isEmpty());

    if (m_sessions.isEmpty()){
        ui->frameUpcomingSessions->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum));
    }else{
        ui->frameUpcomingSessions->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding));
    }
    ui->btnUpcomingSessions->setChecked(!m_sessions.isEmpty());
    on_btnUpcomingSessions_clicked();

    ui->btnUpcomingSessions->setText(tr("Séances à venir") + " (" + QString::number(m_sessions.count()) + ")");

}

void DashboardWidget::processSessionTypesReply(const QList<TeraData> session_types)
{
    for(const TeraData &session_type: session_types){
        TeraData* ses_type;
        int id_session_type = session_type.getId();
        if (m_session_types.contains(id_session_type)){
            ses_type = m_session_types[id_session_type];
            ses_type->updateFrom(session_type);
        }else{
            ses_type = new TeraData(session_type);
            m_session_types[id_session_type] = ses_type;
        }
    }
}

void DashboardWidget::sessionLobbyStartSessionCancelled()
{
    if (m_sessionLobby){
        m_sessionLobby->deleteLater();
        m_sessionLobby = nullptr;
    }
}

void DashboardWidget::sessionLobbyStartSessionRequested()
{
    int id_session_type = m_sessionLobby->getIdSessionType(); //ui->cmbSessionType->currentData().toInt();
    // Start session
    m_comManager->startSession(*m_session_types[id_session_type],
                               m_sessionLobby->getIdSession(),
                               m_sessionLobby->getSessionParticipantsUuids(),
                               m_sessionLobby->getSessionUsersUuids(),
                               m_sessionLobby->getSessionDevicesUuids(),
                               m_sessionLobby->getSessionConfig());

    m_sessionLobby->deleteLater();
    m_sessionLobby = nullptr;
}

void DashboardWidget::connectSignals()
{
    connect(m_comManager, &ComManager::sessionsReceived, this, &DashboardWidget::processSessionsReply);
    connect(m_comManager, &ComManager::sessionTypesReceived, this, &DashboardWidget::processSessionTypesReply);

}

void DashboardWidget::refreshData()
{
    // Refresh upcoming sessions
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_USER, QString::number(m_comManager->getCurrentUser().getId()));
    args.addQueryItem(WEB_QUERY_LIMIT, QString::number(10));
    args.addQueryItem(WEB_QUERY_STATUS, QString::number(TeraSessionStatus::STATUS_NOTSTARTED));
    m_comManager->doQuery(WEB_SESSIONINFO_PATH, args);

    updateUiSpacing();
}

void DashboardWidget::updateUiSpacing()
{
    bool all_closed = !ui->btnAttention->isChecked() && !ui->btnRecentParticipants->isChecked() && !ui->btnUpcomingSessions->isChecked();

    if (all_closed){
        ui->verticalSpacer->changeSize(20, 40, QSizePolicy::Maximum,QSizePolicy::Expanding);
    }else{
        ui->verticalSpacer->changeSize(0, 0, QSizePolicy::Maximum, QSizePolicy::Maximum);
    }
}

void DashboardWidget::showSessionLobby(const int &id_session_type, const int &id_session)
{
    if (m_sessionLobby)
        m_sessionLobby->deleteLater();

    if (!m_session_types.contains(id_session_type)){
        GlobalMessageBox msg;
        msg.showError(tr("Impossible de démarrer la séance"), tr("Cette séance ne peut être démarrée: le type de séance est inconnu."));
        return;
    }

    if (!m_sessions.contains(id_session)){
        GlobalMessageBox msg;
        msg.showError(tr("Impossible de démarrer la séance"), tr("Cette séance ne peut être démarrée: séance introuvable."));
        return;
    }

    int id_project = 0;
    TeraData* session = m_sessions[id_session];
    if (session->hasFieldName("id_project")){
        id_project = m_sessions[id_session]->getFieldValue("id_project").toInt();
    }

    m_sessionLobby = new SessionLobbyDialog(m_comManager, *m_session_types[id_session_type], id_project, id_session, this);


    // Add participants to session
    QList<TeraData> participants;
    QList<int> ids;
    if (session->hasFieldName("session_participants")){
        QVariantList session_participants = session->getFieldValue("session_participants").toList();
        for(const QVariant &participant: qAsConst(session_participants)){
            TeraData part_data(TERADATA_PARTICIPANT, participant.toJsonValue());
            participants.append(part_data);
            ids.append(part_data.getId());
        }
    }
    m_sessionLobby->addParticipantsToSession(participants, ids);
    ids.clear();
    participants.clear();

    // Add users to session
    QList<TeraData> users;
    if (session->hasFieldName("session_users")){
        QVariantList session_users = session->getFieldValue("session_users").toList();
        for(const QVariant &user: qAsConst(session_users)){
            TeraData user_data(TERADATA_USER, user.toJsonValue());
            users.append(user_data);
            ids.append(user_data.getId());
        }
    }
    m_sessionLobby->addUsersToSession(users, ids);
    ids.clear();
    users.clear();

    // Add devices to session
    QList<TeraData> devices;
    if (session->hasFieldName("session_devices")){
        QVariantList session_devices = session->getFieldValue("session_devices").toList();
        for(const QVariant &device: qAsConst(session_devices)){
            TeraData device_data(TERADATA_DEVICE, device.toJsonValue());
            devices.append(device_data);
            ids.append(device_data.getId());
        }
    }
    m_sessionLobby->addDevicesToSession(devices, ids);
    ids.clear();
    devices.clear();

    connect(m_sessionLobby, &QDialog::accepted, this, &DashboardWidget::sessionLobbyStartSessionRequested);
    connect(m_sessionLobby, &QDialog::rejected, this, &DashboardWidget::sessionLobbyStartSessionCancelled);
    if (height()<800)
        m_sessionLobby->showMaximized();

    // Show Session Lobby
    m_sessionLobby->exec();
}


void DashboardWidget::on_tableUpcomingSessions_itemDoubleClicked(QTableWidgetItem *item)
{
    int current_row = item->row();
    QTableWidgetItem* base_item = ui->tableUpcomingSessions->item(current_row, 1);

    if (m_listSessions_items.contains(base_item)){
        int id_session = m_listSessions_items.value(base_item);
        showSessionLobby(m_sessions[id_session]->getFieldValue("id_session_type").toInt(), id_session);
    }
}

