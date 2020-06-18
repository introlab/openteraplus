#include "VideoRehabWebPage.h"

VideoRehabWebPage::VideoRehabWebPage(QObject *parent): QWebEnginePage(parent)
{

    connect(this, &VideoRehabWebPage::featurePermissionRequested, this, &VideoRehabWebPage::featurePermissionHandler);
}

bool VideoRehabWebPage::certificateError(const QWebEngineCertificateError &certificateError)
{
    // TODO: Remove in production

    qDebug() << "Certificate error: " << certificateError.errorDescription();
    /*
    The certificateError parameter contains information about the certificate and details of the error.
    Return true to ignore the error and complete the request. Return false to stop loading the request.
    */

    return true;
}

void VideoRehabWebPage::featurePermissionHandler(const QUrl &securityOrigin, QWebEnginePage::Feature feature)
{
    // TODO: Only allow specific features like webcam, micro, screenshare?

    //Grant all features!
    this->setFeaturePermission(securityOrigin,feature,QWebEnginePage::PermissionGrantedByUser);
}
