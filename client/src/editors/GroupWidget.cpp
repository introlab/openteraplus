#include "GroupWidget.h"
#include "ui_GroupWidget.h"

#include "services/BaseServiceWidget.h"
#include "widgets/SessionInviteWidget.h"
#include "TeraSettings.h"

GroupWidget::GroupWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::GroupWidget)
{   

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    setLimited(false);
#ifndef OPENTERA_WEBASSEMBLY
    m_sessionLobby = nullptr;
#endif
    // Use base class to manage editing
    setEditorControls(ui->wdgGroup, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    // Query participants of that group
    /*if (data->getId() != 0){
        QUrlQuery query;
        query.addQueryItem(WEB_QUERY_ID_GROUP, QString::number(m_data->getId()));
        queryDataRequest(WEB_PARTICIPANTINFO_PATH, query);
    }*/

    ui->wdgGroup->setComManager(m_comManager);
    GroupWidget::setData(data);

    // Query form definition
    QUrlQuery args(WEB_FORMS_QUERY_GROUP);
    if (m_data->hasFieldName("id_site")){
        args.addQueryItem(WEB_QUERY_ID_SITE, m_data->getFieldValue("id_site").toString());
        m_data->removeFieldName("id_site");
    }else{
        if (!dataIsNew()){
            args.addQueryItem(WEB_QUERY_ID, QString::number(m_data->getId()));
        }
    }

    queryDataRequest(WEB_FORMS_PATH, args);

    // Query session types for that group's project
    if (!dataIsNew()){
        if (m_data->hasFieldName("id_project")){
            args.clear();
            args.addQueryItem(WEB_QUERY_ID_PROJECT, m_data->getFieldValue("id_project").toString());
            queryDataRequest(WEB_SESSIONTYPE_PATH, args);
        }
    }

}

GroupWidget::~GroupWidget()
{
    if (ui) {
        delete ui;
    }

    qDeleteAll(m_ids_session_types);
#ifndef OPENTERA_WEBASSEMBLY
    if (m_sessionLobby) {
        m_sessionLobby->deleteLater();
    }
#endif
}

void GroupWidget::saveData(bool signal){

    // If data is new, we request all the fields.
    QJsonDocument group_data = ui->wdgGroup->getFormDataJson(m_data->isNew());

    postDataRequest(WEB_GROUPINFO_PATH, group_data.toJson());

    if (signal){
        TeraData* new_data = ui->wdgGroup->getFormDataObject(TERADATA_GROUP);
        *m_data = *new_data;
        delete new_data;
        emit dataWasChanged();
    }
}

void GroupWidget::updateControlsState(){
    ui->grpSummary->setVisible(!dataIsNew());
    ui->frameActions->setVisible(!dataIsNew());

    if (dataIsNew()){
        ui->frameNewSession->hide();
    }
}

void GroupWidget::updateFieldsValue(){
    if (m_data){
        ui->wdgGroup->fillFormFromData(m_data->toJson());
        ui->lblTitle->setText(m_data->getName());

        bool can_start = canStartSession();
        ui->btnNewSession->setEnabled(can_start);
        ui->cmbSessionType->setVisible(can_start);

    }
}

void GroupWidget::setData(const TeraData *data)
{
    DataEditorWidget::setData(data);

    if (!dataIsNew()){
        // Query stats
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_GROUP, QString::number(m_data->getId()));
        args.addQueryItem(WEB_QUERY_WITH_PARTICIPANTS, "1");
        queryDataRequest(WEB_STATS_PATH, args);

    }
}

bool GroupWidget::canStartSession()
{
    if (m_ids_session_types.isEmpty()){
        return false;
    }

    if (m_activeParticipants.isEmpty()){
        ui->lblInfos->setText(tr("Aucun participant actif dans ce groupe"));
        return false;
    }

    if (m_activeParticipants.count() + 1 > MAX_INVITEES_IN_SESSION){ // +1 since including the current user!
        int max_participants = int(MAX_INVITEES_IN_SESSION) - 1;
        ui->lblInfos->setText(tr("Trop de participants actifs dans ce groupe pour démarrer une séance (max ") +
                              QString::number(max_participants) + " " + tr("participants") + ")");
        return false;
    }

    return true;
}

