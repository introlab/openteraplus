#include "AboutDialogPage.h"
#include <QWebEngineCertificateError>

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
