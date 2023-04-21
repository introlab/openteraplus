#include <QStandardPaths>

#include "KitVideoRehabWidget.h"
#include "ui_KitVideoRehabWidget.h"

#include "GlobalMessageBox.h"

KitVideoRehabWidget::KitVideoRehabWidget(KitConfigManager *kitConfig, ParticipantComManager *partCom, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KitVideoRehabWidget),
    m_kitConfig(kitConfig),
    m_partComManager(partCom)
{
    ui->setupUi(this);

    //m_virtualCamThread = nullptr;

    initUi();
    connectSignals();

    // Set config to shared object
    processKitDevice();

    // Redirect web engine to correct url
    loadConfiguredUrl();
}

KitVideoRehabWidget::~KitVideoRehabWidget()
{
    qDebug() << "~KitVideoRehabWidget";
    m_loadingIcon->deleteLater();
    m_webPage->deleteLater();
    m_webEngine->deleteLater();
    /*
    if (m_virtualCamThread){
        m_virtualCamThread->quit();
        m_virtualCamThread->wait();
        m_virtualCamThread->deleteLater();
     }
    */

    delete ui;

}

void KitVideoRehabWidget::initUi()
{

    // Set and start loading
    ui->frameError->hide();
    setLoading(true);
    ui->wdgWebEngine->hide();

    m_loadingIcon = new QMovie("://status/calling.gif");
    ui->icoLoading->setMovie(m_loadingIcon);
    m_loadingIcon->start();

    m_webEngine = new QWebEngineView(ui->wdgWebEngine);
    connect(m_webEngine, &QWebEngineView::loadFinished, this, &KitVideoRehabWidget::webPageLoaded);

    // Create a new page
    // DL 21/04/2023 settings are now in VideoRehabWebPage constructor
    m_webPage = new VideoRehabWebPage(m_webEngine);
    connect(m_webPage->getSharedObject(), &SharedObject::pageIsReady, this, &KitVideoRehabWidget::webPageReady);
    connect(m_webPage->getSharedObject(), &SharedObject::videoErrorOccured, this, &KitVideoRehabWidget::webPageVideoError);
    connect(m_webPage->getSharedObject(), &SharedObject::audioErrorOccured, this, &KitVideoRehabWidget::webPageAudioError);
    connect(m_webPage->getSharedObject(), &SharedObject::generalErrorOccured, this, &KitVideoRehabWidget::webPageGeneralError);

    // Set current user informations
    m_webPage->getSharedObject()->setContactInformation(m_partComManager->getCurrentParticipant().getName(),
                                                        m_partComManager->getCurrentParticipant().getUuid());

    // Build shared object from kit config

    //processSessionConfig();

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

void KitVideoRehabWidget::reload()
{
    ui->frameError->hide();
    ui->wdgWebEngine->show();
    m_webEngine->reload();
}

void KitVideoRehabWidget::webPageLoaded(bool ok)
{
    if (!ok){
        LOG_ERROR(tr("Impossible de charger la page"), "KitVideoRehabWidget::webPageLoaded");
        //return;
    }
    setLoading(false);
}

void KitVideoRehabWidget::webPageReady()
{
    refreshWebpageSettings();
}

void KitVideoRehabWidget::webPageVideoError(QString context, QString error)
{
    showError(tr("Problème vidéo"), context, error);
}

void KitVideoRehabWidget::webPageAudioError(QString context, QString error)
{
    showError(tr("Problème audio"), context, error);
}

void KitVideoRehabWidget::webPageGeneralError(QString context, QString error)
{
    showError(tr("Erreur"), context, error);
}

void KitVideoRehabWidget::virtualCameraDisconnected()
{
    //showError(tr("Erreur de caméra"), "VideoRehabSetupWidget::virtualCameraDisconnected", tr("Impossible de se connecter à la source vidéo."));
    stopVirtualCamera();
}

void KitVideoRehabWidget::connectSignals()
{
    if (m_webEngine){
        //connect(m_webEngine, &QWebEngineView::urlChanged, this, &VideoRehabWidget::webEngineURLChanged);
    }
    if (m_webPage){
        //connect(m_webPage, &QWebEnginePage::urlChanged, this, &VideoRehabWidget::webEngineURLChanged);
    }

}

void KitVideoRehabWidget::refreshWebpageSettings()
{
    /*QJsonDocument session_config = m_comManager->getCurrentSessionConfig();
    if (session_config.object().contains("service_config_config")){
        SharedObject* shared = m_webPage->getSharedObject();
        QVariantHash session_params = session_config["service_config_config"].toObject().toVariantHash();
        // Check if we must hide "OpenTeraCam" from video source list
        if (session_params.contains("teracam_src")){
            if (session_params["teracam_src"].toString().isEmpty()){
                shared->removeVideoSource("OpenTeraCam");
            }
        }else{
            shared->removeVideoSource("OpenTeraCam");
        }
    }*/
}

void KitVideoRehabWidget::loadConfiguredUrl()
{
    QString base_url = m_kitConfig->getParticipantServiceUrl();

    // Append source
    base_url += "&source=openteraplus";

    // Set url
    m_webEngine->setUrl(base_url);
}

void KitVideoRehabWidget::processKitDevice()
{
    if (m_webPage){
        QJsonObject     kit_config = m_kitConfig->getKitConfig();
        SharedObject*   shared = m_webPage->getSharedObject();
        QVariantHash    session_params = kit_config.toVariantHash();

        if (session_params.contains("camera")) shared->setCurrentCameraName(session_params["camera"].toString());
        if (session_params.contains("audio")) shared->setCurrentAudioSrcName(session_params["audio"].toString());
        if (session_params.contains("mirror")) shared->setLocalMirror(session_params["mirror"].toBool());
        if (session_params.contains("extra_params")) shared->setExtraParams(session_params["extra_params"].toString());
        if (session_params.contains("camera2")) shared->setSecondVideoName(session_params["camera2"].toString());
        if (session_params.contains("audio2")) shared->setSecondAudioSrcName(session_params["audio2"].toString());
        if (session_params.contains("camera_ptz")){
            // Start PTZ camera driver
            int camera_src = session_params["camera_ptz_type"].toInt();
            if (camera_src == 0){ // TODO: Better manage camera sources
                // Vivotek
                if (m_webPage){
                    SharedObject* shared = m_webPage->getSharedObject();
                    if (shared){
                        shared->startPTZCameraDriver(camera_src,
                                                     "OpenTeraCam", // Hard coded for now, but should be associated with a specific camera name
                                                     session_params["camera_ptz_ip"].toString(),
                                                     session_params["camera_ptz_port"].toInt(),
                                                     session_params["camera_ptz_username"].toString(),
                                                     session_params["camera_ptz_password"].toString());
                       // Connect signal
                       // connect(shared->getPTZCameraDriver(), &ICameraDriver::cameraError, this, &VideoRehabSetupWidget::ptzCameraError);
                    }
                }
            }
        }

        if (session_params.contains("teracam_src")){
            if (!session_params["teracam_src"].toString().isEmpty()){
                // Start virtual camera driver
                startVirtualCamera(session_params["teracam_src"].toString());
           }
        }
    }
}
/*
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
            if (session_params.contains("camera_ptz")){
                // Start PTZ camera driver
                int camera_src = session_params["camera_ptz_type"].toInt();
                if (camera_src == 0){ // TODO: Better manage camera sources
                    // Vivotek
                    if (m_webPage){
                        SharedObject* shared = m_webPage->getSharedObject();
                        if (shared){
                            shared->startPTZCameraDriver(camera_src,
                                                         "OpenTeraCam", // Hard coded for now, but should be associated with a specific camera name
                                                         session_params["camera_ptz_ip"].toString(),
                                                         session_params["camera_ptz_port"].toInt(),
                                                         session_params["camera_ptz_username"].toString(),
                                                         session_params["camera_ptz_password"].toString());
                           // Connect signal
                           // connect(shared->getPTZCameraDriver(), &ICameraDriver::cameraError, this, &VideoRehabSetupWidget::ptzCameraError);
                        }
                    }
                }
            }

            if (session_params.contains("teracam_src")){
                if (!session_params["teracam_src"].toString().isEmpty()){
                    // Start virtual camera driver
                    startVirtualCamera(session_params["teracam_src"].toString());
               }
            }
        }else{
            LOG_WARNING("Wrong session config format for that service", "VideoRehabWidget::processSessionConfig");
        }
    }
}*/

void KitVideoRehabWidget::setLoading(const bool &loading)
{
    ui->frameLoading->setVisible(loading);
    if (!ui->frameError->isVisible()){
        ui->wdgWebEngine->setVisible(!loading);
    }
}

void KitVideoRehabWidget::showError(const QString &title, const QString &context, const QString &error)
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

void KitVideoRehabWidget::startVirtualCamera(const QString &src)
{
    qDebug() << "KitVideoRehabWidget::startVirtualCamera not implemented.";
    /*
    if (m_virtualCamThread){
        stopVirtualCamera();
    }
    ui->frameError->hide();
    m_virtualCamThread = new VirtualCameraThread(src);
    connect(m_virtualCamThread, &VirtualCameraThread::virtualCamDisconnected, this, &KitVideoRehabWidget::virtualCameraDisconnected);
    m_virtualCamThread->start();
    */
}

void KitVideoRehabWidget::stopVirtualCamera()
{
    qDebug() << "KitVideoRehabWidget::stopVirtualCamera not implemented.";
    /*
    if (m_virtualCamThread){
        m_virtualCamThread->quit();
        m_virtualCamThread->wait();
        m_virtualCamThread->deleteLater();
        m_virtualCamThread = nullptr;
    }
    */
}

void KitVideoRehabWidget::on_btnRefresh_clicked()
{
    reload();
}
