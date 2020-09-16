#include "VirtualCamera.h"
#include <QDir>
#include <QDebug>

VirtualCamera::VirtualCamera(QObject *parent)
    : QObject(parent)
{
    Ak::registerTypes();

    //This is required and will initialize the IPC system on Windows
    Ak::setQmlEngine(nullptr);

    //Tell the library where to look for plugins
    //TODO Hardcoded for now...
    QStringList searchPaths;
    //searchPaths << QDir::currentPath() + "/AvKysPlugins";
    searchPaths << AVKYS_PLUGIN_DIRECTORY;
    AkElement::setSearchPaths(searchPaths);


    qDebug() << "MultiSrc submodules: " << AkElement::listSubModules("MultiSrc");
    qDebug() << "VirtualCamera submodules: "<< AkElement::listSubModules("VirtualCamera");
    qDebug() << "DesktopCapture submodules: "<< AkElement::listSubModules("DesktopCapture");

}

VirtualCamera::~VirtualCamera()
{
    stop();
}

bool VirtualCamera::init(const QString &source, const QString& device)
{

    m_virtualCameraPtr = AkElement::create("VirtualCamera");

    //Configure virtual camera
    if (m_virtualCameraPtr)
    {
        m_virtualCameraPtr->setProperty("media", device);
        m_virtualCameraPtr->setProperty("fps", "15.0");
        m_virtualCameraPtr->setProperty("swapRgb", false);
    }

    if (source.startsWith("screen"))
    {

        m_desktopCapturePtr = AkElement::create("DesktopCapture");

        if (m_virtualCameraPtr && m_desktopCapturePtr)
        {

            m_desktopCapturePtr->setProperty("media", source);
            //TODO change fps on demand?
            m_desktopCapturePtr->setProperty("fps", "10.0");

            //Connect to virtual camera sink
            m_desktopCapturePtr->link(m_virtualCameraPtr);
            return true;
        }
    }
    else
    {
        m_multiSrcPtr = AkElement::create("MultiSrc");

        if (m_virtualCameraPtr && m_multiSrcPtr)
        {
            //Set Parameters
            m_multiSrcPtr->setProperty("media", source);
            m_multiSrcPtr->setProperty("loop", false);
            m_multiSrcPtr->setProperty("showLog", true);

            //Connect to virtual camera sink
            m_multiSrcPtr->link(m_virtualCameraPtr);
            return true;
        }

    }

    //Something went wrong
    return false;
}

bool VirtualCamera::start()
{
    if (m_virtualCameraPtr && m_multiSrcPtr) {
        m_multiSrcPtr->setState(AkElement::ElementStatePlaying);
        m_virtualCameraPtr->setState(AkElement::ElementStatePlaying);
        return true;
    }
    else if (m_virtualCameraPtr && m_desktopCapturePtr) {
        m_desktopCapturePtr->setState(AkElement::ElementStatePlaying);
        m_virtualCameraPtr->setState(AkElement::ElementStatePlaying);
        return true;
    }

    return false;
}

bool VirtualCamera::stop()
{
    if (m_virtualCameraPtr && m_multiSrcPtr) {
        m_multiSrcPtr->setState(AkElement::ElementStatePaused);
        m_virtualCameraPtr->setState(AkElement::ElementStatePaused);
        return true;
    }
    else if (m_virtualCameraPtr && m_desktopCapturePtr) {
        m_desktopCapturePtr->setState(AkElement::ElementStatePaused);
        m_virtualCameraPtr->setState(AkElement::ElementStatePaused);
        return true;
    }

    return false;
}
