#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <akmultimediasourceelement.h>

int main(int argc, char* argv[])
{

    QCoreApplication app(argc,argv);

    qDebug() << "Starting...";
    //Why do we need to do that?
    qRegisterMetaType<AkPacket>("AkPacket");

    //Tell the library where to look for plugins
    //TODO Hardcoded for now...
    QStringList searchPaths;
    searchPaths << QCoreApplication::applicationDirPath() + "/AvKysPlugins";

    AkElement::setSearchPaths(searchPaths);

    qDebug() << "VirtualCamera submodules: "<< AkElement::listSubModules("VirtualCamera");

    //This should load the library. Let's see...

    auto VirtualCameraPtr = AkElement::create("VirtualCamera");

    if (VirtualCameraPtr)
    {

        qDebug() << "Setting driver Path";
        QString appPath = QCoreApplication::applicationDirPath().replace("/","\\");
        VirtualCameraPtr->setProperty("driverPath",appPath + "\\VirtualCameraSource.dll");

        qDebug() << "PATH : " << VirtualCameraPtr->property("driverPath");
        //Install camera
        QMetaObject::invokeMethod(VirtualCameraPtr.data(),"createWebcam",Q_ARG(QString,"TeraCam"),Q_ARG(QString,""));

        return 0;
    }

    return -1;
}
