#include "VirtualCameraThread.h"

VirtualCameraThread::VirtualCameraThread(QString camera_src)
{
    m_cameraSrc = camera_src;

    //qDebug() << "VirtualCameraThread: " << thread();
}

VirtualCameraThread::~VirtualCameraThread()
{
    qDebug() << "~VirtualCameraThread";
    if (m_virtualCam)
        delete m_virtualCam;
}

void VirtualCameraThread::run()
{
    //qDebug() << "VirtualCameraThread::run " << thread();
    m_virtualCam = new VirtualCamera();

    // Signals
    connect(m_virtualCam, &VirtualCamera::virtualCamDisconnected, this, &VirtualCameraThread::virtualCamDisconnected);
    connect(m_virtualCam, &VirtualCamera::virtualCamConnected, this, &VirtualCameraThread::virtualCamConnected);
    connect(m_virtualCam, &VirtualCamera::virtualCamError, this, &VirtualCameraThread::virtualCamError);

    m_virtualCam->init(m_cameraSrc);
    m_virtualCam->start();

    exec();

    delete m_virtualCam;
    m_virtualCam = nullptr;
}
