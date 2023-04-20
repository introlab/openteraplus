#include "AboutDialogPage.h"
#include <QWebEngineCertificateError>

AboutDialogPage::AboutDialogPage()
{
    //TODO Do something about certificate errors.
}

void AboutDialogPage::onCertificateError(const QWebEngineCertificateError &certificateError)
{
    //TODO do Something about certificates
    qDebug() << "Certificate error: " << certificateError.description();
}
