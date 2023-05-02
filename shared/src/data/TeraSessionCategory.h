#ifndef TERASESSIONCATEGORY_H
#define TERASESSIONCATEGORY_H

#include <QObject>
#include "SharedLib.h"

class SHAREDLIB_EXPORT TeraSessionCategory : public QObject
{
    Q_OBJECT
public:
    enum SessionTypeCategories{
        SESSION_TYPE_UNKNOWN = 0,
        SESSION_TYPE_SERVICE = 1,
        SESSION_TYPE_DATACOLLECT = 2,
        SESSION_TYPE_FILETRANSFER = 3,
        SESSION_TYPE_PROTOCOL = 4
    };

    explicit TeraSessionCategory(QObject *parent = nullptr);

    static QString getSessionTypeCategoryName(const SessionTypeCategories& category);
    static QString getSessionTypeCategoryName(const int& category);

signals:

};

#endif // TERASESSIONCATEGORY_H
