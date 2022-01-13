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

    QString getFullFilename();

    qint64 totalBytes();
    qint64 currentBytes();

    QString getLastError();

    virtual void setNetworkReply(QNetworkReply* reply);
    virtual void abortTransfer();

protected:
    QString         m_filepath;
    QString         m_filename;
    QNetworkReply*  m_reply;
    QFile           m_file;

    qint64          m_totalBytes;
    qint64          m_currentBytes;

    QString         m_lastError;

signals:
    void transferComplete(TransferringFile* transferred_file);

private slots:
    void onTransferCompleted();
};

#endif // TRANSFERRINGFILE_H
