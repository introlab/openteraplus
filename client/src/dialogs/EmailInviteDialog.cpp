#include "EmailInviteDialog.h"
#include "ui_EmailInviteDialog.h"

#include "services/EmailService/EmailServiceWebAPI.h"

#include "GlobalMessageBox.h"

EmailInviteDialog::EmailInviteDialog(ComManager* comMan, TeraData *participant, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EmailInviteDialog),
    m_comManager(comMan),
    m_participant(participant)
{
    ui->setupUi(this);
    ui->btnSendLocalEmail->hide();

    m_emailComManager = new EmailComManager(m_comManager);

    if (m_participant->hasFieldName("participant_email")){
        QString email = m_participant->getFieldValue("participant_email").toString();
        if (!email.isEmpty()){
            ui->lblEmail->setText(m_participant->getName() + "<" + m_participant->getFieldValue("participant_email").toString() + ">");
        }else{
            ui->btnSendLocalEmail->setDisabled(true);
            ui->btnSend->setDisabled(true);
        }
    }else{
        ui->btnSendLocalEmail->setDisabled(true);
        ui->btnSend->setDisabled(true);
    }

    QString user_email = m_comManager->getCurrentUser().getFieldValue("user_email").toString();
    ui->lblError->setVisible(user_email.isEmpty());
    if (ui->btnSend->isEnabled())
        ui->btnSend->setDisabled(user_email.isEmpty());

    connectSignals();
    initLanguages();
}

EmailInviteDialog::~EmailInviteDialog()
{
    delete ui;
    m_emailComManager->deleteLater();
}

void EmailInviteDialog::setFieldValues(const QHash<QString, QString> &fields)
{
    QStringList variables = fields.keys();
    for (const QString& variable: variables){
        ui->wdgEmail->setVariable(variable, fields[variable]);
    }

    ui->wdgEmail->setVariable("$fullname", m_comManager->getCurrentUser().getName());
    ui->wdgEmail->setVariable("$participant", m_participant->getName());
}

void EmailInviteDialog::onEmailComReady(bool ready)
{
    if (ready)
        queryTemplate();
}

void EmailInviteDialog::on_btnOk_clicked()
{
    accept();
}

void EmailInviteDialog::on_btnCopy_clicked()
{
    QTextEdit* preview = ui->wdgEmail->getPreview();
    preview->selectAll();
    preview->copy();
    QTextCursor cursor = preview->textCursor();
    cursor.movePosition( QTextCursor::End );
    preview->setTextCursor( cursor );

    #if defined(Q_OS_LINUX)
        QThread::msleep(1); //workaround for copied text not being available...
    #endif
}

void EmailInviteDialog::on_btnSendLocalEmail_clicked()
{
    /*bool ok_to_send = false;
    QString email;
    if (m_participant->hasFieldName("participant_email")){
        email = m_participant->getFieldValue("participant_email").toString();
        if (!email.isEmpty())
            ok_to_send = true;

    }

    if (!ok_to_send){
        GlobalMessageBox msg;
        msg.showError(tr("Impossible d'envoyer"), tr("Impossible d'envoyer le courriel: aucune adresse courriel spéficiée pour le participant!"));
        return;
    }

    // "mailto" cannot send html formatted text...
    QString body = ui->textEmail->toPlainText();
    body = body.replace("lien.", "lien: " + m_fields["url"]); // TODO: improve by using email templates

    QString mailto = "mailto:" + email + "?subject="
            + tr("Invitation - Séances via OpenTeraPlus") + "&body=" + body;

    QDesktopServices::openUrl(QUrl(mailto));*/

}

void EmailInviteDialog::processTemplateReply(const QJsonObject email_template)
{
    if (!email_template.empty()){
        QString template_html = email_template["email_template"].toString();
        ui->wdgEmail->setEmailTemplate(template_html);
    }
}

void EmailInviteDialog::handleNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code)
{
    GlobalMessageBox msg;
    msg.showError(tr("Envoi courriel invitation"), tr("Le courriel d'invitation n'a pas pu être envoyé.") + "\n\n" + error_msg);
    setDisabled(false);
}

void EmailInviteDialog::emailSentSuccess()
{
    GlobalMessageBox msg;
    msg.showInfo(tr("Envoi courriel invitation"), tr("Le courriel d'invitation a été envoyé avec succès!"));
    setDisabled(false);
    accept();
}

void EmailInviteDialog::initLanguages()
{
    ui->cmbLanguage->clear();
    ui->cmbLanguage->addItem(tr("Français"), "fr");
    ui->cmbLanguage->addItem(tr("Anglais"), "en");
    ui->cmbLanguage->addItem(tr("Español"), "es");

    // Select language
    QString lang = m_comManager->getCurrentPreferences().getLanguage();
    for (int i=0; i<ui->cmbLanguage->count(); i++){
        if (ui->cmbLanguage->itemData(i) == lang){
            ui->cmbLanguage->setCurrentIndex(i);
            break;
        }
    }
}

void EmailInviteDialog::queryTemplate()
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_KEY, KEY_EMAIL_INVITE);
    args.addQueryItem(WEB_QUERY_EMAIL_LANG, ui->cmbLanguage->currentData().toString());
    args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_participant->getFieldValue("id_project").toInt()));
    m_emailComManager->doGet(EMAIL_TEMPLATE_PATH, args);
}

void EmailInviteDialog::on_cmbLanguage_currentIndexChanged(int index)
{
    if (m_emailComManager->isReady())
        queryTemplate();
    if (ui->cmbLanguage->currentData().toString() == "fr"){
        ui->txtSubject->setText(tr("Invitation - Séances via OpenTera"));
    }
    if (ui->cmbLanguage->currentData().toString() == "en"){
        ui->txtSubject->setText(tr("Invitation - Sessions using OpenTera"));
    }
}

void EmailInviteDialog::connectSignals()
{
    connect(m_emailComManager, &EmailComManager::readyChanged, this, &EmailInviteDialog::onEmailComReady);
    connect(m_emailComManager, &EmailComManager::emailTemplateReceived, this, &EmailInviteDialog::processTemplateReply);
    connect(m_emailComManager, &EmailComManager::postResultsOK, this, &EmailInviteDialog::emailSentSuccess);
    connect(m_emailComManager, &EmailComManager::networkError, this, &EmailInviteDialog::handleNetworkError);
}


void EmailInviteDialog::on_btnSend_clicked()
{
    ui->wdgEmail->setEditing(false);

    QJsonDocument doc;
    QJsonObject data_obj;
    data_obj["participant_uuid"] = m_participant->getUuid();
    data_obj["body"] = ui->wdgEmail->getPreview()->toHtml();
    data_obj["subject"] = ui->txtSubject->text();
    doc.setObject(data_obj);
    m_emailComManager->doPost(EMAIL_SEND_PATH, doc.toJson());

    setDisabled(true);
}

