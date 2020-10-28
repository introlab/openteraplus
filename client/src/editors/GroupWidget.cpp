#include "GroupWidget.h"
#include "ui_GroupWidget.h"

GroupWidget::GroupWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::GroupWidget)
{   

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    setLimited(false);

    // Use base class to manage editing
    setEditorControls(ui->wdgGroup, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Connect signals and slots
    connectSignals();

    // Query form definition
    queryDataRequest(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_GROUP));

    // Query participants of that group
    /*if (data->getId() != 0){
        QUrlQuery query;
        query.addQueryItem(WEB_QUERY_ID_GROUP, QString::number(m_data->getId()));
        queryDataRequest(WEB_PARTICIPANTINFO_PATH, query);
    }*/

    ui->wdgGroup->setComManager(m_comManager);
    setData(data);

}

GroupWidget::~GroupWidget()
{
    if (ui)
        delete ui;
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
}

void GroupWidget::updateFieldsValue(){
    if (m_data){
        ui->wdgGroup->fillFormFromData(m_data->toJson());
        ui->lblTitle->setText(m_data->getName());
    }
}
/*
void GroupWidget::updateParticipant(TeraData *participant)
{
    int id_participant = participant->getId();
    QTableWidgetItem* item;
    if (m_listParticipants_items.contains(id_participant)){
        item = m_listParticipants_items[id_participant];

    }else{
        ui->tableParticipants->setRowCount(ui->tableParticipants->rowCount()+1);
        item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_PARTICIPANT)),"");
        ui->tableParticipants->setItem(ui->tableParticipants->rowCount()-1, 0, item);
        m_listParticipants_items[id_participant] = item;

        QTableWidgetItem* item2 = new QTableWidgetItem("");
        item2->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->tableParticipants->setItem(ui->tableParticipants->rowCount()-1, 1, item2);
        item2 = new QTableWidgetItem("");
        item2->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->tableParticipants->setItem(ui->tableParticipants->rowCount()-1, 2, item2);
        item2 = new QTableWidgetItem("");
        item2->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->tableParticipants->setItem(ui->tableParticipants->rowCount()-1, 3, item2);
    }

    // Update values
    item->setText(participant->getName());
    if (participant->isEnabled()){
       ui->tableParticipants->item(item->row(), 1)->setText(tr("Actif"));
       ui->tableParticipants->item(item->row(), 1)->setForeground(Qt::green);
    }else{
       ui->tableParticipants->item(item->row(), 1)->setText(tr("Inactif"));
       ui->tableParticipants->item(item->row(), 1)->setForeground(Qt::red);
    }
    QString date_val_str = tr("Aucune connexion");
    if (participant->hasFieldName("participant_lastonline")){
        if (!participant->getFieldValue("participant_lastonline").isNull()){
            date_val_str = participant->getFieldValue("participant_lastonline").toDateTime().toLocalTime().toString("dd MMMM yyyy - hh:mm");
        }
    }
    ui->tableParticipants->item(item->row(), 2)->setText(date_val_str);
    date_val_str = tr("Aucune séance");
    if (participant->hasFieldName("participant_lastsession")){
        if (!participant->getFieldValue("participant_lastsession").isNull()){
            QDateTime date_val = participant->getFieldValue("participant_lastsession").toDateTime().toLocalTime();
            date_val_str = date_val.toString("dd MMMM yyyy - hh:mm");
            if (participant->isEnabled()){
                // Set background color for last session date
                QColor back_color = TeraForm::getGradientColor(0, 3, 7, static_cast<int>(date_val.daysTo(QDateTime::currentDateTime())));
                back_color.setAlphaF(0.5);
                ui->tableParticipants->item(item->row(), 3)->setBackground(back_color);
            }
        }
    }
    ui->tableParticipants->item(item->row(), 3)->setText(date_val_str);
}*/
/*
void GroupWidget::updateStats()
{


    ui->lblParticipant->setText(QString::number(ui->tableParticipants->rowCount()) + tr(" participant(s)"));


}*/

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

bool GroupWidget::validateData(){
    bool valid = false;

    valid = ui->wdgGroup->validateFormData();

    return valid;
}

void GroupWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type == WEB_FORMS_QUERY_GROUP){
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

        QVariantList parts_list = stats.getFieldValue("participants").toList();

        for(QVariant part:parts_list){
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
            }else{
                status = tr("Inactif");
                item->setForeground(Qt::red);
            }
            item->setText(status);
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableSummary->setItem(current_row, 1, item);

            item = new QTableWidgetItem(part_info["participant_sessions_count"].toString());
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableSummary->setItem(current_row, 2, item);

            item = new QTableWidgetItem(part_info["participant_first_session"].toDateTime().toLocalTime().toString("dd-MM-yyyy hh:mm:ss"));
            item->setTextAlignment(Qt::AlignCenter);
            ui->tableSummary->setItem(current_row, 3, item);

            QDateTime last_session_datetime = part_info["participant_last_session"].toDateTime().toLocalTime();
            item = new QTableWidgetItem(last_session_datetime.toString("dd-MM-yyyy hh:mm:ss"));
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
            item = new QTableWidgetItem(last_connect);
            item->setTextAlignment(Qt::AlignCenter);

            if (part_info["participant_enabled"].toBool() == true && last_connect_datetime.isValid()){
                // Set background color
                QColor back_color = TeraForm::getGradientColor(0, 5, 10, static_cast<int>(last_connect_datetime.daysTo(QDateTime::currentDateTime())));
                back_color.setAlphaF(0.5);
                item->setBackground(back_color);
            }
            ui->tableSummary->setItem(current_row, 5, item);
        }
    }

}

void GroupWidget::postResultReply(QString path)
{
    // OK, data was saved!
    if (path==WEB_GROUPINFO_PATH){
        if (parent())
            emit closeRequest();
    }
}
/*
void GroupWidget::processParticipants(QList<TeraData> participants)
{
    for (TeraData participant:participants){
        if (participant.getFieldValue("id_participant_group").toInt() == m_data->getId()){
            updateParticipant(&participant);
        }
    }

    //ui->tableParticipants->resizeColumnsToContents();
    updateStats();
}*/

void GroupWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &GroupWidget::processFormsReply);
    connect(m_comManager, &ComManager::postResultsOK, this, &GroupWidget::postResultReply);
    connect(m_comManager, &ComManager::statsReceived, this, &GroupWidget::processStatsReply);
}


