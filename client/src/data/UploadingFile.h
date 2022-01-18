#ifndef UPLOADINGFILE_H
#define UPLOADINGFILE_H

#include "TransferringFile.h"

#include "Logger.h"

#include <QObject>
#include <QFileInfo>

class UploadingFile : public TransferringFile
{
    Q_OBJECT
public:
    explicit UploadingFile(QObject *parent = nullptr);
    UploadingFile(const QString& file_name = QString(), QObject *parent = nullptr);
    UploadingFile(const UploadingFile& copy, QObject *parent=nullptr);

    ~UploadingFile();

    bool setFile(const QString& file_name);

    void setNetworkReply(QNetworkReply *reply) override;

private:

};

#endif // UPLOADINGFILE_H