bool GroupWidget::validateData(){
    bool valid = false;

    valid = ui->wdgGroup->validateFormData();

    return valid;
}

void GroupWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type.startsWith(WEB_FORMS_QUERY_GROUP)){
        ui->wdgGroup->buildUiFromStructure(data);
        return;
    }
}

void GroupWidget::processStatsReply(TeraData stats, QUrlQuery reply_query)
{
    // Check if stats are for us
    if (!reply_query.hasQueryItem("id_group"))
        return;

    if (reply_query.queryItemValue("id_group").toInt() != m_data->getId())
        return;

    // Fill stats information
    ui->lblParticipant->setText(stats.getFieldValue("participants_total_count").toString() + tr(" Participants") + "\n"
                                + stats.getFieldValue("participants_enabled_count").toString() + tr(" Participants actifs"));
    ui->lblSessions->setText(stats.getFieldValue("sessions_total_count").toString() + tr(" Séances planifiées \nou réalisées"));

     // Fill participants information
    if (stats.hasFieldName("participants")){
        ui->tableSummary->clearContents();
        m_tableParticipants_items.clear();
        m_activeParticipants.clear();

        QVariantList parts_list = stats.getFieldValue("participants").toList();

        for(const QVariant &part:std::as_const(parts_list)){
            QVariantMap part_info = part.toMap();
            int part_id = part_info["id_participant"].toInt();

            ui->tableSummary->setRowCount(ui->tableSummary->rowCount()+1);
            int current_row = ui->tableSummary->rowCount()-1;
            QTableWidgetItem* item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_PARTICIPANT)),
                                                                part_info["participant_name"].toString());
            m_tableParticipants_items[part_id] = item;
            ui->tableSummary->setItem(current_row, 0, item);

            item = new QTableWidgetItem();
            QString status;
            if (part_info["participant_enabled"].toBool() == true){
                status = tr("Actif");
                item->setForeground(Qt::green);
                if (!m_activeParticipants.contains(part_id))
                    m_activeParticipants.append(part_id);
                ui->tableSummary->showRow(current_row);
            }else{
                status = tr("Inactif");
                item->setForeground(Qt::red);
                if (!ui->chkShowInactive->isChecked())
                    ui->tableSummary->hideRow(current_row);
            }
            item->setText(status);
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableSummary->setItem(current_row, 1, item);

            item = new TableNumberWidgetItem(part_info["participant_sessions_count"].toString());
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableSummary->setItem(current_row, 2, item);

            item = new TableDateWidgetItem(part_info["participant_first_session"].toDateTime().toLocalTime().toString("dd-MM-yyyy hh:mm:ss"));
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableSummary->setItem(current_row, 3, item);

            QDateTime last_session_datetime = part_info["participant_last_session"].toDateTime().toLocalTime();
            item = new TableDateWidgetItem(last_session_datetime.toString("dd-MM-yyyy hh:mm:ss"));
            if (part_info["participant_enabled"].toBool() == true && last_session_datetime.isValid()){
                // Set background color
                QColor back_color = TeraForm::getGradientColor(0, 5, 10, static_cast<int>(last_session_datetime.daysTo(QDateTime::currentDateTime())));
                back_color.setAlphaF(0.5);
                item->setBackground(back_color);
            }
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableSummary->setItem(current_row, 4, item);

            QString last_connect;
            QDateTime last_connect_datetime;
            if (part_info.contains("participant_last_online")){
                last_connect_datetime =  part_info["participant_last_online"].toDateTime().toLocalTime();
                if (last_connect_datetime.isValid())
                    last_connect = last_connect_datetime.toString("dd-MM-yyyy hh:mm:ss");
            }
            item = new TableDateWidgetItem(last_connect);
            item->setTextAlignment(Qt::AlignCenter);

            if (part_info["participant_enabled"].toBool() == true && last_connect_datetime.isValid()){
                // Set background color
                QColor back_color = TeraForm::getGradientColor(0, 5, 10, static_cast<int>(last_connect_datetime.daysTo(QDateTime::currentDateTime())));
                back_color.setAlphaF(0.5);
                item->setBackground(back_color);
            }
            ui->tableSummary->setItem(current_row, 5, item);
        }
        ui->tableSummary->resizeColumnsToContents();
        ui->tableSummary->sortByColumn(4, Qt::DescendingOrder); // Sort by last session date
    }

}

