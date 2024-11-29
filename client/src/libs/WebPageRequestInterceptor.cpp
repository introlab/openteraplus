#include "WebPageRequestInterceptor.h"
#include "Utils.h"

WebPageRequestInterceptor::WebPageRequestInterceptor(bool inject_client, QObject *p) :
    QWebEngineUrlRequestInterceptor(p)
    ,m_injectClient(inject_client)
{
    // Cache OS information
    if (m_injectClient){
        m_osName = Utils::getOsName();
        m_osVersion = Utils::getOsVersion();
    }
}

WebPageRequestInterceptor::~WebPageRequestInterceptor()
{

}

void WebPageRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    // Inject client name and version
    if (m_injectClient){
        info.setHttpHeader("X-Client-Name", QByteArray(OPENTERAPLUS_CLIENT_NAME));
        info.setHttpHeader("X-Client-Version", QByteArray(OPENTERAPLUS_VERSION));
        info.setHttpHeader("X-OS-Name", m_osName.toUtf8());
        info.setHttpHeader("X-OS-Version", m_osVersion.toUtf8());
    }

    // Inject required language
    QString localeString = QLocale().bcp47Name();
    //qDebug() << "localeString : " << localeString;
    info.setHttpHeader(QByteArray("Accept-Language"), localeString.toUtf8());
}
