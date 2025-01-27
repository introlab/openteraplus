#include "SurveyServiceConfigWidget.h"
#include "ui_SurveyServiceConfigWidget.h"

#include "SurveyServiceWebAPI.h"
#include "GlobalMessageBox.h"

SurveyServiceConfigWidget::SurveyServiceConfigWidget(ComManager *comManager, int id_project, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SurveyServiceConfigWidget)
    , m_comManager(comManager)
    , m_id_project(id_project)
{

    m_surveyComManager = new SurveyComManager(comManager);

    ui->setupUi(this);

    ui->frameItemEditor->hide();
    ui->btnDelete->setEnabled(false);
    ui->wdgMessages->hide();

    connectSignals();
}

SurveyServiceConfigWidget::~SurveyServiceConfigWidget()
{
    delete ui;
    if (m_data){
        delete m_data;
    }
}

void SurveyServiceConfigWidget::refresh()
{
    if (!m_surveyComManager->isReady()){
        m_refreshRequested = true;
        return;
    }
    queryTestTypes();
}

void SurveyServiceConfigWidget::connectSignals()
{
    connect(m_comManager, &ComManager::testTypesReceived, this, &SurveyServiceConfigWidget::processTestTypesReply);
    connect(m_comManager, &ComManager::deleteResultsOK, this, &SurveyServiceConfigWidget::comManagerDeleteOK);

    connect(m_surveyComManager, &SurveyComManager::activeSurveyReceived, this, &SurveyServiceConfigWidget::processActiveSurveyReply);
    connect(m_surveyComManager, &SurveyComManager::networkError, this, &SurveyServiceConfigWidget::handleNetworkError);

    connect(ui->wdgMessages, &ResultMessageWidget::nextMessageShown, this, &SurveyServiceConfigWidget::nextMessageWasShown);
}

void SurveyServiceConfigWidget::queryTestTypes()
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_id_project));
    m_comManager->doGet(WEB_TESTTYPEINFO_PATH, args);

    ui->frameItemEditor->hide();
    m_listTestTypes_items.clear();
    m_listTestTypes_items_ids.clear();
    ui->lstData->clear();
}

void SurveyServiceConfigWidget::setEditMode(const bool &editing)
{
    ui->frameEditButton->setVisible(!editing && m_id_survey > 0);
    ui->frameSave->setVisible(editing);

    ui->btnEditSurvey->setVisible(m_id_survey > 0 && !editing);
    ui->btnAnswers->setVisible(m_id_survey > 0 && !editing);
    ui->frameEditor->setEnabled(editing);
    ui->frameItemEditor->setVisible(editing || m_id_survey > 0);
    ui->frameItems->setEnabled(!editing);
    ui->btnSave->setDisabled(ui->txtName->text().isEmpty());
    ui->txtName->setFocus();
}

void SurveyServiceConfigWidget::showSurveyEditor()
{
    /*if (m_surveyEditor){
        m_surveyEditor->deleteLater();
        m_surveyEditor = nullptr;
    }*/

    if (!m_data)
        return;

    if (m_data->isNew())
        return;

    if (!m_surveyEditor){
        m_surveyEditor = new SurveyEditorDialog(m_surveyComManager, m_data->getUuid(), this);
        connect(m_surveyEditor, &SurveyEditorDialog::finished, this, &SurveyServiceConfigWidget::surveyEditorFinished);
    }else{
        m_surveyEditor->setCurrentTestTypeUuid(m_data->getUuid());
    }
    m_surveyEditor->loadEditor();

    m_surveyEditor->showMaximized();

}

void SurveyServiceConfigWidget::showSurveyManager()
{
    if (!m_data)
        return;

    if (m_data->isNew())
        return;

    if (!m_surveyEditor){
        m_surveyEditor = new SurveyEditorDialog(m_surveyComManager, m_data->getUuid(), this);
        connect(m_surveyEditor, &SurveyEditorDialog::finished, this, &SurveyServiceConfigWidget::surveyEditorFinished);
    }else{
        m_surveyEditor->setCurrentTestTypeUuid(m_data->getUuid());
    }
    m_surveyEditor->loadManager(m_id_project);

    m_surveyEditor->showMaximized();
}

void SurveyServiceConfigWidget::processTestTypesReply(QList<TeraData> ttp_list, QUrlQuery reply_query)
{
    for(const TeraData &data:ttp_list){
        if (!data.hasFieldName("test_type_service_key"))
            continue;

        if (data.getFieldValue("test_type_service_key").toString() != "SurveyJSService")
            continue;

        QListWidgetItem* item = nullptr;
        int id_testtype = data.getId();
        QString testtype_uuid = data.getUuid();

        // Check if we already have that item
        if (m_listTestTypes_items.contains(testtype_uuid))
            item = m_listTestTypes_items[testtype_uuid];

        QString tt_name = data.getName();

        // If we don't, create a new one.
        if (!item){
            // Check if we have a new item that we could match
            item = new QListWidgetItem(tt_name, ui->lstData);
            item->setIcon(QIcon(TeraData::getIconFilenameForDataType(TERADATA_TESTTYPE)));
            m_listTestTypes_items[testtype_uuid] = item;
            m_listTestTypes_items_ids[id_testtype] = item;
            if (m_data){
                if (m_data->getUuid() == testtype_uuid){
                    *m_data = data;
                    // Make sure item is selected
                    item->setSelected(true);
                }
            }

        }else{
            // Update name if needed
            m_listTestTypes_items[testtype_uuid]->setText(tt_name);

            // Refresh infos if we are looking for it
            if (m_data){
                if (m_data->getUuid() == testtype_uuid){
                    *m_data = data;
                    ui->txtDescription->setPlainText(m_data->getFieldValue("test_type_description").toString());
                    ui->txtName->setText(m_data->getName());
                }
            }
        }
        /*if (m_data){
            if (m_data->getUuid() == testtype_uuid){
                // Make sure item is selected
                item->setSelected(true);

            }
        }*/
    }
}

