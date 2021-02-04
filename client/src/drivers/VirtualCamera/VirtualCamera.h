#ifndef VIRTUALCAMERA_H
#define VIRTUALCAMERA_H

#include <QObject>
#include <QSharedPointer>
#include <QVariant>
#include <ak.h>
#include <akelement.h>
#include <akpacket.h>


class VirtualCamera : public QObject
{
    Q_OBJECT
public:
    explicit VirtualCamera(QObject *parent = nullptr);

    virtual ~VirtualCamera();

    bool init(const QString &source/*, const QString &device*/);
    bool start();
    bool stop();

signals:
    void virtualCamError(const QString &message);
    void virtualCamDisconnected();

private:
    AkElementPtr m_multiSrcPtr;
    AkElementPtr m_virtualCameraPtr;
    AkElementPtr m_desktopCapturePtr;

    int m_retryCounts;

    void printSignals(QObject* obj);

private slots:

    void virtualCamStateChanged(AkElement::ElementState state);
    void virtualCamReconnecting();
    void virtualCamConnected();
};

#endif // VIRTUALCAMERA_H
