#ifndef DOWNLOADEDFILE_H
#define DOWNLOADEDFILE_H

#include <QObject>
#include <QNetworkReply>
#include <QFile>
#include <QDir>

#include "TransferringFile.h"

class DownloadingFile : public TransferringFile
{
    Q_OBJECT
public:
    explicit DownloadingFile(QObject *parent = nullptr);
    DownloadingFile(const QString& save_path, const QString& file_name = QString(), QObject *parent = nullptr);
    DownloadingFile(const DownloadingFile& copy, QObject *parent=nullptr);

    DownloadingFile &operator = (const DownloadingFile& other);

    void setNetworkReply(QNetworkReply* reply);
    void abortTransfer();

private:


signals:
    void downloadProgress(DownloadingFile* downloaded_file);

private slots:
    void onDownloadDataReceived();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

};

#endif // DOWNLOADEDFILE_H
