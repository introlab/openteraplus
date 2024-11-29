#ifndef WEBPAGEREQUESTINTERCEPTOR_H
#define WEBPAGEREQUESTINTERCEPTOR_H

#include <QObject>
#include <QWebEngineUrlRequestInterceptor>

class WebPageRequestInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
public:
    WebPageRequestInterceptor(bool inject_client = false, QObject *p = nullptr);
    ~WebPageRequestInterceptor();
    void interceptRequest(QWebEngineUrlRequestInfo &info) override;
private:
    QString m_osName;
    QString m_osVersion;
    bool    m_injectClient = false;
};

#endif // WEBPAGEREQUESTINTERCEPTOR_H
