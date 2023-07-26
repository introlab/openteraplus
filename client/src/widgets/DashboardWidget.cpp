#include "DashboardWidget.h"
#include "ui_DashboardWidget.h"

DashboardWidget::DashboardWidget(ComManager *comMan, int id_site, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DashboardWidget),
    m_comManager(comMan)
{
    ui->setupUi(this);
#ifndef OPENTERA_WEBASSEMBLY
    m_sessionLobby = nullptr;
#endif
    m_cleanupDiag = nullptr;
    ui->tableUpcomingSessions->hide();
    ui->tableRecentParticipants->hide();
    ui->treeWarnings->hide();

    connectSignals();

    // Query available sessions types for the current user
    m_comManager->doGet(WEB_SESSIONTYPE_PATH);

    setCurrentSiteId(id_site, false);

}

DashboardWidget::~DashboardWidget()
{
    qDeleteAll(m_sessions);
    qDeleteAll(m_session_types);

    if (m_cleanupDiag) {
        m_cleanupDiag->deleteLater();
    }

#ifndef OPENTERA_WEBASSEMBLY
    if (m_sessionLobby) {
        m_sessionLobby->deleteLater();
    }
#endif

    delete ui;
}

void DashboardWidget::setCurrentSiteId(const int &id_site, const bool &refresh_data)
{
    if (id_site != m_siteId){
        m_siteId = id_site;
    }
    if (refresh_data)
        refreshData();
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

void DashboardWidget::processSessionsReply(const QList<TeraData> sessions)
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

    ui->tableUpcomingSessions->resizeColumnToContents(0);

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

    // Refresh data
    refreshData();

    // Disconnect signal since we don't need it anymore
    disconnect(m_comManager, &ComManager::sessionTypesReceived, this, &DashboardWidget::processSessionTypesReply);
}

void DashboardWidget::processParticipantsReply(const QList<TeraData> participants, const QUrlQuery reply_query)
{
    if (!reply_query.hasQueryItem(WEB_QUERY_ID_SITE))
        return;

    if (reply_query.queryItemValue(WEB_QUERY_ID_SITE).toInt() != m_siteId)
        return;

    ui->tableRecentParticipants->clearContents();
    ui->tableRecentParticipants->setRowCount(0);
    m_listParticipants_items.clear();

    QTableWidgetItem* name_item;
    TableDateWidgetItem* date_item;
    QTableWidgetItem* project_item;

    for(const TeraData &participant: participants){
        int id_participant = participant.getId();
        if (std::find(m_listParticipants_items.cbegin(), m_listParticipants_items.cend(), id_participant) != m_listParticipants_items.cend()){
            // Already there, get items
           name_item = m_listParticipants_items.key(id_participant);
           date_item = dynamic_cast<TableDateWidgetItem*>(ui->tableRecentParticipants->item(name_item->row(), 2));
           project_item = ui->tableRecentParticipants->item(name_item->row(), 1);
        }else{
            ui->tableRecentParticipants->setRowCount(ui->tableRecentParticipants->rowCount()+1);
            int current_row = ui->tableRecentParticipants->rowCount()-1;
            name_item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_PARTICIPANT)),"");
            //name_item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ui->tableRecentParticipants->setItem(current_row, 0, name_item);
            date_item = new TableDateWidgetItem("");
            date_item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ui->tableRecentParticipants->setItem(current_row, 2, date_item);
            project_item = new QTableWidgetItem("");
            project_item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ui->tableRecentParticipants->setItem(current_row, 1, project_item);

            m_listParticipants_items[name_item] = id_participant;
        }

        // Update values
        name_item->setText(participant.getName());

        if (participant.hasFieldName("participant_project")){
            QVariantHash project_info = participant.getFieldValue("participant_project").toHash();
            if (project_info.contains("project_name")){
                project_item->setText(project_info["project_name"].toString());
            }
        }

        if (participant.hasFieldName("participant_lastsession")){
            date_item->setDate(participant.getFieldValue("participant_lastsession"));
            if (!participant.getFieldValue("participant_lastsession").toString().isEmpty()){
                // Set background color
                QColor back_color = TeraForm::getGradientColor(0, 7, 14, static_cast<int>(participant.getFieldValue("participant_lastsession")
                                                                                          .toDateTime().toLocalTime().daysTo(QDateTime::currentDateTime())));
                back_color.setAlphaF(0.5);
                date_item->setBackground(back_color);
            }
        }

    }

    ui->lblNoRecentParticipants->setVisible(m_listParticipants_items.isEmpty());
    ui->tableRecentParticipants->setVisible(!m_listParticipants_items.isEmpty());
    ui->tableRecentParticipants->resizeColumnToContents(0);
    //ui->tableRecentParticipants->resizeColumnsToContents();

    if (m_listParticipants_items.isEmpty()){
        ui->frameRecent->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum));
    }else{
        ui->frameRecent->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding));
    }
    ui->btnRecentParticipants->setChecked(!m_listParticipants_items.isEmpty());
    on_btnRecentParticipants_clicked();

    ui->btnRecentParticipants->setText(tr("Participants récents") + " (" + QString::number(m_listParticipants_items.count()) + ")");


}

