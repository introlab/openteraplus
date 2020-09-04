#ifndef VIRTUALCAMERA_H
#define VIRTUALCAMERA_H

#include <QObject>
#include <ak.h>
#include <akelement.h>
#include <akpacket.h>


class VirtualCamera : public QObject
{
    Q_OBJECT
public:
    explicit VirtualCamera(QObject *parent = nullptr);

    bool init(const QString device);
    bool start();
    bool stop();

signals:

};

#endif // VIRTUALCAMERA_H
