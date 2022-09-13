/*
 * main.cpp
 *
 */
#include "ClientApp.h"
#include <QWebEngineSettings>
#include <QTranslator>

int main(int argc, char* argv[])
{
    // Global settings
    QCoreApplication::setOrganizationName("INTER");
    QCoreApplication::setApplicationName("OpenTeraPlus");

#ifdef WIN32
    //QApplication::setAttribute(Qt::AA_UseOpenGLES, true); // Force use of ANGLE instead of Desktop OpenGL - some memory leak may happen with WebRTC on Intel Graphics Cards otherwise.
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton, true);
#endif

#ifdef __APPLE__
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#endif

#ifdef QT_DEBUG
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--autoplay-policy=no-user-gesture-required --remote-debugging-port=22222"); // Allow auto-play feature in webengine without any user interaction to test, for example, the microphone and play sounds
#else
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--autoplay-policy=no-user-gesture-required"); // Allow auto-play feature in webengine without any user interaction to test, for example, the microphone and play sounds
#endif
    //qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--autoplay-policy=no-user-gesture-required --remote-debugging-port=22222"); // Allow auto-play feature in webengine without any user interaction to test, for example, the microphone and play sounds
    ClientApp app(argc, argv);

//Set application style
#ifndef WIN32 // Don't set style on Windows - it creates some issues with combobox look.
    //app->setStyle("windows");
#endif
    //WebEngine default Settings
    /*QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);*/

    return app.exec();
}
