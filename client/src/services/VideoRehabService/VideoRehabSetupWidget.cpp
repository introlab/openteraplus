#include "VideoRehabSetupWidget.h"
#include "ui_VideoRehabSetupWidget.h"

#include "Utils.h"

VideoRehabSetupWidget::VideoRehabSetupWidget(ComManager *comManager, QWidget *parent) :
    BaseServiceSetupWidget(comManager, parent),
    ui(new Ui::VideoRehabSetupWidget)
{
    ui->setupUi(this);

    setLoading(true); // Disable until page is fully loaded

    m_valueJustChanged = false;
    m_virtualCamThread = nullptr;

    initUI();
    connectSignals();

    // Query service config form for VideoRehab
    QUrlQuery args(WEB_FORMS_QUERY_SERVICE_CONFIG);
    args.addQueryItem(WEB_QUERY_KEY, "VideoRehabService");
    m_comManager->doQuery(WEB_FORMS_PATH, args);

}

VideoRehabSetupWidget::~VideoRehabSetupWidget()
{
    delete ui;

    m_webPage->deleteLater();
    m_webEngine->deleteLater();

    if (m_virtualCamThread){
        m_virtualCamThread->quit();
        m_virtualCamThread->deleteLater();
     }
}

QJsonDocument VideoRehabSetupWidget::getSetupConfig()
{
    return ui->widgetSetup->getFormDataJson(true);
}

void VideoRehabSetupWidget::initUI()
{
    ui->frameError->hide();

    //// Web engine setup
    m_webEngine = new QWebEngineView(ui->wdgWebEngine);
    connect(m_webEngine, &QWebEngineView::loadFinished, this, &VideoRehabSetupWidget::webPageLoaded);

    //Create a new page
    m_webPage = new VideoRehabWebPage(m_webEngine);
    connect(m_webPage->getSharedObject(), &SharedObject::pageIsReady, this, &VideoRehabSetupWidget::webPageReady);
    connect(m_webPage->getSharedObject(), &SharedObject::currentCameraWasChanged, this, &VideoRehabSetupWidget::webPageCameraChanged);
    connect(m_webPage->getSharedObject(), &SharedObject::videoErrorOccured, this, &VideoRehabSetupWidget::webPageVideoError);
    connect(m_webPage->getSharedObject(), &SharedObject::audioErrorOccured, this, &VideoRehabSetupWidget::webPageAudioError);
    connect(m_webPage->getSharedObject(), &SharedObject::generalErrorOccured, this, &VideoRehabSetupWidget::webPageGeneralError);

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

    // Wait for service configuration before setting url
    //m_webEngine->setUrl(QUrl("qrc:/VideoRehabService/html/index.html"));
}


void VideoRehabSetupWidget::connectSignals()
{
    connect(m_comManager, &ComManager::servicesConfigReceived, this, &VideoRehabSetupWidget::processServiceConfigsReply);
    connect(m_comManager, &ComManager::formReceived, this, &VideoRehabSetupWidget::processFormsReply);

    connect(ui->widgetSetup, &TeraForm::widgetValueHasChanged, this, &VideoRehabSetupWidget::setupFormValueChanged);
    connect(ui->widgetSetup, &TeraForm::formIsNowDirty, this, &VideoRehabSetupWidget::setupFormDirtyChanged);

}

void VideoRehabSetupWidget::setLoading(const bool &loading)
{
    ui->widgetSetup->setVisible(!loading);
    ui->widgetSetup->setEnabled(!loading);
    //ui->splitter->setVisible(!loading);
    ui->frameButtons->setVisible(!loading);
    if (!ui->frameError->isVisible()){
        ui->wdgWebEngine->setVisible(!loading);
        ui->lblLoading->setVisible(loading);
    }

}

void VideoRehabSetupWidget::showError(const QString &title, const QString &context, const QString &error, bool hide_retry)
{
    ui->lblTitle->setText(title);
#ifdef QT_DEBUG
    ui->lblError->setText(context + " - " + error);
#else
    ui->lblError->setText(error);
#endif
    ui->lblLoading->hide();
    ui->btnRefresh->setVisible(!hide_retry);
    ui->frameError->show();
}