void DashboardWidget::processStatsReply(const TeraData stats, const QUrlQuery reply_query)
{
    if (!reply_query.hasQueryItem(WEB_QUERY_ID_SITE))
        return;

    if (reply_query.queryItemValue(WEB_QUERY_ID_SITE).toInt() != m_siteId)
        return;

    ui->treeWarnings->clear();
    int warnings = 0;
    int total_warnings = 0;
    ui->treeWarnings->setColumnWidth(0, 48);
    if (stats.hasFieldName("warning_participants_count")){
        warnings = stats.getFieldValue("warning_participants_count").toInt();
        if (warnings > 0){
            QTreeWidgetItem* item = new QTreeWidgetItem();
            item->setText(1, QString::number(warnings) + " " + tr("participant(s) sans séance depuis au moins 6 mois"));
            item->setIcon(1, QIcon("://status/warning.png"));
            item->setForeground(1, Qt::yellow);
            ui->treeWarnings->addTopLevelItem(item);
            QToolButton* manage_btn = createManageWarningButton();
            manage_btn->setProperty("data", stats.getFieldValue("warning_participants"));
            manage_btn->setProperty("data_type", TERADATA_PARTICIPANT);
            ui->treeWarnings->setItemWidget(item, 0, manage_btn);  

            // Participants details
            QVariantList participants = stats.getFieldValue("warning_participants").toList();
            for (const QVariant &part: qAsConst(participants)){
                QVariantHash part_data = part.toHash();
                QString data_str;
                data_str = part_data["participant_name"].toString();
                if (part_data.contains("project_name"))
                    data_str += " (" + part_data["project_name"].toString() + ")";

                if (part_data.contains("months"))
                    data_str += " - " + part_data["months"].toString() + " " + tr("mois");
                QTreeWidgetItem* part_item = new QTreeWidgetItem(item);
                part_item->setText(1, data_str);
                part_item->setIcon(1, QIcon("://icons/patient.png"));
            }
        }
    }
    total_warnings += warnings;

    if (stats.hasFieldName("warning_nosession_participants_count")){
        warnings = stats.getFieldValue("warning_nosession_participants_count").toInt();
        if (warnings > 0){
            QTreeWidgetItem* item = new QTreeWidgetItem();
            item->setText(1, QString::number(warnings) + " " + tr("participant(s) sans aucune séance réalisée"));
            item->setIcon(1, QIcon("://status/warning.png"));
            item->setForeground(1, Qt::yellow);
            ui->treeWarnings->addTopLevelItem(item);
            QToolButton* manage_btn = createManageWarningButton();
            manage_btn->setProperty("data", stats.getFieldValue("warning_nosession_participants"));
            manage_btn->setProperty("data_type", TERADATA_PARTICIPANT);
            ui->treeWarnings->setItemWidget(item, 0, manage_btn);

            // Participants details
            QVariantList participants = stats.getFieldValue("warning_nosession_participants").toList();
            for (const QVariant &part: qAsConst(participants)){
                QVariantHash part_data = part.toHash();
                QString data_str;
                data_str = part_data["participant_name"].toString();
                if (part_data.contains("project_name"))
                    data_str += " (" + part_data["project_name"].toString() + ")";

                if (part_data.contains("months"))
                    data_str += " - " + tr("Créé depuis") + " " + part_data["months"].toString() + " " + tr("mois");
                QTreeWidgetItem* part_item = new QTreeWidgetItem(item);
                part_item->setText(1, data_str);
                part_item->setIcon(1, QIcon("://icons/patient.png"));
            }
        }
    }
    total_warnings += warnings;

    if (m_comManager->getCurrentUserSiteRole(m_siteId) == "admin"){
        if (stats.hasFieldName("warning_users_count")){
            warnings = stats.getFieldValue("warning_users_count").toInt();
            if (warnings > 0){
                QTreeWidgetItem* item = new QTreeWidgetItem();
                item->setIcon(1, QIcon("://status/warning.png"));
                item->setForeground(1, Qt::yellow);
                item->setText(1, QString::number(warnings) + " " + tr("utilisateur(s) non-connecté(s) depuis au moins 6 mois"));
                ui->treeWarnings->addTopLevelItem(item);
                QToolButton* manage_btn = createManageWarningButton();
                manage_btn->setProperty("data", stats.getFieldValue("warning_users"));
                manage_btn->setProperty("data_type", TERADATA_USER);
                ui->treeWarnings->setItemWidget(item, 0, manage_btn);

                // Users details
                QVariantList users = stats.getFieldValue("warning_users").toList();
                for (const QVariant &user: qAsConst(users)){
                    QVariantHash user_data = user.toHash();
                    QString data_str;
                    data_str = user_data["user_fullname"].toString();
                    if (user_data.contains("months"))
                        data_str += " - " + user_data["months"].toString() + " " + tr("mois");
                    QTreeWidgetItem* part_item = new QTreeWidgetItem(item);
                    part_item->setText(1, data_str);
                    part_item->setIcon(1, QIcon("://icons/software_user.png"));
                }
            }
        }
        total_warnings += warnings;

        if (stats.hasFieldName("warning_neverlogged_users_count")){
            warnings = stats.getFieldValue("warning_neverlogged_users_count").toInt();
            if (warnings > 0){
                QTreeWidgetItem* item = new QTreeWidgetItem();
                item->setIcon(1, QIcon("://status/warning.png"));
                item->setForeground(1, Qt::yellow);
                item->setText(1, QString::number(warnings) + " " + tr("utilisateur(s) jamais connecté(s)"));
                ui->treeWarnings->addTopLevelItem(item);
                QToolButton* manage_btn = createManageWarningButton();
                manage_btn->setProperty("data", stats.getFieldValue("warning_neverlogged_users"));
                manage_btn->setProperty("data_type", TERADATA_USER);
                ui->treeWarnings->setItemWidget(item, 0, manage_btn);

                // Users details
                QVariantList users = stats.getFieldValue("warning_neverlogged_users").toList();
                for (const QVariant &user: qAsConst(users)){
                    QVariantHash user_data = user.toHash();
                    QString data_str;
                    data_str = user_data["user_fullname"].toString();
                    if (user_data.contains("months"))
                        data_str += " - " + tr("Créé depuis") + " " + user_data["months"].toString() + " " + tr("mois");
                    QTreeWidgetItem* part_item = new QTreeWidgetItem(item);
                    part_item->setText(1, data_str);
                    part_item->setIcon(1, QIcon("://icons/software_user.png"));
                }
            }
        }
        total_warnings += warnings;
    }

    //ui->treeWarnings->resizeColumnToContents(0);

    ui->lblAttention->setVisible(ui->treeWarnings->topLevelItemCount() == 0);
    ui->treeWarnings->setVisible(ui->treeWarnings->topLevelItemCount() > 0);

    if (ui->treeWarnings->topLevelItemCount() == 0){
        ui->frameAttention->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum));
    }else{
        ui->frameAttention->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding));
    }
    ui->btnAttention->setChecked(ui->treeWarnings->topLevelItemCount() > 0);
    on_btnAttention_clicked();

    ui->btnAttention->setText(tr("Attention requise") + " (" + QString::number(total_warnings) + ")");

}
#ifndef OPENTERA_WEBASSEMBLY
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
#endif

