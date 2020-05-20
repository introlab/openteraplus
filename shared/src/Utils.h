#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QRandomGenerator>

class Utils : public QObject
{
    Q_OBJECT
public:
    explicit Utils(QObject *parent = nullptr);

    static QString generatePassword(const int &len);

signals:

};

#endif // UTILS_H
