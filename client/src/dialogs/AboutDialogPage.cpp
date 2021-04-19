#include "AboutDialogPage.h"

AboutDialogPage::AboutDialogPage()
{

}

bool AboutDialogPage::certificateError(const QWebEngineCertificateError &certificateError)
{
    Q_UNUSED(certificateError)

    return true; // Accept all certificates
}
