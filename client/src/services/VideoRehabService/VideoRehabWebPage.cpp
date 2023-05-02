#include "VideoRehabWebPage.h"
#include "Logger.h"
#include <QWebEngineSettings>

VideoRehabWebPage::VideoRehabWebPage(QObject *parent): QWebEnginePage(parent)
{
    //Set page Settings
    auto settings = this->settings();
    settings->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    settings->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, true);
    settings->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, true);


    // Create shared object for communication with webpage
    m_sharedObject = new SharedObject(this);

    // Create websocket for communication with page
    m_webSocketServer = new QWebSocketServer("localpage", QWebSocketServer::NonSecureMode, this);
    if (!m_webSocketServer->listen(QHostAddress::LocalHost, 12345)){
        LOG_ERROR("Can't start WebSocket Server: " + m_webSocketServer->errorString(), "VideoRehabWebPage::VideoRehabWebPage");
        return;
    }

    m_clientWrapper = new WebSocketClientWrapper(m_webSocketServer,this);
    m_webChannel = new QWebChannel(this);

    // Connect signals
    connect(m_clientWrapper, &WebSocketClientWrapper::clientConnected, m_webChannel, &QWebChannel::connectTo); // Transport will be automatically connected
    connect(this, &VideoRehabWebPage::featurePermissionRequested, this, &VideoRehabWebPage::featurePermissionHandler);
    connect(this,&QWebEnginePage::certificateError, this, &VideoRehabWebPage::onCertificateError);

    m_webChannel->registerObject(QStringLiteral("SharedObject"), m_sharedObject);
}

VideoRehabWebPage::~VideoRehabWebPage()
{
    m_webSocketServer->close();
    delete m_webSocketServer;
}

SharedObject *VideoRehabWebPage::getSharedObject() const
{
    return m_sharedObject;
}

void VideoRehabWebPage::onCertificateError(const QWebEngineCertificateError &certificateError)
{

    //TODO do Something about certificates
    qDebug() << "Certificate error: " << certificateError.description();

    //TODO Do not accept certificates in production ?
    auto mutableError = const_cast<QWebEngineCertificateError&>(certificateError);
    mutableError.acceptCertificate();

}

void VideoRehabWebPage::featurePermissionHandler(const QUrl &securityOrigin, QWebEnginePage::Feature feature)
{
    // TODO: Only allow specific features like webcam, micro, screenshare?
    qDebug() << securityOrigin << " requesting: " << feature;

    //Grant all features!
    this->setFeaturePermission(securityOrigin,feature,QWebEnginePage::PermissionGrantedByUser);
}
