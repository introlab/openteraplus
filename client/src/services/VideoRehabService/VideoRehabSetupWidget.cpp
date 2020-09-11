#include "VideoRehabSetupWidget.h"
#include "ui_VideoRehabSetupWidget.h"

#include "Utils.h"

VideoRehabSetupWidget::VideoRehabSetupWidget(ComManager *comManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoRehabSetupWidget),
    m_comManager(comManager)
{
    ui->setupUi(this);

    setLoading(true); // Disable until page is fully loaded

    initUI();
    connectSignals();

    // Query current service configuration
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_USER, QString::number(m_comManager->getCurrentUser().getId()));
    args.addQueryItem(WEB_QUERY_SERVICE_KEY, "VideoRehabService");
    args.addQueryItem(WEB_QUERY_ID_SPECIFIC, Utils::getMachineUniqueId());
    m_comManager->doQuery(WEB_SERVICECONFIGINFO_PATH, args);
}

VideoRehabSetupWidget::~VideoRehabSetupWidget()
{
    delete ui;

    m_webPage->deleteLater();
    m_webEngine->deleteLater();
}

void VideoRehabSetupWidget::initUI()
{
    //// Local device enumeration
    QList<QAudioDeviceInfo> audio_devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    foreach(QAudioDeviceInfo input, audio_devices){
        ui->cmbAudioSrc->addItem(input.deviceName(), input.deviceName());
    }

    QList<QCameraInfo> video_devices = QCameraInfo::availableCameras();
    for (QCameraInfo camera:video_devices){
        //item_combo->addItem(camera.description(), camera.deviceName());
        ui->cmbVideoSrc->addItem(camera.description(), camera.description());
    }

    //// Web engine setup
    m_webEngine = new QWebEngineView(ui->wdgWebEngine);
    connect(m_webEngine, &QWebEngineView::loadFinished, this, &VideoRehabSetupWidget::webPageLoaded);

    //Create a new page
    m_webPage = new VideoRehabWebPage(m_webEngine);
    connect(m_webPage->getSharedObject(), &SharedObject::pageIsReady, this, &VideoRehabSetupWidget::webPageReady);
    connect(m_webPage->getSharedObject(), &SharedObject::currentCameraWasChanged, this, &VideoRehabSetupWidget::webPageCameraChanged);

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

    // UI updates to webpage
    connect(ui->cmbVideoSrc, &QComboBox::currentTextChanged, this, &VideoRehabSetupWidget::refreshWebpageSettings);
    connect(ui->cmbAudioSrc, &QComboBox::currentTextChanged, this, &VideoRehabSetupWidget::refreshWebpageSettings);
    connect(ui->chkMirror, &QCheckBox::stateChanged, this, &VideoRehabSetupWidget::refreshWebpageSettings);

}

void VideoRehabSetupWidget::selectVideoSrcByName(const QString &name)
{
    for (int i=0; i<ui->cmbVideoSrc->count(); i++){
        if (ui->cmbVideoSrc->itemText(i) == name){
            ui->cmbVideoSrc->setCurrentIndex(i);
            return;
        }
    }
    LOG_WARNING(tr("La source vidéo ") + name + tr(" n'a pas été trouvée sur ce système."), "VideoRehabSetupWidget::selectVideoSrcByName");
}

void VideoRehabSetupWidget::selectAudioSrcByName(const QString &name)
{
    for (int i=0; i<ui->cmbAudioSrc->count(); i++){
        if (ui->cmbAudioSrc->itemText(i) == name){
            ui->cmbAudioSrc->setCurrentIndex(i);
            return;
        }
    }
    LOG_WARNING(tr("La source audio ") + name + tr(" n'a pas été trouvée sur ce système."), "VideoRehabSetupWidget::selectAudioSrcByName");
}

void VideoRehabSetupWidget::setLoading(const bool &loading)
{
    setEnabled(!loading);
    ui->wdgWebEngine->setVisible(!loading);
    ui->lblLoading->setVisible(loading);
}

void VideoRehabSetupWidget::refreshWebpageSettings()
{

    if (!m_webPage->getSharedObject()->isPageReady())
        return;

    // Set PTZ capabilities
    // TODO: use real settings!
    m_webPage->getSharedObject()->setPTZCapabilities(false, false, false);
    m_webPage->getSharedObject()->sendPTZCapabilities();

    // Update video source
    qDebug() << "Setting Video Src to " << ui->cmbVideoSrc->currentText();
    m_webPage->getSharedObject()->setCurrentCameraName(ui->cmbVideoSrc->currentText());
    m_webPage->getSharedObject()->sendCurrentVideoSource();

    // Update mirror
    m_webPage->getSharedObject()->setLocalMirror(ui->chkMirror->isChecked());
    m_webPage->getSharedObject()->getLocalMirror();

    // TODO Audio?
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

void VideoRehabSetupWidget::processServiceConfigsReply(QList<TeraData> configs, QUrlQuery query)
{
    Q_UNUSED(query)

    // Update values from config
    if (!configs.isEmpty()){
        // Only use the first returned config
        TeraData config = configs.first();
        if (config.getFieldValue("id_user") == m_comManager->getCurrentUser().getId()){
            QVariantMap config_values = config.getFieldValue("service_config_config").toMap();

            if (config_values.contains("camera")){
                selectVideoSrcByName(config_values["camera"].toString());
            }

            if (config_values.contains("audio")){
                selectAudioSrcByName(config_values["audio"].toString());
            }

            // Load page
            if (m_webEngine && m_webPage){
                // Set initial video source
                m_webPage->getSharedObject()->setCurrentCameraName(ui->cmbVideoSrc->currentText());

                // Load page
                m_webEngine->setUrl(QUrl("qrc:/VideoRehabService/html/index.html"));
            }
        }
    }

    //refreshWebpageSettings();

}
