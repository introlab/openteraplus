#include "SessionLobbyDialog.h"
#include "ui_SessionLobbyDialog.h"

SessionLobbyDialog::SessionLobbyDialog(ComManager* comManager, TeraData &session_type, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SessionLobbyDialog),
    m_comManager(comManager),
    m_sessionType(session_type)
{
    ui->setupUi(this);

    ui->lblTitle->setText(m_sessionType.getFieldValue("session_type_name").toString());

    configureWidget();
}

SessionLobbyDialog::~SessionLobbyDialog()
{
    delete ui;
}

void SessionLobbyDialog::setSetupWidget(QWidget *wdg)
{

    // Check for layout
    if (!ui->wdgSessionConfig->layout()){
        // No existing layout - create one
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        ui->wdgSessionConfig->setLayout(layout);
    }

    ui->wdgSessionConfig->layout()->addWidget(wdg);


}

void SessionLobbyDialog::configureWidget()
{
    // Get session type category
    TeraSessionCategory::SessionTypeCategories category = getSessionTypeCategory();

    switch(category){
    case TeraSessionCategory::SESSION_TYPE_SERVICE:{
        // Get service key to load the proper ui
        QString service_key = m_sessionType.getFieldValue("session_type_service_key").toString();
        bool handled = false;
        if (service_key == "VideoRehabService"){
            setSetupWidget(new VideoRehabSetupWidget(m_comManager));
            handled = true;
        }

        if (!handled){
            ui->wdgSessionConfig->hide();
        }

        break;
    }
    default:
       ui->wdgSessionConfig->hide();
    }
}

TeraSessionCategory::SessionTypeCategories SessionLobbyDialog::getSessionTypeCategory()
{
    return static_cast<TeraSessionCategory::SessionTypeCategories>(m_sessionType.getFieldValue("session_type_category").toInt());
}

void SessionLobbyDialog::on_btnCancel_clicked()
{
    reject();
}

void SessionLobbyDialog::on_btnStartSession_clicked()
{
    accept();
}