void GroupWidget::processSessionTypesReply(QList<TeraData> session_types)
{
    for (const TeraData &st:session_types){
        if (!m_ids_session_types.contains(st.getId())){
            m_ids_session_types[st.getId()] = new TeraData(st);

            // New session ComboBox
            QString ses_type_name = st.getName();

            // Check if it is a session type handled
            if (st.hasFieldName("session_type_category")){
                TeraSessionCategory::SessionTypeCategories st_category = static_cast<TeraSessionCategory::SessionTypeCategories>(st.getFieldValue("session_type_category").toInt());
                if (BaseServiceWidget::getHandledSessionCategories().contains(st_category)){
                    bool can_start_session = true;
                    if (st_category == TeraSessionCategory::SESSION_TYPE_SERVICE){
                        if (st.hasFieldName("session_type_service_key")){
                            can_start_session = BaseServiceWidget::getHandledServiceKeys().contains(st.getFieldValue("session_type_service_key").toString());
                        }else{
                            can_start_session=false;
                        }
                    }
                    if (can_start_session){
                        ui->cmbSessionType->addItem(ses_type_name, st.getId());
                    }
                }
            }
        }else{
            // Existing, must update values
            *m_ids_session_types[st.getId()] = st;
            for (int i=0; i<ui->cmbSessionType->count(); i++){
                if (ui->cmbSessionType->itemData(i).toInt() == st.getId()){
                    ui->cmbSessionType->setItemText(i, st.getName());
                }
            }
        }
    }

    // Query sessions for that participant
    if (!m_data->isNew()){
        // Select current session type based on last session type used with that participant
        int last_session_type_id = TeraSettings::getUserSettingForProject(m_comManager->getCurrentUser().getUuid(),
                                                                          m_data->getFieldValue("id_project").toInt(),
                                                                          SETTINGS_LASTSESSIONTYPEID).toInt();

        if (last_session_type_id > 0){
            // We have a setting - try to select it from the list
            for (int i=0; i<ui->cmbSessionType->count(); i++){
                if (ui->cmbSessionType->itemData(i).toInt() == last_session_type_id){
                    ui->cmbSessionType->setCurrentIndex(i);
                    break;
                }
            }
        }
    }

    // Hide new session frame if no session type
    ui->frameNewSession->setVisible(!m_ids_session_types.isEmpty());
}

void GroupWidget::postResultReply(QString path)
{
    // OK, data was saved!
    if (path==WEB_GROUPINFO_PATH){
        if (parent())
            emit closeRequest();
    }
}

void GroupWidget::deleteDataReply(QString path, int del_id)
{
    if (path == WEB_PARTICIPANTINFO_PATH){
        // A participant got deleted - check if it affects the current display
        if (m_tableParticipants_items.contains(del_id)){
            ui->tableSummary->removeRow(m_tableParticipants_items[del_id]->row());
            m_tableParticipants_items.remove(del_id);
        }
    }
}
#ifndef OPENTERA_WEBASSEMBLY
void GroupWidget::showSessionLobby(const int &id_session_type, const int &id_session)
{
    if (m_sessionLobby)
        m_sessionLobby->deleteLater();
    m_sessionLobby = new SessionLobbyDialog(m_comManager, *m_ids_session_types[id_session_type], m_data->getFieldValue("id_project").toInt(), id_session, this);


    // Add current participants to session
    //m_sessionLobby->addParticipantsToSession(QList<TeraData>() << *m_data, QList<int>() << m_data->getId());
    for(int id_part:std::as_const(m_activeParticipants)){
        TeraData part_data;
        part_data.setDataType(TeraDataTypes::TERADATA_PARTICIPANT);
        part_data.setId(id_part);
        part_data.setName(m_tableParticipants_items[id_part]->text());
        m_sessionLobby->addParticipantsToSession(QList<TeraData>() << part_data/*, QList<int>() << id_part*/);
    }

    // Add current user to session
    m_sessionLobby->addUsersToSession(QList<TeraData>() << m_comManager->getCurrentUser(), QList<int>() <<m_comManager->getCurrentUser().getId());

    connect(m_sessionLobby, &QDialog::accepted, this, &GroupWidget::sessionLobbyStartSessionRequested);
    connect(m_sessionLobby, &QDialog::rejected, this, &GroupWidget::sessionLobbyStartSessionCancelled);
    if (height()<800)
        m_sessionLobby->showMaximized();

    // Show Session Lobby
    m_sessionLobby->exec();
}

