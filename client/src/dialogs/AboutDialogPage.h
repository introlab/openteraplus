#ifndef ABOUTDIALOGPAGE_H
#define ABOUTDIALOGPAGE_H

#include <QWebEnginePage>
#include <QObject>

class AboutDialogPage : public QWebEnginePage
{
    Q_OBJECT
public:
    AboutDialogPage();

protected:
    virtual bool certificateError(const QWebEngineCertificateError &certificateError) override;
};

#endif // ABOUTDIALOGPAGE_H
