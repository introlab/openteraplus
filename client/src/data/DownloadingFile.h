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

    QString getAssociatedUuid();
    void setAssociatedUuid(const QString& uuid);

private:

    QString         m_associatedUuid;


signals:


private slots:
    void onDownloadDataReceived();

};

#endif // DOWNLOADEDFILE_H
