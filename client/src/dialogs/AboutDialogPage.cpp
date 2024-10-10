#include "AboutDialogPage.h"
#include <QWebEngineCertificateError>
#include <QDesktopServices>

AboutDialogPage::AboutDialogPage()
{
    //TODO Do something about certificate errors.
    connect(this, &QWebEnginePage::certificateError, this, &AboutDialogPage::onCertificateError);
}

void AboutDialogPage::onCertificateError(const QWebEngineCertificateError &certificateError)
{
    //TODO do Something about certificates
    qDebug() << "Certificate error: " << certificateError.description();
    //TODO Do not accept certificates in production ?
    auto mutableError = const_cast<QWebEngineCertificateError&>(certificateError);
    mutableError.acceptCertificate();
}

bool AboutDialogPage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    if (type == NavigationTypeLinkClicked){
        // Open links in external browser
        QDesktopServices::openUrl(url);

        return false;
    }

    return true;
}
