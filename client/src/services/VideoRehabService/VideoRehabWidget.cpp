#include "VideoRehabWidget.h"
#include "ui_VideoRehabWidget.h"

VideoRehabWidget::VideoRehabWidget(ComManager *comMan, QWidget *parent) :
    BaseServiceWidget(comMan, parent),
    ui(new Ui::VideoRehabWidget)
{
    ui->setupUi(this);

    initUI();
    connectSignals();

}

VideoRehabWidget::~VideoRehabWidget()
{
    m_loadingIcon->deleteLater();
    m_webPage->deleteLater();
    m_webEngine->deleteLater();
    delete ui;

}

void VideoRehabWidget::initUI()
{
    // Hide debug url text box
    ui->txtURL->hide();

    // Set and start loading
    ui->frameError->hide();
    setLoading(true);
    ui->wdgWebEngine->hide();

    m_loadingIcon = new QMovie("://status/calling.gif");
    ui->icoLoading->setMovie(m_loadingIcon);
    m_loadingIcon->start();

    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
    m_webEngine = new QWebEngineView(ui->wdgWebEngine);
    connect(m_webEngine, &QWebEngineView::loadFinished, this, &VideoRehabWidget::webPageLoaded);

    //Create a new page
    m_webPage = new VideoRehabWebPage(m_webEngine);
    connect(m_webPage->getSharedObject(), &SharedObject::pageIsReady, this, &VideoRehabWidget::webPageReady);
    connect(m_webPage->getSharedObject(), &SharedObject::videoErrorOccured, this, &VideoRehabWidget::webPageVideoError);
    connect(m_webPage->getSharedObject(), &SharedObject::audioErrorOccured, this, &VideoRehabWidget::webPageAudioError);
    connect(m_webPage->getSharedObject(), &SharedObject::generalErrorOccured, this, &VideoRehabWidget::webPageGeneralError);

    //Set page to view
    m_webEngine->setPage(m_webPage);

    QWebEngineProfile::defaultProfile()->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    QWebEngineProfile::defaultProfile()->setHttpCacheType(QWebEngineProfile::NoCache);


    // Create layout for widget if missing
    if (!ui->wdgWebEngine->layout()){
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        ui->wdgWebEngine->setLayout(layout);
    }
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_webEngine->setSizePolicy(sizePolicy);
    ui->wdgWebEngine->layout()->addWidget(m_webEngine);
}

bool VideoRehabWidget::handleJoinSessionEvent(const JoinSessionEvent &event)
{
    // Redirect web engine to session url
    if (m_webPage){
        QString session_url = QString::fromStdString(event.session_url());
        if (m_webPage->url().toString() != session_url){
            m_webPage->setUrl(session_url);
            return true;  // Accepts the request
        }
    }

    return false;  // Refuses the request

}

void VideoRehabWidget::on_txtURL_returnPressed()
{
    m_webEngine->setUrl(ui->txtURL->text());
}

void VideoRehabWidget::webEngineURLChanged(QUrl url)
{
    ui->txtURL->setText(url.toString());
}

void VideoRehabWidget::webPageLoaded(bool ok)
{
    if (!ok){
        LOG_ERROR(tr("Impossible de charger la page de prévisualisation de la caméra"), "VideoRehabSetupWidget::webPageLoaded");
        return;
    }
    setLoading(false);
}

void VideoRehabWidget::webPageReady()
{
    refreshWebpageSettings();
}

void VideoRehabWidget::webPageVideoError(QString context, QString error)
{
    showError(tr("Problème vidéo"), context, error);
}

void VideoRehabWidget::webPageAudioError(QString context, QString error)
{
    showError(tr("Problème audio"), context, error);
}

void VideoRehabWidget::webPageGeneralError(QString context, QString error)
{
    showError(tr("Erreur"), context, error);
}

void VideoRehabWidget::connectSignals()
{
    if (m_webEngine){
        //connect(m_webEngine, &QWebEngineView::urlChanged, this, &VideoRehabWidget::webEngineURLChanged);
    }
    if (m_webPage){
        connect(m_webPage, &QWebEnginePage::urlChanged, this, &VideoRehabWidget::webEngineURLChanged);
    }

}

void VideoRehabWidget::refreshWebpageSettings()
{
    // TODO: Set camera and audio
}

void VideoRehabWidget::setLoading(const bool &loading)
{
    ui->frameLoading->setVisible(loading);
    if (!ui->frameError->isVisible()){
        ui->wdgWebEngine->setVisible(!loading);
    }
}

void VideoRehabWidget::showError(const QString &title, const QString &context, const QString &error)
{
    ui->lblTitle->setText(title);
#ifdef QT_DEBUG
    ui->lblError->setText(context + " - " + error);
#else
    Q_UNUSED(context)
    ui->lblError->setText(error);
#endif
    ui->frameLoading->hide();
    ui->frameError->show();
    ui->wdgWebEngine->hide();
}
