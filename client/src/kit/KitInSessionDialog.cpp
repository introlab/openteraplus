#include "KitInSessionDialog.h"
#include "ui_KitInSessionDialog.h"

KitInSessionDialog::KitInSessionDialog(KitConfigManager *kitConfig, ParticipantComManager *partCom, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KitInSessionDialog),
    m_kitConfig(kitConfig),
    m_partComManager(partCom)
{
    ui->setupUi(this);
    m_serviceWidget = nullptr;
    m_otherSoftProcess = nullptr;

    initUi();
}

KitInSessionDialog::~KitInSessionDialog()
{
    delete ui;
    if (m_serviceWidget)
        m_serviceWidget->deleteLater();

    if (m_otherSoftProcess){
        stopOtherSoftware();
    }
}

void KitInSessionDialog::on_btnLogout_clicked()
{
    accept();
}

void KitInSessionDialog::initUi()
{
    QString service_key = m_kitConfig->getServiceKey();

    if (service_key == "VideoRehabService"){
        m_serviceWidget = new KitVideoRehabWidget(m_kitConfig, m_partComManager, this);
        setMainWidget(m_serviceWidget);
    }else{
        GlobalMessageBox msg_box;
        msg_box.showWarning(tr("Service non-supporté"), tr("Le service \"") + service_key + tr("\" n'est pas gérée par cette version du logiciel.\n\nVeuillez vérifier si une mise à jour existe ou contribuez au développement du logiciel!"));
    }

    ui->btnLauncher->setVisible(!m_kitConfig->getOtherSoftwarePath().isEmpty());
}

void KitInSessionDialog::setMainWidget(QWidget *wdg)
{
    // Check for layout
    if (!ui->widgetMain->layout()){
        // No existing layout - create one
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        ui->widgetMain->setLayout(layout);
    }

    ui->widgetMain->layout()->addWidget(wdg);

}

void KitInSessionDialog::startOtherSoftware()
{
    if (m_otherSoftProcess){
        stopOtherSoftware();
    }
    m_otherSoftProcess = new QProcess(this);
    QString software = m_kitConfig->getOtherSoftwarePath();
    m_otherSoftProcess->setProgram(software);
    QFileInfo fi(software);
    m_otherSoftProcess->setWorkingDirectory(fi.canonicalPath());
    m_otherSoftProcess->start();
}

void KitInSessionDialog::stopOtherSoftware()
{
    m_otherSoftProcess->terminate();
    m_otherSoftProcess->kill();
    m_otherSoftProcess->waitForFinished(10000);
    m_otherSoftProcess->deleteLater();
    m_otherSoftProcess = nullptr;
}

void KitInSessionDialog::on_btnLauncher_clicked()
{
    if (!m_kitConfig->getOtherSoftwarePath().isEmpty()){
        startOtherSoftware();
    }
}

