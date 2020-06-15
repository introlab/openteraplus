#include "StartSessionDialog.h"
#include "ui_StartSessionDialog.h"

StartSessionDialog::StartSessionDialog(QString title, ComManager *comManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StartSessionDialog),
    m_comManager(comManager)
{
    ui->setupUi(this);

    // Setup properties
    setSizeGripEnabled(false);
    setFixedWidth(450);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);

    // Setup UI
    m_loadingIcon = new QMovie("://status/session_loading.gif");
    ui->icoWorking->setMovie(m_loadingIcon);
    m_loadingIcon->start();

    ui->lblTitle->setText(title);
    ui->btnCancel->hide(); // For now!

    // Signals
    connect(m_comManager, &ComManager::sessionStarted, this, &StartSessionDialog::closeRequest);
    connect(m_comManager, &ComManager::sessionStopped, this, &StartSessionDialog::closeRequest);
    connect(m_comManager, &ComManager::networkError, this, &StartSessionDialog::closeRequest);

}

StartSessionDialog::~StartSessionDialog()
{
    m_loadingIcon->deleteLater();
    delete ui;
}

void StartSessionDialog::closeRequest()
{
    close();
}

void StartSessionDialog::on_btnCancel_clicked()
{

}
