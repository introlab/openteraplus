#include "SurveyServiceConfigWidget.h"
#include "ui_SurveyServiceConfigWidget.h"

#include "SurveyServiceWebAPI.h"

SurveyServiceConfigWidget::SurveyServiceConfigWidget(ComManager *comManager, int id_project, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SurveyServiceConfigWidget)
    , m_comManager(comManager)
    , m_id_project(id_project)
{

    m_surveyComManager = new SurveyComManager(comManager);

    ui->setupUi(this);

    ui->frameEditor->hide();
    ui->btnDelete->setEnabled(false);
    ui->wdgMessages->hide();

    connectSignals();
}

SurveyServiceConfigWidget::~SurveyServiceConfigWidget()
{
    delete ui;
}

void SurveyServiceConfigWidget::refresh()
{
    queryTestTypes();
}

void SurveyServiceConfigWidget::connectSignals()
{
    connect(m_comManager, &ComManager::testTypesReceived, this, &SurveyServiceConfigWidget::processTestTypesReply);

    connect(m_surveyComManager, &SurveyComManager::activeSurveyReceived, this, &SurveyServiceConfigWidget::processActiveSurveyReply);
    connect(m_surveyComManager, &SurveyComManager::networkError, this, &SurveyServiceConfigWidget::handleNetworkError);

    connect(ui->wdgMessages, &ResultMessageWidget::nextMessageShown, this, &SurveyServiceConfigWidget::nextMessageWasShown);
}

void SurveyServiceConfigWidget::queryTestTypes()
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_id_project));
    m_comManager->doGet(WEB_TESTTYPEINFO_PATH, args);

    m_listTestTypes_items.clear();
    ui->lstData->clear();
}

void SurveyServiceConfigWidget::setEditMode(const bool &editing)
{
    ui->frameEditButton->setVisible(!editing && m_current_id > 0);
    ui->frameSave->setVisible(editing);

    ui->btnEditSurvey->setVisible(m_current_id > 0);
    ui->frameEditor->setEnabled(editing);
    ui->frameEditor->setVisible(editing || m_current_id > 0);
    ui->frameItems->setEnabled(!editing);
    ui->btnSave->setDisabled(ui->txtName->text().isEmpty());
    ui->txtName->setFocus();
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

        // Check if we already have that item
        if (m_listTestTypes_items.contains(id_testtype))
            item = m_listTestTypes_items[id_testtype];

        QString tt_name = data.getName();

        // If we don't, create a new one.
        if (!item){
            // Check if we have a new item that we could match
            item = new QListWidgetItem(tt_name, ui->lstData);
            item->setIcon(QIcon(TeraData::getIconFilenameForDataType(TERADATA_TESTTYPE)));
            m_listTestTypes_items[id_testtype] = item;

        }else{
            // Update name if needed
            m_listTestTypes_items[id_testtype]->setText(tt_name);
        }
    }
}

void SurveyServiceConfigWidget::processActiveSurveyReply(const QJsonObject survey)
{
    if (m_current_id == 0){
        // New servey
        m_current_id = survey.value("id_active_survey").toInt();
    }else{
        // Updated survey

    }
    // Update related test type
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_UUID, survey.value("active_survey_test_type_uuid").toString());
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

void SurveyServiceConfigWidget::nextMessageWasShown(Message current_message)
{
    if (current_message.getMessageType()==Message::MESSAGE_NONE){
        ui->wdgMessages->hide();
    }else{
        ui->wdgMessages->show();
    }
}

void SurveyServiceConfigWidget::on_btnNew_clicked()
{
    ui->txtName->clear();
    ui->txtDescription->clear();
    m_current_id = 0;

    setEditMode(true);

}


void SurveyServiceConfigWidget::on_btnUndo_clicked()
{
    setEditMode(false);
}


void SurveyServiceConfigWidget::on_btnSave_clicked()
{
    // POST to service
    QJsonObject base_obj;
    QJsonObject data_obj;
    QJsonDocument doc;

    data_obj["id_active_survey"] = m_current_id;
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

