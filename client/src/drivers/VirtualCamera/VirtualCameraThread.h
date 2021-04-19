#ifndef VIRTUALCAMERATHREAD_H
#define VIRTUALCAMERATHREAD_H

#include <QThread>
#include "VirtualCamera.h"


class VirtualCameraThread : public QThread
{
    Q_OBJECT
public:
    VirtualCameraThread(QString camera_src);
    ~VirtualCameraThread();

protected:
    void run() override;

private:
    VirtualCamera* m_virtualCam;

    QString m_cameraSrc;

signals:
    void virtualCamConnected();
    void virtualCamDisconnected();
    void virtualCamError(QString message);
};

#endif // VIRTUALCAMERATHREAD_H
