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

    m_timer.setInterval(1000);

    // Signals
    connect(m_comManager, &ComManager::sessionStarted, this, &StartSessionDialog::accepted);
    connect(m_comManager, &ComManager::sessionStopped, this, &StartSessionDialog::accepted);
    connect(m_comManager, &ComManager::sessionError, this, &StartSessionDialog::rejected);
    connect(m_comManager, &ComManager::networkError, this, &StartSessionDialog::rejected);
    connect(&m_timer, &QTimer::timeout, this, &StartSessionDialog::timerTimeOut);

    m_timer.start();

}

StartSessionDialog::~StartSessionDialog()
{
    m_loadingIcon->deleteLater();
    delete ui;
}

void StartSessionDialog::closeRequest()
{
    accept();
}

void StartSessionDialog::on_btnCancel_clicked()
{

}

void StartSessionDialog::timerTimeOut()
{

    m_count++;
    if (m_count>m_timeout){
        emit timeout();
        reject();
    }
    ui->lblTimeout->setText(QString::number(m_timeout - m_count));
}
