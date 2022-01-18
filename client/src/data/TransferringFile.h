#ifndef TRANSFERRINGFILE_H
#define TRANSFERRINGFILE_H

#include <QObject>
#include <QNetworkReply>
#include <QFile>

class TransferringFile : public QObject
{
    Q_OBJECT
public:
    explicit TransferringFile(QObject *parent = nullptr);
    TransferringFile(const QString& file_path, const QString& file_name, QObject *parent = nullptr);
    TransferringFile(const TransferringFile& copy, QObject *parent=nullptr);

    TransferringFile &operator = (const TransferringFile& other);

    QString getFileName();
    QString getFilePath();
    QString getFullFilename();

    QFile* getFile();

    qint64 totalBytes();
    qint64 currentBytes();

    QString getLastError();

    virtual void setNetworkReply(QNetworkReply* reply);
    QNetworkReply* getNetworkReply();

    virtual void abortTransfer();

protected:
    QString         m_filepath;
    QString         m_filename;
    QNetworkReply*  m_reply;
    QFile           m_file;

    qint64          m_totalBytes;
    qint64          m_currentBytes;

    QString         m_lastError;

private:
    bool            m_aborting;

signals:
    void transferComplete(TransferringFile* transferred_file);
    void transferProgress(TransferringFile* transferred_file);
    void transferAborted(TransferringFile* transferred_file);

protected slots:
    void onTransferCompleted();
    void onTransferProgress(qint64 bytesReceived, qint64 bytesTotal);
};

#endif // TRANSFERRINGFILE_H
