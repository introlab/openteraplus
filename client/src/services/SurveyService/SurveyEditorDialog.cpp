#include "SurveyEditorDialog.h"
#include "ui_SurveyEditorDialog.h"

#include <QWebEngineProfile>
#include <QFileDialog>
#include <QStandardPaths>

SurveyEditorDialog::SurveyEditorDialog(SurveyComManager *surveyComManager, const QString& test_type_uuid, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SurveyEditorDialog)
    , m_surveyComManager(surveyComManager)
    , m_testTypeUuid(test_type_uuid)
{
    ui->setupUi(this);
    ui->wdgWebView->hide();
    ui->frameLoading->hide();
    ui->frameError->hide();

    // Setup loading icon animation
    m_loadingIcon = new QMovie("://status/loading.gif");
    ui->lblLoadingIcon->setMovie(m_loadingIcon);
    m_loadingIcon->start();

    m_webView = new QWebEngineView(ui->wdgWebView);
    m_webView->setContextMenuPolicy(Qt::NoContextMenu);

    QVBoxLayout *layout = new QVBoxLayout(ui->wdgWebView);
    layout->addWidget(m_webView);

    /*QWebEngineProfile * profile = new QWebEngineProfile("OpenTeraPlusSurveyService", m_webView);
    profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    qDebug() << "Cache Path = " << profile->cachePath();*/

    m_requestInterceptor = new WebPageRequestInterceptor(false, this);
    m_webPage = new QWebEnginePage(m_webView);
    m_webPage->setUrlRequestInterceptor(m_requestInterceptor);

    connect(m_webPage, &QWebEnginePage::certificateError, this, &SurveyEditorDialog::onCertificateError);
    connect(m_webPage, &QWebEnginePage::loadingChanged, this, &SurveyEditorDialog::onPageLoadingChanged);
    connect(m_webPage, &QWebEnginePage::loadProgress, this, &SurveyEditorDialog::onPageLoadingProcess);
    m_webView->setPage(m_webPage);

    connect(m_webPage->profile(), &QWebEngineProfile::downloadRequested, this, &SurveyEditorDialog::onFileDownloadRequested);

    connect(m_surveyComManager, &SurveyComManager::userTokenUpdated, this, &SurveyEditorDialog::onUserTokenUpdated);

}

SurveyEditorDialog::~SurveyEditorDialog()
{
    delete ui;
}

void SurveyEditorDialog::setCurrentTestTypeUuid(const QString &test_type_uuid)
{
    m_testTypeUuid = test_type_uuid;
}

void SurveyEditorDialog::loadEditor()
{
    ui->wdgWebView->hide();
    QUrl editor_url = m_surveyComManager->getServerUrl();
    editor_url.setPath(m_surveyComManager->getServiceEndpoint("editor"));
    QUrlQuery args;
    args.addQueryItem("token", m_surveyComManager->getCurrentToken());
    args.addQueryItem("test_type_uuid", m_testTypeUuid);
    editor_url.setQuery(args);
    //qDebug() << "SurveyEditorDialog - Loading " << editor_url.toString();
    m_webPage->load(editor_url);
}

void SurveyEditorDialog::loadManager(const int &id_project)
{
    ui->wdgWebView->hide();
    QUrl manager_url = m_surveyComManager->getServerUrl();
    manager_url.setPath(m_surveyComManager->getServiceEndpoint("manager"));
    QUrlQuery args;
    args.addQueryItem("token", m_surveyComManager->getCurrentToken());
    args.addQueryItem("test_type_uuid", m_testTypeUuid);
    //args.addQueryItem("id_project", QString::number(id_project));
    manager_url.setQuery(args);
    m_webPage->load(manager_url);
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
    if (loadingInfo.status() == QWebEngineLoadingInfo::LoadStartedStatus){
        ui->progressLoading->setValue(0);
    }
    ui->frameLoading->setVisible(loadingInfo.status() == QWebEngineLoadingInfo::LoadStartedStatus);
    ui->frameError->setVisible(loadingInfo.status() == QWebEngineLoadingInfo::LoadFailedStatus);

    ui->wdgWebView->setVisible(loadingInfo.status() == QWebEngineLoadingInfo::LoadSucceededStatus);

    //qDebug() << loadingInfo.status();
    if (loadingInfo.isErrorPage())
        qDebug() << "Page loading error: " << loadingInfo.errorCode() << ": " << loadingInfo.errorString();
}

void SurveyEditorDialog::onPageLoadingProcess(int progress)
{
    ui->progressLoading->setValue(progress);
}

void SurveyEditorDialog::onFileDownloadRequested(QWebEngineDownloadRequest *download)
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Enregistrer sous"), QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first() + "/" + download->suggestedFileName());
    if (!filename.isEmpty()){
        download->setDownloadFileName(filename);
        download->accept();
    }else{
        download->cancel();
    }
}

void SurveyEditorDialog::onUserTokenUpdated()
{
    //qDebug() << "Changing user token in Survey Editor";
    m_webPage->runJavaScript("set_user_token(\"" + m_surveyComManager->getCurrentToken() + "\");");
}

