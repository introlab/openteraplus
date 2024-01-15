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
};

#endif // ABOUTDIALOGPAGE_H
