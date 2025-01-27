#ifndef ABOUTDIALOGPAGE_H
#define ABOUTDIALOGPAGE_H

#include <QWebEnginePage>
#include <QObject>

class AboutDialogPage : public QWebEnginePage
{
    Q_OBJECT
public:
    AboutDialogPage();

protected slots:

    void onCertificateError(const QWebEngineCertificateError &certificateError);

    // QWebEnginePage interface
protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override;
};

#endif // ABOUTDIALOGPAGE_H
