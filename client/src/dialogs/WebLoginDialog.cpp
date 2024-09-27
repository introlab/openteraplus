#include "WebLoginDialog.h"
#include "ui_WebLoginDialog.h"

#include <QVBoxLayout>

#include "Utils.h"
#include "TeraSettings.h"

WebLoginDialog::WebLoginDialog(ConfigManagerClient *config, QWidget *parent)
    : QDialog(parent), ui(new Ui::WebLoginDialog), m_config(config)
{
    ui->setupUi(this);

    //Create Web View
    m_webView = new QWebEngineView(ui->centralWidget);
    ui->lblError->hide();
    ui->centralWidget->hide();

    QVBoxLayout *layout = new QVBoxLayout(ui->centralWidget);
    layout->addWidget(m_webView);

    m_requestInterceptor = new WebLoginRequestInterceptor(this);

    m_webPage = new QWebEnginePage(m_webView);
    m_webPage->setUrlRequestInterceptor(m_requestInterceptor);

    QWebChannel *channel = new QWebChannel(m_webPage);

    WebLoginSharedObject *myObject = new WebLoginSharedObject();
    channel->registerObject("qtObject", myObject);
    m_webPage->setWebChannel(channel);

    auto settings = m_webPage->settings();
    settings->setAttribute(QWebEngineSettings::ShowScrollBars, false);

    connect(m_webPage, &QWebEnginePage::certificateError, this, &WebLoginDialog::onCertificateError);
    connect(m_webView, &QWebEngineView::loadFinished, this, &WebLoginDialog::onLoginPageLoaded);
    connect(m_webView, &QWebEngineView::loadStarted, this, &WebLoginDialog::onLoginPageLoading);

    connect(myObject, &WebLoginSharedObject::loginSuccess, this, &WebLoginDialog::loginSuccess);
    connect(myObject, &WebLoginSharedObject::loginFailure, this, &WebLoginDialog::loginFailure);

    m_webPage->setBackgroundColor(QColor(0x2c3338));
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
        loginUrl.setQuery("no_logo");
        m_webPage->setUrl(loginUrl);
    }
}

void WebLoginDialog::onLoginPageLoaded(bool ok)
{
    if (ok){
        ui->centralWidget->show();
        ui->frameLoginMessages->hide();
        m_webView->setFocus();
    }else{
        ui->lblError->show();
        ui->lblLoading->hide();
    }
}

void WebLoginDialog::onLoginPageLoading()
{
    ui->centralWidget->hide();
    ui->lblError->hide();
    ui->lblLoading->show();
    ui->frameLoginMessages->show();
}

QString WebLoginDialog::currentServerName()
{
    if (ui->cmbServers->currentIndex() >=0 && ui->cmbServers->isVisible()){
        return ui->cmbServers->currentText();
    }
    return QString();
}


WebLoginRequestInterceptor::WebLoginRequestInterceptor(QObject *p) : QWebEngineUrlRequestInterceptor(p)
{
    // Cache OS information
    m_osName = Utils::getOsName();
    m_osVersion = Utils::getOsVersion();
}

WebLoginRequestInterceptor::~WebLoginRequestInterceptor()
{

}

void WebLoginRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    // Inject client name and version
    info.setHttpHeader("X-Client-Name", QByteArray(OPENTERAPLUS_CLIENT_NAME));
    info.setHttpHeader("X-Client-Version", QByteArray(OPENTERAPLUS_VERSION));
    info.setHttpHeader("X-OS-Name", m_osName.toUtf8());
    info.setHttpHeader("X-OS-Version", m_osVersion.toUtf8());

    // Inject required language
    QString localeString = QLocale().bcp47Name();
    //qDebug() << "localeString : " << localeString;
    info.setHttpHeader(QByteArray("Accept-Language"), localeString.toUtf8());
}

