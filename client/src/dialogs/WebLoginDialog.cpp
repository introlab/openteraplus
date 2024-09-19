#include "WebLoginDialog.h"
#include "ui_WebLoginDialog.h"
#include <QVBoxLayout>


WebLoginDialog::WebLoginDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::WebLoginDialog)
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

    m_webPage->setUrl(QUrl("https://127.0.0.1:40075/login"));

    m_webView->setPage(m_webPage);
}

WebLoginDialog::~WebLoginDialog()
{
    delete ui;
}


void WebLoginDialog::onCertificateError(const QWebEngineCertificateError &certificateError)
{
    //TODO do Something about certificates
    qDebug() << "Certificate error: " << certificateError.description();
    //TODO Do not accept certificates in production ?
    auto mutableError = const_cast<QWebEngineCertificateError&>(certificateError);
    mutableError.acceptCertificate();
}