void DashboardWidget::cleanupDialogCompleted()
{
    if (m_cleanupDiag){
        //if (m_cleanupDiag->result() == QDialog::Accepted)
        refreshData();
        m_cleanupDiag->deleteLater();
        m_cleanupDiag = nullptr;
    }
}

void DashboardWidget::connectSignals()
{
    connect(m_comManager, &ComManager::sessionsReceived, this, &DashboardWidget::processSessionsReply);
    connect(m_comManager, &ComManager::sessionTypesReceived, this, &DashboardWidget::processSessionTypesReply);
    connect(m_comManager, &ComManager::participantsReceived, this, &DashboardWidget::processParticipantsReply);
    connect(m_comManager, &ComManager::statsReceived, this, &DashboardWidget::processStatsReply);

}

void DashboardWidget::refreshData()
{
    // Refresh upcoming sessions
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_USER, QString::number(m_comManager->getCurrentUser().getId()));
    args.addQueryItem(WEB_QUERY_LIMIT, QString::number(20));
    args.addQueryItem(WEB_QUERY_STATUS, QString::number(TeraSessionStatus::STATUS_NOTSTARTED));
    m_comManager->doGet(WEB_SESSIONINFO_PATH, args);

    // Refresh recent participants list
    args.clear();
    args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_siteId));
    args.addQueryItem(WEB_QUERY_LIMIT, QString::number(10));
    args.addQueryItem(WEB_QUERY_ORDERBY_RECENTS, "1");
    args.addQueryItem(WEB_QUERY_FULL, "1");
    args.addQueryItem(WEB_QUERY_ENABLED,"1");
    m_comManager->doGet(WEB_PARTICIPANTINFO_PATH, args);

    // Refresh warnings
    args.clear();
    args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_siteId));
    args.addQueryItem(WEB_QUERY_WITH_WARNINGS, "1");
    m_comManager->doGet(WEB_STATS_PATH, args);

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

