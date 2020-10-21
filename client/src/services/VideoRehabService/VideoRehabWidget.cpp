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
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, false);

    m_webEngine = new QWebEngineView(ui->wdgWebEngine);
    connect(m_webEngine, &QWebEngineView::loadFinished, this, &VideoRehabWidget::webPageLoaded);

    // Create a new page
    m_webPage = new VideoRehabWebPage(m_webEngine);
    connect(m_webPage->getSharedObject(), &SharedObject::pageIsReady, this, &VideoRehabWidget::webPageReady);
    connect(m_webPage->getSharedObject(), &SharedObject::videoErrorOccured, this, &VideoRehabWidget::webPageVideoError);
    connect(m_webPage->getSharedObject(), &SharedObject::audioErrorOccured, this, &VideoRehabWidget::webPageAudioError);
    connect(m_webPage->getSharedObject(), &SharedObject::generalErrorOccured, this, &VideoRehabWidget::webPageGeneralError);

    // Set current user informations
    m_webPage->getSharedObject()->setContactInformation(m_comManager->getCurrentUser().getName(),
                                                        m_comManager->getCurrentUser().getUuid());

    // Build shared object from session config
    processSessionConfig();

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
            // Append source to the URL to connect QWebChannel
            session_url += "&source=openteraplus";
            m_webPage->setUrl(session_url);
            return true;  // Accepts the request
        }
    }

    return false;  // Refuses the request

}

void VideoRehabWidget::reload()
{
    m_webEngine->reload();
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

void VideoRehabWidget::processSessionConfig()
{
    if (m_webPage){
        QJsonDocument session_config = m_comManager->getCurrentSessionConfig();
        SharedObject* shared = m_webPage->getSharedObject();
        if (session_config.object().contains("service_config_config")){
            QVariantHash session_params = session_config["service_config_config"].toObject().toVariantHash();
            if (session_params.contains("camera")) shared->setCurrentCameraName(session_params["camera"].toString());
            if (session_params.contains("audio")) shared->setCurrentAudioSrcName(session_params["audio"].toString());
            if (session_params.contains("mirror")) shared->setLocalMirror(session_params["mirror"].toBool());
            if (session_params.contains("extra_params")) shared->setExtraParams(session_params["extra_params"].toString());
            if (session_params.contains("camera2")) shared->setSecondVideoName(session_params["camera2"].toString());
            if (session_params.contains("audio2")) shared->setSecondAudioSrcName(session_params["audio2"].toString());
            if (session_params.contains("camera_ptz")) shared->setPTZCapabilities(session_params["camera_ptz"].toBool(),
                    session_params["camera_ptz"].toBool(), session_params["camera_ptz"].toBool()); // For now, all features enabled!
        }else{
            LOG_WARNING("Wrong session config format for that service", "VideoRehabWidget::processSessionConfig");
        }
    }
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
