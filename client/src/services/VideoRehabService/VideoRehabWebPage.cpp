#include "VideoRehabWebPage.h"

VideoRehabWebPage::VideoRehabWebPage(QObject *parent): QWebEnginePage(parent)
{
    // Create shared object for communication with webpage
    m_sharedObject = new SharedObject(this);

    // Create websocket for communication with page
    m_webSocketServer = new QWebSocketServer("localpage", QWebSocketServer::NonSecureMode, this);
    m_webSocketServer->listen(QHostAddress::LocalHost, 12345);

    m_clientWrapper = new WebSocketClientWrapper(m_webSocketServer,this);
    m_webChannel = new QWebChannel(this);

    // Connect signals
    connect(m_clientWrapper, &WebSocketClientWrapper::clientConnected, m_webChannel, &QWebChannel::connectTo); // Transport will be automatically connected
    connect(this, &VideoRehabWebPage::featurePermissionRequested, this, &VideoRehabWebPage::featurePermissionHandler);

    m_webChannel->registerObject(QStringLiteral("SharedObject"), m_sharedObject);
}

SharedObject *VideoRehabWebPage::getSharedObject() const
{
    return m_sharedObject;
}

bool VideoRehabWebPage::certificateError(const QWebEngineCertificateError &certificateError)
{
#ifdef QT_DEBUG

    qDebug() << "Certificate error: " << certificateError.errorDescription();
    /*
    The certificateError parameter contains information about the certificate and details of the error.
    Return true to ignore the error and complete the request. Return false to stop loading the request.
    */

    return true; // Accept all certificates in debug
#else
    // Refuse invalid certificates
    return false;
#endif
}

void VideoRehabWebPage::featurePermissionHandler(const QUrl &securityOrigin, QWebEnginePage::Feature feature)
{
    // TODO: Only allow specific features like webcam, micro, screenshare?

    //Grant all features!
    this->setFeaturePermission(securityOrigin,feature,QWebEnginePage::PermissionGrantedByUser);
}
