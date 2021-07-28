#include <QStandardPaths>

#include "VideoRehabWidget.h"
#include "ui_VideoRehabWidget.h"

#include "GlobalMessageBox.h"

VideoRehabWidget::VideoRehabWidget(ComManager *comMan, QWidget *parent) :
    BaseServiceWidget(comMan, parent),
    ui(new Ui::VideoRehabWidget)
{
    ui->setupUi(this);

    m_virtualCamThread = nullptr;

    initUI();
    connectSignals();

    emit widgetIsReady(false); // We wait until webpage is fully loaded...


}

VideoRehabWidget::~VideoRehabWidget()
{
    qDebug() << "~VideoRehabWidget";
    m_loadingIcon->deleteLater();
    m_webPage->deleteLater();
    m_webEngine->deleteLater();
    if (m_virtualCamThread){
        m_virtualCamThread->quit();
        m_virtualCamThread->wait();
        m_virtualCamThread->deleteLater();
     }

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

    QWebEngineProfile::defaultProfile()->setPersistentCookiesPolicy(/*QWebEngineProfile::AllowPersistentCookies*/QWebEngineProfile::NoPersistentCookies);
    QWebEngineProfile::defaultProfile()->setHttpCacheType(/*QWebEngineProfile::DiskHttpCache*/QWebEngineProfile::NoCache);

    // Set download path
    QStringList documents_path = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    QString download_path = ""; // TODO: Other default download path?
    if (!documents_path.empty())
        download_path = documents_path.first();
    download_path += "/OpenTeraPlus/downloads/";
    QWebEngineProfile::defaultProfile()->setDownloadPath(download_path);

    connect(QWebEngineProfile::defaultProfile(), &QWebEngineProfile::downloadRequested, this, &VideoRehabWidget::webEngineDownloadRequested);


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
        if (!m_webPage->url().toString().contains(session_url)){
            // Append source to the URL to connect QWebChannel
            session_url += "&source=openteraplus";

            // Append language to set the page to correct language
            session_url += "&lang=" + m_comManager->getCurrentPreferences().getLanguage();

            m_webPage->setUrl(session_url);
            return true;  // Accepts the request
        }
    }

    return false;  // Refuses the request

}

void VideoRehabWidget::reload()
{
    ui->frameError->hide();
    ui->wdgWebEngine->show();
    m_webEngine->reload();
    emit widgetIsReady(false);
}

void VideoRehabWidget::startRecording()
{
    if (m_webPage){
        m_webPage->getSharedObject()->startRecording();
    }
}

void VideoRehabWidget::stopRecording()
{
    if (m_webPage){
        m_webPage->getSharedObject()->stopRecording();
    }
}

void VideoRehabWidget::on_txtURL_returnPressed()
{
    m_webEngine->setUrl(ui->txtURL->text());
}

void VideoRehabWidget::webEngineURLChanged(QUrl url)
{
    ui->txtURL->setText(url.toString());
}

void VideoRehabWidget::webEngineDownloadRequested(QWebEngineDownloadItem *item)
{
    //qDebug() << "WebEngine: about to download " << item->suggestedFileName();
    emit widgetIsReady(false);

    // Rework filename
    QString file_name = item->suggestedFileName();
    QDir target_dir(item->downloadDirectory());
    QString part_name;

    if (m_session){
        file_name = "";
        QStringList file_parts = item->downloadFileName().split(".");
        if (m_session->hasFieldName("session_participants")){
            QVariantList item_list;
            item_list = m_session->getFieldValue("session_participants").toList();
            if (!item_list.isEmpty()){
                QVariantMap part_info = item_list.first().toMap();
                part_name = Utils::removeAccents(part_info["participant_name"].toString());
                file_name = part_name + "_";
            }
        }
        file_name += Utils::removeAccents(m_session->getName());

        // Append file number
        //file_name = Utils::removeAccents(file_name);
        //file_name = file_name.replace(" ", "_").replace(".","_");
        int file_num = target_dir.entryList(QStringList() << ("*" + part_name + "*." + file_parts.last()),
                                            QDir::Files).count() + 1;
        file_name += "_" + QString::number(file_num) + "." + file_parts.last();
        item->setDownloadFileName(file_name);
    }

    connect(item, &QWebEngineDownloadItem::finished, this, &VideoRehabWidget::webEngineDownloadCompleted);
    item->accept();

}

void VideoRehabWidget::webEngineDownloadCompleted()
{

    // Enable buttons
    emit widgetIsReady(true);

    QWebEngineDownloadItem* item = dynamic_cast<QWebEngineDownloadItem*>(sender());
    if (item){
        if (item->receivedBytes() == 0)
            return;
        GlobalMessageBox msg_box;
        msg_box.setTextFormat(Qt::RichText);
        QString full_dir_path = item->downloadDirectory();
        QString file_name = item->downloadFileName();
        QString full_file_path;
    #ifdef Q_OS_WINDOWS
        full_dir_path = full_dir_path.replace("/","\\");
        full_file_path = full_dir_path + "\\" + file_name;
    #else
        full_file_path = full_dir_path + "/" + file_name;
    #endif

        msg_box.showInfo(tr("Fichier disponible"), tr("Le fichier") + "\n <a href=\"file:///" + full_file_path + "\" style=\"color: cyan;\">" + file_name + "</a>\n" +
                         tr("est disponible dans le répertoire") + "\n <a href=\"file:///" + full_dir_path + "\" style=\"color: cyan;\">" + item->downloadDirectory() + "</a>");

    }

}

void VideoRehabWidget::webPageLoaded(bool ok)
{
    if (!ok){
        LOG_ERROR(tr("Impossible de charger la page"), "VideoRehabWidget::webPageLoaded");
        //return;
    }
    setLoading(false);
    emit widgetIsReady(true);
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

void VideoRehabWidget::virtualCameraDisconnected()
{
    //showError(tr("Erreur de caméra"), "VideoRehabSetupWidget::virtualCameraDisconnected", tr("Impossible de se connecter à la source vidéo."));
    stopVirtualCamera();
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
    QJsonDocument session_config = m_comManager->getCurrentSessionConfig();
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
    }
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

void VideoRehabWidget::startVirtualCamera(const QString &src)
{
    if (m_virtualCamThread){
        stopVirtualCamera();
    }
    ui->frameError->hide();
    m_virtualCamThread = new VirtualCameraThread(src);
    connect(m_virtualCamThread, &VirtualCameraThread::virtualCamDisconnected, this, &VideoRehabWidget::virtualCameraDisconnected);
    m_virtualCamThread->start();
}

void VideoRehabWidget::stopVirtualCamera()
{
    qDebug() << "VideoRehabWidget::stopVirtualCamera";
    if (m_virtualCamThread){
        m_virtualCamThread->quit();
        m_virtualCamThread->wait();
        m_virtualCamThread->deleteLater();
        m_virtualCamThread = nullptr;
    }
}

void VideoRehabWidget::on_btnRefresh_clicked()
{
    reload();
}
