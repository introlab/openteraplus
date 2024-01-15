#ifndef AUDIOVIDEOUTILS_H
#define AUDIOVIDEOUTILS_H

#include <QObject>

#ifdef Q_OS_WINDOWS
    #include <strmif.h>
    #include <initguid.h>
    #include <uuids.h>
    #include <vfwmsgs.h>
    DEFINE_GUID(CLSID_VirtualCameraSource, 0x41764b79, 0x7320, 0x5669, 0x72, 0x74, 0x75, 0x61, 0x6c, 0x43, 0x61, 0x6d);
#endif

class AudioVideoUtils: public QObject
{
    Q_OBJECT

public:
    explicit AudioVideoUtils(QObject *parent = nullptr);

    static QStringList getAudioDeviceNames();
    static QStringList getVideoDeviceNames();

private:
#ifdef Q_OS_WINDOWS
    static QStringList getVirtualCameras();
    static HRESULT enumerateCameras(IEnumMoniker **ppEnum);
#endif
};

#endif // AUDIOVIDEOUTILS_H
