#ifndef UTILS_H
#define UTILS_H

#include "SharedLib.h"
#include <QObject>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSysInfo>
#include <QSettings>
#include <QUuid>
#include <QTime>

class SHAREDLIB_EXPORT Utils : public QObject
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

    static void inStringUnicodeConverter(QString* str);

    static QString removeAccents(const QString& s);
    static QString removeNonAlphanumerics(const QString& s);
    static QString toCamelCase(const QString& s);

    static bool isNewerVersion(QString version);

    static QString formatFileSize(const int &file_size);
    static QString formatDuration(const QString &duration);

signals:

};

#endif // UTILS_H
