#include "VirtualCamera.h"
#include <QDir>
#include <QDebug>
#include <QCoreApplication>

VirtualCamera::VirtualCamera(QObject *parent)
    : QObject(parent)
{
    qDebug() << "** Creating VirtualCamera on thread" << thread();

    //Ak::registerTypes();
    qRegisterMetaType<AkPacket>("AkPacket");

    m_retryCounts = 0;
    m_running = false;

    //This is required and will initialize the IPC system on Windows
    //Ak::setQmlEngine(nullptr);

    //Tell the library where to look for plugins
    //TODO Hardcoded for now...
    QStringList searchPaths;
    //searchPaths << QDir::currentPath() + "/AvKysPlugins";
    searchPaths << QCoreApplication::applicationDirPath() + "/AvKysPlugins";
    searchPaths << AVKYS_PLUGIN_DIRECTORY;
    /*QDir dir;
    if (!dir.exists(AVKYS_PLUGIN_DIRECTORY)){
        qDebug() << "Setting AVKYS plugin directory to: " +  QDir::currentPath() + "/AvKysPlugins";
        searchPaths <<  QDir::currentPath() + "/AvKysPlugins";
    }*/

    AkElement::setSearchPaths(searchPaths);


    //qDebug() << "MultiSrc submodules: " << AkElement::listSubModules("MultiSrc");
    //qDebug() << "VirtualCamera submodules: "<< AkElement::listSubModules("VirtualCamera");
    //qDebug() << "DesktopCapture submodules: "<< AkElement::listSubModules("DesktopCapture");

}

VirtualCamera::~VirtualCamera()
{
    qDebug() << "** Stopping VirtualCamera on thread " << thread();

    /*if (m_running)
        stop();*/
}

bool VirtualCamera::init(const QString &source/*, const QString& device*/)
{

    m_virtualCameraPtr = AkElement::create("VirtualCamera");

    //Configure virtual camera
    if (m_virtualCameraPtr)
    {
        //m_virtualCameraPtr->setProperty("media", device);
        m_virtualCameraPtr->setProperty("fps", "30.0");
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
            m_multiSrcPtr->setProperty("showLog", false);

            // Connect signals
            QObject::connect(m_multiSrcPtr.data(), SIGNAL(error(QString)), this, SIGNAL(virtualCamError(QString)));
            QObject::connect(m_multiSrcPtr.data(), SIGNAL(reconnectingStream()), this, SLOT(virtualCamReconnecting()));
            QObject::connect(m_multiSrcPtr.data(), SIGNAL(streamConnected()), this, SLOT(virtualCamStreamConnected()));
            //connect(m_multiSrcPtr.get(), &AkElement::stateChanged, this, &VirtualCamera::virtualCamStateChanged);

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
    m_retryCounts = 0;
    m_running = true;

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
    m_running = false;
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

void VirtualCamera::printSignals(QObject *obj)
{
    const QMetaObject *moTest = obj->metaObject();
    for(int methodIdx = moTest->methodOffset(); methodIdx < moTest->methodCount(); ++methodIdx) {
      QMetaMethod mmTest = moTest->method(methodIdx);
      switch((int)mmTest.methodType()) {
        case QMetaMethod::Signal:
          qDebug() << "Signal: " << (QString(mmTest.methodSignature())); // Requires Qt 5.0 or newer
          break;
        case QMetaMethod::Slot:
          qDebug() << "Slot: " << (QString(mmTest.methodSignature())); // Requires Qt 5.0 or newer
          break;
      }
    }
}

void VirtualCamera::virtualCamStreamConnected()
{
    qDebug() << "VirtualCamera - got stream!";
    m_retryCounts = 0;
    emit virtualCamConnected();
}

void VirtualCamera::virtualCamStateChanged(AkElement::ElementState state)
{
    qDebug() << "VirtualCamera State Changed: " << state;
}

void VirtualCamera::virtualCamReconnecting()
{
    m_retryCounts++;

    if (m_retryCounts >= 3){
        qDebug() << "Aborting... ";
        //stop();
        emit virtualCamDisconnected();

        return;
    }
    qDebug() << "VirtualCamera reconnecting...";
}
