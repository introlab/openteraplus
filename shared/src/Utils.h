#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QRandomGenerator>
#include <QSysInfo>
#include <QSettings>
#include <QUuid>
#include <QAudioDeviceInfo>
#include <QCameraInfo>

class Utils : public QObject
{
    Q_OBJECT
public:
    explicit Utils(QObject *parent = nullptr);

    static QString generatePassword(const int &len);

    static QString getMachineUniqueId();

    static QStringList getAudioDeviceNames();
    static QStringList getVideoDeviceNames();

signals:

};

#endif // UTILS_H
