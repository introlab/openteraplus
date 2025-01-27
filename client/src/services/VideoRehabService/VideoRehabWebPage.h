#ifndef VIDEOREHABWEBPAGE_H
#define VIDEOREHABWEBPAGE_H

#include <QObject>
#include <QWebEnginePage>
#include <QWebEngineCertificateError>
#include <QWebChannel>
#include <QWebSocketServer>
#include <QWebEngineDesktopMediaRequest>

#include "WebSocket/SharedObject.h"
#include "WebSocket/WebSocketClientWrapper.h"


class VideoRehabWebPage : public QWebEnginePage
{
    Q_OBJECT
public:
    VideoRehabWebPage(QObject *parent = nullptr);
    ~VideoRehabWebPage() override;

    SharedObject* getSharedObject() const;

protected:


private slots:
    void featurePermissionHandler(const QUrl &securityOrigin, QWebEnginePage::Feature feature);
    void onCertificateError(const QWebEngineCertificateError &certificateError);
    void onDesktopMediaRequest(const QWebEngineDesktopMediaRequest &request);

private:
    SharedObject            *m_sharedObject;           // Shared object for communication with page itself

    QWebSocketServer        *m_webSocketServer;    // Websocket server for communication with page
    WebSocketClientWrapper  *m_clientWrapper;
    QWebChannel             *m_webChannel;
};

#endif // VIDEOREHABWEBPAGE_H
