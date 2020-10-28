#include "EmailInviteDialog.h"
#include "ui_EmailInviteDialog.h"

EmailInviteDialog::EmailInviteDialog(ComManager* comMan, TeraData *participant, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EmailInviteDialog),
    m_comManager(comMan),
    m_participant(participant)
{
    ui->setupUi(this);
    if (m_participant->hasFieldName("participant_email")){
        QString email = m_participant->getFieldValue("participant_email").toString();
        if (!email.isEmpty()){
            ui->lblEmail->setText(m_participant->getFieldValue("participant_email").toString());
        }else{
            ui->btnSendLocalEmail->setDisabled(true);
        }
    }else{
        ui->btnSendLocalEmail->setDisabled(true);
    }
}

EmailInviteDialog::~EmailInviteDialog()
{
    delete ui;
}

void EmailInviteDialog::setFieldValues(const QHash<QString, QString> &fields)
{
    m_fields = fields;
    for (int i=0; i<fields.count(); i++){
        if (fields.keys().at(i) == "url"){
            ui->textEmail->setText(ui->textEmail->toHtml().replace("{link}", "<a href=" + fields["url"] + ">" + tr("lien") + "</a>"));
        }
    }

    ui->textEmail->setText(ui->textEmail->toHtml().replace("{username}", m_comManager->getCurrentUser().getName()));
}

void EmailInviteDialog::on_btnOk_clicked()
{
    accept();
}

void EmailInviteDialog::on_btnCopy_clicked()
{
    ui->textEmail->selectAll();
    ui->textEmail->copy();
    QTextCursor cursor = ui->textEmail->textCursor();
    cursor.movePosition( QTextCursor::End );
    ui->textEmail->setTextCursor( cursor );

    #if defined(Q_OS_LINUX)
        QThread::msleep(1); //workaround for copied text not being available...
    #endif
}

void EmailInviteDialog::on_btnSendLocalEmail_clicked()
{
    bool ok_to_send = false;
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

    QDesktopServices::openUrl(QUrl(mailto));

}
