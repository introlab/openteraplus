#include "VirtualCamera.h"
#include <QDir>

VirtualCamera::VirtualCamera(QObject *parent)
    : QObject(parent)
{
    Ak::registerTypes();

    //This is required and will initialize the IPC system on Windows
    Ak::setQmlEngine(nullptr);

    //Tell the library where to look for plugins
    //TODO Hardcoded for now...
    QStringList searchPaths;
    searchPaths << QDir::currentPath() + "/AvKysPlugins";
    AkElement::setSearchPaths(searchPaths);
}

bool VirtualCamera::init(const QString device)
{
    return false;
}

bool VirtualCamera::start()
{
    return false;
}

bool VirtualCamera::stop()
{
    return false;
}