void SurveyServiceConfigWidget::processActiveSurveyReply(const QJsonObject survey)
{
    QString test_type_uuid = survey.value("active_survey_test_type_uuid").toString();
    if (m_id_survey == 0){
        // New survey
        m_id_survey = survey.value("id_active_survey").toInt();
        if (m_data){
            delete m_data;
        }
        m_data = new TeraData(TeraDataTypes::TERADATA_TESTTYPE);
        m_data->setUuid(test_type_uuid);

    }else{
        // Updated survey

    }
    // Update related test type
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_UUID_TESTTYPE, test_type_uuid);
    m_comManager->doGet(WEB_TESTTYPEINFO_PATH, args);

    setEditMode(false);
}

void SurveyServiceConfigWidget::handleNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code)
{
    if (error == QNetworkReply::OperationCanceledError && op == QNetworkAccessManager::PostOperation){
        // Transfer was cancelled by user - no need to alert anyone!
        return;
    }

    if (error_msg.endsWith('\n'))
        error_msg = error_msg.left(error_msg.length()-1);


    QString error_str;

    if (status_code > 0)
        error_str = tr("Erreur HTTP ") + QString::number(status_code) + ": " + error_msg;
    else
        error_str = tr("Erreur ") + QString::number(error) + ": " + error_msg;

    error_str = error_str.replace('\n', " - ");
    error_str = error_str.replace('\r', "");
    ui->wdgMessages->addMessage(Message(Message::MESSAGE_ERROR, error_str));
}

void SurveyServiceConfigWidget::surveyComManagerReady(bool ready)
{
    if (ready){
        if (m_refreshRequested){
            m_refreshRequested = false;
            refresh();
        }
    }
}

void SurveyServiceConfigWidget::comManagerDeleteOK(QString path, int id)
{
    if (path == WEB_TESTTYPEINFO_PATH){
        // Remove test type info from list
        QListWidgetItem* item = m_listTestTypes_items_ids.value(id);
        if (!item){
            // Not in list, nothing to do!
            return;
        }

        if (m_data){
            if (m_data->getId() == id){
                delete m_data;
                m_data = nullptr;
                m_id_survey = 0;
            }
        }

        QString uuid = m_listTestTypes_items.key(item);
        m_listTestTypes_items.remove(uuid);
        m_listTestTypes_items_ids.remove(id);
        delete item;
    }
}

void SurveyServiceConfigWidget::nextMessageWasShown(Message current_message)
{
    if (current_message.getMessageType()==Message::MESSAGE_NONE){
        ui->wdgMessages->hide();
    }else{
        ui->wdgMessages->show();
    }
}

void SurveyServiceConfigWidget::surveyEditorFinished(int result)
{
    /*m_surveyEditor->deleteLater();
    m_surveyEditor = nullptr;*/
}

void SurveyServiceConfigWidget::on_btnNew_clicked()
{
    ui->txtName->clear();
    ui->txtDescription->clear();
    m_id_survey = 0;

    setEditMode(true);

}


void SurveyServiceConfigWidget::on_btnUndo_clicked()
{
    setEditMode(false);
    if (ui->lstData->currentItem()){
        on_lstData_currentItemChanged(ui->lstData->currentItem(), nullptr);
    }
}


void SurveyServiceConfigWidget::on_btnSave_clicked()
{
    // POST to service
    QJsonObject base_obj;
    QJsonObject data_obj;
    QJsonDocument doc;

    data_obj["id_active_survey"] = m_id_survey;
    data_obj["active_survey_name"] = ui->txtName->text();
    data_obj["active_survey_description"] = ui->txtDescription->toPlainText();

    base_obj["active_survey"] = data_obj;
    doc.setObject(base_obj);

    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_id_project));
    m_surveyComManager->doPostWithParams(SURVEY_ACTIVE_PATH, doc.toJson(), args);

}


void SurveyServiceConfigWidget::on_txtName_textChanged(const QString &new_text)
{
    ui->btnSave->setDisabled(new_text.isEmpty());
}


void SurveyServiceConfigWidget::on_lstData_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    ui->btnDelete->setEnabled(current);

    if (m_data){
        delete m_data;
        m_data = nullptr;
    }

    if (!current){
        ui->frameItemEditor->hide();
        return;
    }

    m_data = new TeraData(TeraDataTypes::TERADATA_TESTTYPE);
    m_data->setUuid(m_listTestTypes_items.key(current));

    QUrlQuery args;
    args.addQueryItem(SURVEY_QUERY_ACTIVE_TEST_TYPE_UUID, m_data->getUuid());
    m_surveyComManager->doGet(SURVEY_ACTIVE_PATH, args);

}


void SurveyServiceConfigWidget::on_btnEditSurvey_clicked()
{
    showSurveyEditor();
}

void SurveyServiceConfigWidget::on_btnEdit_clicked()
{
    setEditMode(true);
}


void SurveyServiceConfigWidget::on_btnDelete_clicked()
{
    if (!ui->lstData->currentItem() || !m_data)
        return;

    GlobalMessageBox msg;
    GlobalMessageBox::StandardButton result;
    result = msg.showYesNo(tr("Supprimer") + "?", tr("Voulez-vous supprimer le questionnaire") + " '" + ui->lstData->currentItem()->text() + "' ?");
    if (result == GlobalMessageBox::Yes){
        m_comManager->doDelete(WEB_TESTTYPEINFO_PATH, m_data->getId());
    }
}


void SurveyServiceConfigWidget::on_btnAnswers_clicked()
{
    showSurveyManager();
}

