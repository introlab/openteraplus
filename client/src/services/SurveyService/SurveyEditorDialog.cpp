#include "SurveyEditorDialog.h"
#include "ui_SurveyEditorDialog.h"

SurveyEditorDialog::SurveyEditorDialog(SurveyComManager *surveyComManager, const QString& test_type_uuid, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SurveyEditorDialog)
    , m_surveyComManager(surveyComManager)
    , m_testTypeUuid(test_type_uuid)
{
    ui->setupUi(this);
    m_webView = new QWebEngineView(ui->wdgWebView);
    m_webView->setContextMenuPolicy(Qt::NoContextMenu);

    QVBoxLayout *layout = new QVBoxLayout(ui->wdgWebView);
    layout->addWidget(m_webView);

    m_webPage = new QWebEnginePage(m_webView);
    connect(m_webPage, &QWebEnginePage::certificateError, this, &SurveyEditorDialog::onCertificateError);
    connect(m_webPage, &QWebEnginePage::loadingChanged, this, &SurveyEditorDialog::onPageLoadingChanged);
    m_webView->setPage(m_webPage);

    loadEditor();
}

SurveyEditorDialog::~SurveyEditorDialog()
{
    delete ui;
}

void SurveyEditorDialog::loadEditor()
{
    QUrl editor_url = m_surveyComManager->getServerUrl();
    editor_url.setPath(m_surveyComManager->getServiceEndpoint("editor"));
    QUrlQuery args;
    args.addQueryItem("token", m_surveyComManager->getCurrentToken());
    args.addQueryItem("test_type_uuid", m_testTypeUuid);
    editor_url.setQuery(args);
    qDebug() << "SurveyEditorDialog - Loading " << editor_url.toString();
    m_webPage->load(editor_url);
}

void SurveyEditorDialog::on_btnClose_clicked()
{
    reject();
}

void SurveyEditorDialog::onCertificateError(const QWebEngineCertificateError &certificateError)
{
    //TODO do Something about certificates
    qDebug() << "Certificate error: " << certificateError.description();
    //TODO Do not accept certificates in production ?
    auto mutableError = const_cast<QWebEngineCertificateError&>(certificateError);
    mutableError.acceptCertificate();
}

void SurveyEditorDialog::onPageLoadingChanged(const QWebEngineLoadingInfo &loadingInfo)
{
    qDebug() << loadingInfo.status();
    if (loadingInfo.isErrorPage())
        qDebug() << "Page loading error: " << loadingInfo.errorCode() << ": " << loadingInfo.errorString();
}

