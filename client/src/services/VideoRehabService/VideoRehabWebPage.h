#ifndef VIDEOREHABWEBPAGE_H
#define VIDEOREHABWEBPAGE_H

#include <QObject>
#include <QWebEnginePage>
#include <QWebEngineCertificateError>

class VideoRehabWebPage : public QWebEnginePage
{
    Q_OBJECT
public:
    VideoRehabWebPage(QObject *parent = nullptr);

protected:
    virtual bool certificateError(const QWebEngineCertificateError &certificateError) override;

private slots:
    void featurePermissionHandler(const QUrl &securityOrigin, QWebEnginePage::Feature feature);
};

#endif // VIDEOREHABWEBPAGE_H
