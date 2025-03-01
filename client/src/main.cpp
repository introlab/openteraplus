/*
 * main.cpp
 *
 */
#include "ClientApp.h"
//#include <QWebEngineSettings>
#include <QTranslator>

int main(int argc, char* argv[])
{
    // Global settings
    QCoreApplication::setOrganizationName("INTER");
    QCoreApplication::setApplicationName("OpenTeraPlus");
#ifdef QT_DEBUG
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--autoplay-policy=no-user-gesture-required --remote-debugging-port=22222"); // Allow auto-play feature in webengine without any user interaction to test, for example, the microphone and play sounds
#else
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--autoplay-policy=no-user-gesture-required"); // Allow auto-play feature in webengine without any user interaction to test, for example, the microphone and play sounds
#endif
    //qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--autoplay-policy=no-user-gesture-required --remote-debugging-port=22222"); // Allow auto-play feature in webengine without any user interaction to test, for example, the microphone and play sounds
    ClientApp app(argc, argv);

//Set application style
#ifdef WIN32
    app.setStyle("windows"); // Force use of "windows" style, not windows 11 one if on that OS, since it has some issues right now
#endif

    //WebEngine default Settings
    //QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    //QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    //QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    int rval = app.exec();
    //qDebug() << "Bye!";
    return rval;
}
