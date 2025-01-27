#include "StandardLoginWidget.h"
#include "ui_StandardLoginWidget.h"

StandardLoginWidget::StandardLoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StandardLoginWidget)
{
    ui->setupUi(this);
    ui->btnQuit->hide();

    // Setup loading icon animation
    m_animatedIcon = new QMovie("://status/loading.gif");
    ui->lblLoadingIcon->setMovie(m_animatedIcon);
    m_animatedIcon->start();

    // Hide messages
    ui->frameMessage->hide();
    ui->lblWarningIcon->hide();

    ui->txtUsername->setFocus();
}

StandardLoginWidget::~StandardLoginWidget()
{
    delete ui;
    delete m_animatedIcon;
}

void StandardLoginWidget::setStatusMessage(const QString &message, const bool &error)
{
    if (message.isEmpty()){
        ui->frameMessage->hide();
    }else{
        QString filtered_msg = message; //QTextDocumentFragment::fromHtml(message).toPlainText();
        filtered_msg = filtered_msg.replace("\n"," - ");
        if (error)
            ui->lblMessage->setStyleSheet("color:red;");
        else {
            ui->lblMessage->setStyleSheet("color:yellow;");
        }
        ui->lblMessage->setText(filtered_msg);
        ui->lblWarningIcon->setVisible(error);
        ui->lblLoadingIcon->setVisible(!error);
        ui->frameMessage->show();
    }
    ui->frameLogin->setEnabled(error);
    ui->btnConnect->setEnabled(error);
}

void StandardLoginWidget::setFocus()
{
    ui->txtUsername->setFocus();
}

void StandardLoginWidget::on_btnConnect_clicked()
{
    if (ui->txtUsername->text().isEmpty()){
        setStatusMessage(tr("Code utilisateur invalide"),true);
        return;
    }

    if (ui->txtPassword->text().isEmpty()){
        setStatusMessage(tr("Mot de passe invalide"), true);
        return;
    }

    setStatusMessage(tr("Connexion en cours..."));

    emit standardLoginRequest(ui->txtUsername->text(), ui->txtPassword->text());

}

void StandardLoginWidget::on_btnQuit_clicked()
{
    emit quitRequest();
}