#ifndef OPENTERA_WEBASSEMBLY
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
#endif

QToolButton *DashboardWidget::createManageWarningButton()
{
    QToolButton* manage_btn = new QToolButton();
    manage_btn->setIcon(QIcon("://icons/config.png"));
    manage_btn->setIconSize(QSize(20,20));
    manage_btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    manage_btn->setFixedSize(28, 28);
    manage_btn->setToolTip(tr("Gérer"));
    manage_btn->setCursor(Qt::PointingHandCursor);
    manage_btn->setStyleSheet("QToolButton::hover{background-color:transparent;}");

    connect(manage_btn, &QToolButton::clicked, this, &DashboardWidget::btnManageWarning_clicked);

    return manage_btn;
}


void DashboardWidget::on_tableUpcomingSessions_itemDoubleClicked(QTableWidgetItem *item)
{
#ifndef OPENTERA_WEBASSEMBLY
    int current_row = item->row();
    QTableWidgetItem* base_item = ui->tableUpcomingSessions->item(current_row, 1);
    if (m_listSessions_items.contains(base_item)){
        int id_session = m_listSessions_items.value(base_item);
        showSessionLobby(m_sessions[id_session]->getFieldValue("id_session_type").toInt(), id_session);
    }
#endif
}


void DashboardWidget::on_tableRecentParticipants_itemDoubleClicked(QTableWidgetItem *item)
{
    int current_row = item->row();
    QTableWidgetItem* base_item = ui->tableRecentParticipants->item(current_row, 0);

    if (m_listParticipants_items.contains(base_item)){
        int id_participant = m_listParticipants_items.value(base_item);
        emit dataDisplayRequest(TERADATA_PARTICIPANT, id_participant);
    }
}

void DashboardWidget::btnManageWarning_clicked()
{
    // Check if the sender is a QToolButton (from the action column)
    QToolButton* action_btn = dynamic_cast<QToolButton*>(sender());
    if (action_btn){
        QVariantList data = action_btn->property("data").toList();
        TeraDataTypes data_type = static_cast<TeraDataTypes>(action_btn->property("data_type").toInt());

        if (m_cleanupDiag){
            m_cleanupDiag->deleteLater();
        }

        m_cleanupDiag = new CleanUpDialog(m_comManager, data, data_type);
        m_cleanupDiag->setMinimumWidth(2*width()/3);
        m_cleanupDiag->setMinimumHeight(2*height()/3);
        connect(m_cleanupDiag, &CleanUpDialog::accepted, this, &DashboardWidget::cleanupDialogCompleted);
        connect(m_cleanupDiag, &CleanUpDialog::rejected, this, &DashboardWidget::cleanupDialogCompleted);

        m_cleanupDiag->exec();
    }
}

