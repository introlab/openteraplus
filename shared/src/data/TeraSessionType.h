#ifndef TERASESSIONTYPE_H
#define TERASESSIONTYPE_H

#include "data/TeraData.h"
#include <QObject>

class TeraSessionType : public TeraData
{
    Q_OBJECT
public:
    explicit TeraSessionType(QObject *parent = nullptr);
    TeraSessionType(const TeraData& copy, QObject *parent=nullptr);

    QString getAssetsRelatedServiceUuid() const;

signals:
};

#endif // TERASESSIONTYPE_H
