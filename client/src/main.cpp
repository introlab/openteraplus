/*
 * main.cpp
 *
 */
#include "ClientApp.h"
#include <QWebEngineSettings>

int main(int argc, char* argv[])
{
    int rval;

    //QApplication::setAttribute(Qt::AA_EnableHighDpiScaling,true);
    ClientApp* app = new ClientApp(argc, argv);

    // Force use of ANGLE instead of Desktop OpenGL - some memory leak may happen with WebRTC on Intel Graphics Cards otherwise.
    app->setAttribute(Qt::AA_UseOpenGLES, true);

    //WebEngine default Settings
    /*QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);*/

    rval = app->exec();

    delete app;

    return rval;
}
