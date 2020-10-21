#include "EmailInviteDialog.h"
#include "ui_EmailInviteDialog.h"

EmailInviteDialog::EmailInviteDialog(ComManager* comMan, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EmailInviteDialog),
    m_comManager(comMan)
{
    ui->setupUi(this);
}

EmailInviteDialog::~EmailInviteDialog()
{
    delete ui;
}

void EmailInviteDialog::setFieldValues(const QHash<QString, QString> &fields)
{
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
