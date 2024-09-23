#include "WebLoginDialog.h"
#include "ui_WebLoginDialog.h"
#include <QVBoxLayout>


WebLoginDialog::WebLoginDialog(ConfigManagerClient *config, QWidget *parent)
    : QDialog(parent), ui(new Ui::WebLoginDialog), m_config(config)
{
    ui->setupUi(this);

    //Create Web View
    m_webView = new QWebEngineView(ui->centralWidget);

    QVBoxLayout *layout = new QVBoxLayout(ui->centralWidget);
    layout->addWidget(m_webView);

    m_webPage = new QWebEnginePage(m_webView);

    QWebChannel *channel = new QWebChannel(m_webPage);

    WebLoginSharedObject *myObject = new WebLoginSharedObject();
    channel->registerObject("qtObject", myObject);
    m_webPage->setWebChannel(channel);

    connect(m_webPage, &QWebEnginePage::certificateError, this, &WebLoginDialog::onCertificateError);
    connect(myObject, &WebLoginSharedObject::loginSuccess, this, &WebLoginDialog::loginSuccess);
    connect(myObject, &WebLoginSharedObject::loginFailure, this, &WebLoginDialog::loginFailure);

    m_webView->setPage(m_webPage);   
}

WebLoginDialog::~WebLoginDialog()
{
    delete ui;
}

void WebLoginDialog::setServerNames(QStringList servers)
{
    ui->cmbServers->clear();
    foreach(QString server, servers){
        ui->cmbServers->addItem(server);
    }

    // Connecting signal after will avoid calling onServerSelected when adding initial servers
    connect(ui->cmbServers, &QComboBox::currentIndexChanged, this, &WebLoginDialog::onServerSelected);

    // Get last server used
    QVariant current_server = TeraSettings::getGlobalSetting("last_used_server");

    // Select server from the list if we have a setting for that
    if (current_server.isValid()){
        QString server_name = current_server.toString();
        if (servers.contains(server_name))
            ui->cmbServers->setCurrentText(server_name);
    }
}


void WebLoginDialog::showServers(bool show)
{
    if (ui->cmbServers->count() == 1)
        show = false; // Never show server list if only one item.

    ui->cmbServers->setVisible(show);
}


void WebLoginDialog::onCertificateError(const QWebEngineCertificateError &certificateError)
{
    //TODO do Something about certificates
    qDebug() << "Certificate error: " << certificateError.description();
    //TODO Do not accept certificates in production ?
    auto mutableError = const_cast<QWebEngineCertificateError&>(certificateError);
    mutableError.acceptCertificate();
}

void WebLoginDialog::onServerSelected(int index)
{
    QString currentServer = ui->cmbServers->itemText(index);
    // Update last server
    TeraSettings::setGlobalSetting("last_used_server", currentServer);

    //Get URL
    if (m_config && m_webPage)
    {
        QUrl loginUrl = m_config->getServerLoginUrl(currentServer);
        m_webPage->setUrl(loginUrl);
    }
}

QString WebLoginDialog::currentServerName()
{
    if (ui->cmbServers->currentIndex() >=0 && ui->cmbServers->isVisible()){
        return ui->cmbServers->currentText();
    }
    return QString();
}
