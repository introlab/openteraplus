#ifndef TERAASSET_H
#define TERAASSET_H

#include <QObject>
// #include <QMimeType>
// #include <QMimeDatabase>

class TeraAsset : public QObject
{
    Q_OBJECT
public:
    explicit TeraAsset(QObject *parent = nullptr);

    static QString getIconForContentType(const QString& content_type);

signals:

};

#endif // TERAASSET_H
