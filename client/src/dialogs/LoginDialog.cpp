#include "LoginDialog.h"
#include "ui_LoginDialog.h"
#include <QStyledItemDelegate>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    ui->cmbServers->setItemDelegate(new QStyledItemDelegate(ui->cmbServers));

    // Hide logos for now
    ui->frameLogos->hide();

    //setAttribute(Qt::WA_StyledBackground);
    //setStyleSheet("QDialog{background-image: url(://TeRA_Background.png); }");

    // Setup properties
    setSizeGripEnabled(false);
    setFixedWidth(450);
    setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    // Set version number
    setWindowTitle("OpenTeraPlus - Version " + QString(OPENTERAPLUS_VERSION));

    // Setup loading icon animation
    m_animatedIcon = new QMovie("://status/loading.gif");
    ui->lblLoadingIcon->setMovie(m_animatedIcon);
    m_animatedIcon->start();

    // Hide messages
    ui->frameMessage->hide();
    ui->lblWarningIcon->hide();

    ui->txtUsername->setFocus();
}

LoginDialog::~LoginDialog()
{
    m_animatedIcon->deleteLater();
    delete ui;
}

void LoginDialog::setServerNames(QStringList servers)
{
    ui->cmbServers->clear();
    foreach(QString server, servers){
        ui->cmbServers->addItem(server);
    }
    if(servers.count()>0)
        ui->cmbServers->setCurrentIndex(0);

    // Select server from the list if we have a setting for that
    QVariant current_server = TeraSettings::getGlobalSetting("last_used_server");
    if (current_server.isValid()){
        QString server_name = current_server.toString();
        if (servers.contains(server_name))
            ui->cmbServers->setCurrentText(server_name);
    }

    /*if(servers.count() == 1){
        showServers(false); // No need to show server box if only one server in the list!
    }*/
}

void LoginDialog::showServers(bool show)
{
    if (ui->cmbServers->count() == 1)
        show = false; // Never show server list if only one item.

    ui->lblServer->setVisible(show);
    ui->cmbServers->setVisible(show);
    ui->icoServer->setVisible(show);
}

QString LoginDialog::currentServerName()
{
    if (ui->cmbServers->currentIndex() >=0 && ui->cmbServers->isVisible()){
        return ui->cmbServers->currentText();
    }
    return QString();
}

void LoginDialog::setStatusMessage(QString message, bool error)
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

void LoginDialog::on_btnQuit_clicked()
{
    emit quitRequest();
}

void LoginDialog::on_btnConnect_clicked()
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

    emit loginRequest(ui->txtUsername->text(), ui->txtPassword->text(), ui->cmbServers->currentText());

    TeraSettings::setGlobalSetting("last_used_server", ui->cmbServers->currentText());

}