void GroupWidget::sessionLobbyStartSessionRequested()
{
    int id_session_type = m_sessionLobby->getIdSessionType(); //ui->cmbSessionType->currentData().toInt();

    // Save last session type for further use
    TeraSettings::setUserSettingForProject(m_comManager->getCurrentUser().getUuid(),
                                           m_data->getFieldValue("id_project").toInt(),
                                           SETTINGS_LASTSESSIONTYPEID,
                                           id_session_type);

    // Start session
    m_comManager->startSession(*m_ids_session_types[id_session_type],
                               m_sessionLobby->getIdSession(),
                               m_sessionLobby->getSessionParticipantsUuids(),
                               m_sessionLobby->getSessionUsersUuids(),
                               m_sessionLobby->getSessionDevicesUuids(),
                               m_sessionLobby->getSessionConfig());

    m_sessionLobby->deleteLater();
    m_sessionLobby = nullptr;
}

void GroupWidget::sessionLobbyStartSessionCancelled()
{
    if (m_sessionLobby){
        m_sessionLobby->deleteLater();
        m_sessionLobby = nullptr;
    }
}

#endif
void GroupWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &GroupWidget::processFormsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &GroupWidget::postResultReply);
    connect(m_comManager, &ComManager::statsReceived, this, &GroupWidget::processStatsReply);
    connect(m_comManager, &ComManager::deleteResultsOK, this, &GroupWidget::deleteDataReply);
    connect(m_comManager, &ComManager::sessionTypesReceived, this, &GroupWidget::processSessionTypesReply);
}



void GroupWidget::on_btnNewParticipant_clicked()
{
    emit dataDisplayRequest(TERADATA_PARTICIPANT, 0);
}


void GroupWidget::on_btnDelete_clicked()
{
    GlobalMessageBox diag;
    QMessageBox::StandardButton answer = QMessageBox::No;
    if (ui->tableSummary->selectionModel()->selectedRows().count() == 1){
        QTableWidgetItem* base_item = ui->tableSummary->item(ui->tableSummary->currentRow(),0);
        answer = diag.showYesNo(tr("Suppression?"), tr("Êtes-vous sûrs de vouloir supprimer """) + base_item->text() + """?");
    }else{
        answer = diag.showYesNo(tr("Suppression?"), tr("Êtes-vous sûrs de vouloir supprimer tous les participants sélectionnés?"));
    }

    if (answer == QMessageBox::Yes){
        // We must delete!
        foreach(QModelIndex selected_row, ui->tableSummary->selectionModel()->selectedRows()){
            QTableWidgetItem* base_item = ui->tableSummary->item(selected_row.row(),0); // Get item at index 0 which is linked to session id
            //m_comManager->doDelete(TeraData::getPathForDataType(TERADATA_SESSION), m_listSessions_items.key(base_item));
            m_comManager->doDelete(TeraData::getPathForDataType(TERADATA_PARTICIPANT), m_tableParticipants_items.key(base_item));
        }
    }
}


void GroupWidget::on_tableSummary_itemSelectionChanged()
{
    ui->btnDelete->setEnabled(!ui->tableSummary->selectedItems().isEmpty());
}

#ifndef OPENTERA_WEBASSEMBLY
void GroupWidget::on_btnNewSession_clicked()
{
    if (ui->cmbSessionType->currentIndex() < 0)
        return;

    if (!m_data)
        return;

    int id_session_type = ui->cmbSessionType->currentData().toInt();
    int id_session = 0;

    showSessionLobby(id_session_type, id_session);
}
#endif

void GroupWidget::on_chkShowInactive_stateChanged(int checked)
{
    for(QTableWidgetItem* item: m_tableParticipants_items){
        int row = item->row();
        if (ui->chkShowInactive->isChecked()){
            if (ui->tableSummary->isRowHidden(row))
                ui->tableSummary->showRow(row);
        }else{
            bool active = ui->tableSummary->item(row, 1)->foreground() != Qt::red;
            if (!active){
                ui->tableSummary->hideRow(row);
            }
        }
    }
    ui->tableSummary->resizeColumnsToContents();
}

