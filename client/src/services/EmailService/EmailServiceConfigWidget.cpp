#include "EmailServiceConfigWidget.h"
#include "ui_EmailServiceConfigWidget.h"
#include "EmailServiceWebAPI.h"

#include <QStyledItemDelegate>

EmailServiceConfigWidget::EmailServiceConfigWidget(ComManager *comManager, const int &id_site, const int &id_project, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::EmailServiceConfigWidget),
    m_idSite(id_site),
    m_idProject(id_project)
{
    m_comManager = comManager;
    m_emailComManager = new EmailComManager(comManager);

    ui->setupUi(this);
    ui->cmbTemplate->setItemDelegate(new QStyledItemDelegate(ui->cmbTemplate));
    ui->btnBold->hide();
    ui->btnItalic->hide();
    ui->btnColor->hide();
    ui->btnUnderline->hide();
    ui->wdgMessages->hide();

    initVariablesMenu();
    initTemplates();
    connectSignals();

    // Only one template for now... query it!
    if (ui->cmbTemplate->count() > 0){
        //queryTemplate(ui->cmbTemplate->currentData().toString());
    }


}

EmailServiceConfigWidget::~EmailServiceConfigWidget()
{
    delete ui;
    delete m_emailComManager;
}

void EmailServiceConfigWidget::setSiteId(const int &id_site)
{
    m_idSite = id_site;
}

void EmailServiceConfigWidget::setProjectId(const int &id_project)
{
    m_idProject = id_project;
}

void EmailServiceConfigWidget::insertCurrentVariable()
{
    QObject* sender = QObject::sender();
    if (!sender)
        return;

    QAction* action = dynamic_cast<QAction*>(sender);
    if (action){
        ui->txtTemplate->insertHtml(action->data().toString());
    }
}

void EmailServiceConfigWidget::processTemplateReply(const QJsonObject email_template)
{
    if (!email_template.empty()){
        QString template_html = email_template["email_template"].toString();
        ui->txtTemplate->setHtml(template_html);
        ui->txtTemplate->setProperty("original", ui->txtTemplate->toHtml());
        ui->frameEditor->show();
        ui->frameSave->show();
        ui->frameSave->setEnabled(false);
    }
    m_currentTemplate = email_template;
}

void EmailServiceConfigWidget::handleNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code)
{
    Q_UNUSED(error)

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

void EmailServiceConfigWidget::emailComPostOK(QString path, QString data)
{
    ui->wdgMessages->addMessage(Message(Message::MESSAGE_OK, tr("Données sauvegardées")));

    QJsonParseError json_error;

    // Process reply
    QJsonDocument data_list = QJsonDocument::fromJson(data.toUtf8(), &json_error);
    if (json_error.error!= QJsonParseError::NoError){
        ui->wdgMessages->addMessage(Message(Message::MESSAGE_ERROR, tr("Impossible de lire la réponse.")));
        return;
    }

    processTemplateReply(data_list.object());

}

void EmailServiceConfigWidget::nextMessageWasShown(Message current_message)
{
    if (current_message.getMessageType()==Message::MESSAGE_NONE){
        ui->wdgMessages->hide();
    }else{
        ui->wdgMessages->show();
    }
}

void EmailServiceConfigWidget::connectSignals()
{
    connect(m_emailComManager, &EmailComManager::emailTemplateReceived, this, &EmailServiceConfigWidget::processTemplateReply);
    connect(m_emailComManager, &EmailComManager::postResultsOK, this, &EmailServiceConfigWidget::emailComPostOK);
    connect(m_emailComManager, &EmailComManager::networkError, this, &EmailServiceConfigWidget::handleNetworkError);

    connect(ui->wdgMessages, &ResultMessageWidget::nextMessageShown, this, &EmailServiceConfigWidget::nextMessageWasShown);

}

void EmailServiceConfigWidget::initVariablesMenu()
{
    if (!m_variablesMenu){
        m_variablesMenu = new QMenu(this);

        QAction* new_action = new QAction(tr("Nom participant"));
        new_action->setData("$participant");
        connect(new_action, &QAction::triggered, this, &EmailServiceConfigWidget::insertCurrentVariable);
        m_variablesMenu->addAction(new_action);

        new_action = new QAction(tr("Nom utilisateur"));
        new_action->setData("$fullname");
        connect(new_action, &QAction::triggered, this, &EmailServiceConfigWidget::insertCurrentVariable);
        m_variablesMenu->addAction(new_action);

        new_action = new QAction(tr("Lien de connexion"));
        new_action->setData("$join_link");
        connect(new_action, &QAction::triggered, this, &EmailServiceConfigWidget::insertCurrentVariable);
        m_variablesMenu->addAction(new_action);

        ui->btnVariables->setMenu(m_variablesMenu);
    }
}

void EmailServiceConfigWidget::initTemplates()
{
    ui->cmbTemplate->clear();
    ui->cmbTemplate->addItem(tr("Courriel d'invitation"), "INVITE_EMAIL");

    ui->cmbLanguage->clear();
    ui->cmbLanguage->addItem(tr("Français"), "fr");
    ui->cmbLanguage->addItem(tr("Anglais"), "en");

    // Select language
    QString lang = m_comManager->getCurrentPreferences().getLanguage();
    for (int i=0; i<ui->cmbLanguage->count(); i++){
        if (ui->cmbLanguage->itemData(i) == lang){
            ui->cmbLanguage->setCurrentIndex(i);
            break;
        }
    }

}

void EmailServiceConfigWidget::queryTemplate(const QString &key)
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_KEY, key);
    args.addQueryItem(WEB_QUERY_EMAIL_LANG, ui->cmbLanguage->currentData().toString());
    if (m_idSite > 0)
        args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_idSite));
    if (m_idProject > 0)
        args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_idProject));

    ui->frameEditor->hide();
    ui->frameSave->hide();
    m_emailComManager->doGet(EMAIL_TEMPLATE_PATH, args);
}


void EmailServiceConfigWidget::on_txtTemplate_textChanged()
{
    bool has_changes = ui->txtTemplate->toHtml() != ui->txtTemplate->property("original").toString();
    ui->frameSave->setEnabled(has_changes);
}


void EmailServiceConfigWidget::on_btnSave_clicked()
{
    if (m_currentTemplate.value("inherited").toBool()){
        m_currentTemplate["id_email_template"] = 0; // Inherited template, must create a new one!
    }

    if (m_idSite > 0)
        m_currentTemplate["id_site"] = m_idSite;
    else
        m_currentTemplate["id_site"] = QJsonValue::Null;

    if (m_idProject > 0)
        m_currentTemplate["id_project"] = m_idProject;
    else
        m_currentTemplate["id_project"] = QJsonValue::Null;

    m_currentTemplate["email_template"] = ui->txtTemplate->toHtml();

    QJsonObject base_obj;
    QJsonDocument doc;
    base_obj.insert("email_template", m_currentTemplate);
    doc.setObject(base_obj);
    m_emailComManager->doPost(EMAIL_TEMPLATE_PATH, doc.toJson());

}


void EmailServiceConfigWidget::on_btnUndo_clicked()
{
    ui->txtTemplate->setHtml(ui->txtTemplate->property("original").toString());
    ui->txtTemplate->setProperty("original", ui->txtTemplate->toHtml());
}


void EmailServiceConfigWidget::on_cmbLanguage_currentIndexChanged(int index)
{
    queryTemplate(ui->cmbTemplate->currentData().toString());
}

