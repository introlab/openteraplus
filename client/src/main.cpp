/*
 * main.cpp
 *
 */
#include "ClientApp.h"
#include <QWebEngineSettings>
#include <QTranslator>

int main(int argc, char* argv[])
{
    int rval;





    // Force use of ANGLE instead of Desktop OpenGL - some memory leak may happen with WebRTC on Intel Graphics Cards otherwise.
#ifdef WIN32
    QApplication::setAttribute(Qt::AA_UseOpenGLES, true);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton, true);
#endif

#ifdef __APPLE__
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#endif
    ClientApp* app = new ClientApp(argc, argv);

//Set application style
#ifndef WIN32 // Don't set style on Windows - it creates some issues with combobox look.
    //app->setStyle("windows");
#endif


    //WebEngine default Settings
    /*QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);*/

    //Translations...
     QTranslator translator;
    // look up e.g. :/translations/myapp_de.qm
     qDebug() << QLocale();
    if (translator.load(QLocale(), QLatin1String("openteraplus"), QLatin1String("_"), QLatin1String(":/translations")))
        QCoreApplication::installTranslator(&translator);



    rval = app->exec();

    delete app;

    return rval;
}
