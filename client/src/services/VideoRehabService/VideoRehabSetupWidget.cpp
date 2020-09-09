#include "VideoRehabSetupWidget.h"
#include "ui_VideoRehabSetupWidget.h"

VideoRehabSetupWidget::VideoRehabSetupWidget(ComManager *comManager, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoRehabSetupWidget),
    m_comManager(comManager)
{
    ui->setupUi(this);

    initUI();
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
    //Create a new page
    m_webPage = new VideoRehabWebPage(m_webEngine);

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

    m_webEngine->setUrl(QUrl("qrc:/VideoRehabService/html/index.html"));
}
