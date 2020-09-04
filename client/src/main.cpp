/*
 * main.cpp
 *
 */
#include "ClientApp.h"
#include <QWebEngineSettings>
#include <QTranslator>
#include "VirtualCamera.h"

int main(int argc, char* argv[])
{
    // Global settings
    QCoreApplication::setOrganizationName("INTER");
    QCoreApplication::setApplicationName("OpenTeraPlus");



    // Force use of ANGLE instead of Desktop OpenGL - some memory leak may happen with WebRTC on Intel Graphics Cards otherwise.
#ifdef WIN32
    QApplication::setAttribute(Qt::AA_UseOpenGLES, true);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton, true);
#endif

#ifdef __APPLE__
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#endif
    ClientApp app(argc, argv);

//Set application style
#ifndef WIN32 // Don't set style on Windows - it creates some issues with combobox look.
    //app->setStyle("windows");
#endif
    //WebEngine default Settings
    /*QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);*/


    //DL - Testing Virtual camera driver, should be put elsewhere
    VirtualCamera cam;
    cam.init("rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mov", "/akvcam/video0");
    cam.start();


    return app.exec();
}