void VideoRehabSetupWidget::startVirtualCamera(const QString &src)
{
    if (m_virtualCamThread){
        stopVirtualCamera();
    }
    ui->frameError->hide();
    m_virtualCamThread = new VirtualCameraThread(src);
    connect(m_virtualCamThread, &VirtualCameraThread::virtualCamDisconnected, this, &VideoRehabSetupWidget::virtualCameraDisconnected);
    m_virtualCamThread->start();
}

void VideoRehabSetupWidget::stopVirtualCamera()
{
    if (m_virtualCamThread){
        m_virtualCamThread->quit();
        m_virtualCamThread->wait();
        m_virtualCamThread->deleteLater();
        m_virtualCamThread = nullptr;
    }
}

void VideoRehabSetupWidget::refreshWebpageSettings()
{

    if (!m_webPage->getSharedObject()->isPageReady())
        return;

    // Set PTZ capabilities
    bool ptz = ui->widgetSetup->getFieldValue("camera_ptz").toBool();
    m_webPage->getSharedObject()->setPTZCapabilities(ptz, ptz, ptz);
    m_webPage->getSharedObject()->sendPTZCapabilities();

    // Update video source
    QString video_src = ui->widgetSetup->getFieldValue("camera").toString();
    //qDebug() << "Setting Video Src to " << video_src;
    m_webPage->getSharedObject()->setCurrentCameraName(video_src);
    m_webPage->getSharedObject()->sendCurrentVideoSource();

    // Update audio source
    QString audio_src = ui->widgetSetup->getFieldValue("audio").toString();
    //qDebug() << "Setting Audio Src to " << audio_src;
    m_webPage->getSharedObject()->setCurrentAudioSrcName(audio_src);
    m_webPage->getSharedObject()->sendCurrentAudioSource();

    // Update mirror
    bool mirror = ui->widgetSetup->getFieldValue("mirror").toBool();
    m_webPage->getSharedObject()->setLocalMirror(mirror);
    m_webPage->getSharedObject()->getLocalMirror();


}

void VideoRehabSetupWidget::webPageLoaded(bool ok)
{
    if (!ok){
        LOG_ERROR(tr("Impossible de charger la page de prévisualisation de la caméra"), "VideoRehabSetupWidget::webPageLoaded");
        return;
    }
}

void VideoRehabSetupWidget::webPageReady()
{
    refreshWebpageSettings();

}

void VideoRehabSetupWidget::webPageCameraChanged()
{
    setLoading(false);
}

void VideoRehabSetupWidget::webPageVideoError(QString context, QString error)
{
    showError(tr("Problème vidéo"), context, error);
}

void VideoRehabSetupWidget::webPageAudioError(QString context, QString error)
{
    showError(tr("Problème audio"), context, error);
}

void VideoRehabSetupWidget::webPageGeneralError(QString context, QString error)
{
    showError(tr("Erreur"), context, error);
}

void VideoRehabSetupWidget::processServiceConfigsReply(QList<TeraData> configs, QUrlQuery query)
{
    Q_UNUSED(query)

    // Update values from config
    if (!configs.isEmpty()){
        // Only use the first returned config
        TeraData config = configs.first();
        if (config.getFieldValue("id_user") == m_comManager->getCurrentUser().getId()){
            /*QVariantMap config_values = config.getFieldValue("service_config_config").toMap();

            if (config_values.contains("camera")){
                selectVideoSrcByName(config_values["camera"].toString());
            }

            if (config_values.contains("audio")){
                selectAudioSrcByName(config_values["audio"].toString());
            }*/
            ui->widgetSetup->fillFormFromData(config.toJson("service_config_config"));

            m_id_service_config = config.getId();
        }
    }else{
        m_id_service_config = 0;
    }

    // Load page
    if (m_webEngine && m_webPage){
        if (m_webEngine->url().isEmpty()){
            // Set initial video source
            m_webPage->getSharedObject()->setCurrentCameraName(ui->widgetSetup->getFieldValue("camera").toString());

            // Load page
            m_webEngine->setUrl(QUrl("qrc:/VideoRehabService/html/index.html"));
        }
    }

    //refreshWebpageSettings();

}

void VideoRehabSetupWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type.contains(WEB_FORMS_QUERY_SERVICE_CONFIG)){
        ui->widgetSetup->buildUiFromStructure(data);
        // Hide default fields
        on_btnAdvancedConfig_clicked();

        // Query current service configuration
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_USER, QString::number(m_comManager->getCurrentUser().getId()));
        args.addQueryItem(WEB_QUERY_SERVICE_KEY, "VideoRehabService");
        args.addQueryItem(WEB_QUERY_ID_SPECIFIC, Utils::getMachineUniqueId());
        m_comManager->doQuery(WEB_SERVICECONFIGINFO_PATH, args);
    }
}

void VideoRehabSetupWidget::on_btnRefresh_clicked()
{
    if (m_webEngine){
        setLoading(true);
        m_webEngine->reload();
    }
}

void VideoRehabSetupWidget::on_btnAdvancedConfig_clicked()
{
    // Hide - show fields
    if (ui->btnAdvancedConfig->isChecked()){
        ui->widgetSetup->showField("camera_ptz");
        ui->widgetSetup->showField("camera2");
    }else{
        ui->widgetSetup->hideField("camera_ptz");
        ui->widgetSetup->hideField("camera2");
    }
}

void VideoRehabSetupWidget::on_btnSaveConfig_clicked()
{
    QJsonDocument service_config_data = ui->widgetSetup->getFormDataJson();
    QJsonObject service_config_obj;// = service_config_data.object();
    QJsonObject service_config_base = {
        {WEB_QUERY_ID_SPECIFIC,  Utils::getMachineUniqueId()},
        {WEB_QUERY_ID_USER, m_comManager->getCurrentUser().getId()},
        {WEB_QUERY_ID_SERVICE_CONFIG, m_id_service_config},
        {WEB_QUERY_SERVICE_KEY, "VideoRehabService"}
    };
    service_config_base.insert("service_config_config", service_config_data.object()["service_config_config"]);

    service_config_obj.insert("service_config", service_config_base);
    service_config_data.setObject(service_config_obj);

    m_comManager->doPost(WEB_SERVICECONFIGINFO_PATH, service_config_data.toJson());
}

void VideoRehabSetupWidget::setupFormDirtyChanged(bool dirty)
{
    ui->btnSaveConfig->setEnabled(dirty);
}

void VideoRehabSetupWidget::setupFormValueChanged(QWidget *wdg, QVariant value)
{
    if (m_valueJustChanged){
        m_valueJustChanged = false;
        return;
    }

    // OpenTeraCam camera source
    if (wdg == ui->widgetSetup->getWidgetForField("teracam_src")){
        VideoRehabVirtualCamSetupDialog dlg(ui->widgetSetup->getFieldValue("teracam_src").toString());
        m_valueJustChanged = true;
        if (dlg.exec() == QDialog::Accepted){
            ui->widgetSetup->setFieldValue("teracam_src", dlg.getCurrentSource());
            startVirtualCamera(dlg.getCurrentSource());
        }else{
            dynamic_cast<QLineEdit*>(ui->widgetSetup->getWidgetForField("teracam_src"))->undo();
        }
    }

    // Camera source
    if (wdg == ui->widgetSetup->getWidgetForField("camera")){
        if (value.toString().contains("OpenTeraCam")){
            QString src = ui->widgetSetup->getFieldValue("teracam_src").toString();
            if (!src.isEmpty()){
                startVirtualCamera(src);
            }
        }else{
            if (m_virtualCamThread)
                stopVirtualCamera();
        }
    }



    refreshWebpageSettings();
}

void VideoRehabSetupWidget::virtualCameraDisconnected()
{
    showError(tr("Erreur de caméra"), "VideoRehabSetupWidget::virtualCameraDisconnected", tr("Impossible de se connecter à la source vidéo."), true);
    stopVirtualCamera();

}
