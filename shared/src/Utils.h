#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QRandomGenerator>
#include <QSysInfo>
#include <QSettings>
#include <QUuid>

class Utils : public QObject
{
    Q_OBJECT
public:
    explicit Utils(QObject *parent = nullptr);

    static QString generatePassword(const int &len);

    static QString getMachineUniqueId();

signals:

};

#endif // UTILS_H
