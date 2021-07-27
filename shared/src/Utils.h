#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QRandomGenerator>
#include <QSysInfo>
#include <QSettings>
#include <QUuid>

#ifndef WEBASSEMBLY_COMPILATION
    #include <QAudioDeviceInfo>
    #include <QCameraInfo>
#endif

class Utils : public QObject
{
    Q_OBJECT
public:
    enum PasswordValidationErrors{
        PASSWORD_LENGTH = 1,
        PASSWORD_NOCAPS = 2,
        PASSWORD_NONOCAPS = 3,
        PASSWORD_NODIGITS = 4,
        PASSWORD_NOCHAR = 5
    };

    explicit Utils(QObject *parent = nullptr);

    static QString generatePassword(const int &len);
    static QList<PasswordValidationErrors> validatePassword(const QString& password);

    static QString getMachineUniqueId();

    static QStringList getAudioDeviceNames();
    static QStringList getVideoDeviceNames();

    static void inStringUnicodeConverter(QString* str);

    static QString removeAccents(QString s);

    static bool isNewerVersion(QString version);

signals:

};

#endif // UTILS_H
