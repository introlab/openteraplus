#include "TransferringFile.h"

TransferringFile::TransferringFile(QObject *parent)
    : QObject{parent}
{
    m_reply = nullptr;
    m_status = TransferStatus::WAITING;
    m_aborting = false;
    m_totalBytes = 1;
    m_currentBytes = 0;
}

TransferringFile::TransferringFile(const QString &file_path, const QString &file_name, QObject *parent)
{
    m_filename = file_name;
    m_filepath = file_path;
    m_status = TransferStatus::WAITING;
    m_aborting = false;
    m_totalBytes = 1;
    m_currentBytes = 0;
}

TransferringFile::TransferringFile(const TransferringFile &copy, QObject *parent)
{
    *this = copy;
}

TransferringFile::~TransferringFile()
{
    if (m_file.isOpen())
        m_file.close();

    // qDebug() << "Deleting " << m_filename << " file object...";
}

TransferringFile &TransferringFile::operator =(const TransferringFile &other)
{
    //m_file = other.m_file;

    m_filename = other.m_filename;
    m_reply = other.m_reply;
    m_filepath = other.m_filepath;
    m_status = other.m_status;

    m_totalBytes = other.m_totalBytes;
    m_currentBytes = other.m_currentBytes;

    return *this;
}

QString TransferringFile::getFileName()
{
    return m_filename;
}

QString TransferringFile::getFilePath()
{
    return m_filepath;
}

void TransferringFile::setNetworkReply(QNetworkReply *reply)
{
    m_reply = reply;
    connect(m_reply, &QNetworkReply::finished, this, &TransferringFile::onTransferCompleted);
}

QNetworkReply *TransferringFile::getNetworkReply()
{
    return m_reply;
}

void TransferringFile::abortTransfer()
{
    if (m_aborting)
        return;
    //qDebug() << "Aborting transfer...";
    if (m_status != TransferStatus::ERROR && m_status != TransferStatus::COMPLETED){ // Don't change status if not needed
        m_status = TransferStatus::ABORTED;
        m_lastError = tr("Transfert annulé");
    }
    m_aborting = true;
    if (m_reply)
        m_reply->abort();

    emit transferAborted(this);
}

void TransferringFile::onTransferCompleted()
{
    m_file.close();
    // qDebug() << "Transfer completed.";

    if (m_status == TransferStatus::INPROGRESS){
        m_status = TransferStatus::COMPLETED;
        emit transferComplete(this);
    }
}

void TransferringFile::onTransferProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    m_totalBytes = bytesTotal;
    m_currentBytes = bytesReceived;
    //qDebug() << m_filename << " - " << m_currentBytes << " / " << m_totalBytes;

    emit transferProgress(this);
}

QString TransferringFile::getFullFilename(){
    return m_filepath + "/" + m_filename;
}

TransferringFile::TransferStatus TransferringFile::getStatus()
{
    return m_status;
}

void TransferringFile::setStatus(const TransferStatus &status)
{
    m_status = status;
}

qint64 TransferringFile::totalBytes()
{
    return m_totalBytes;
}

qint64 TransferringFile::currentBytes()
{
    return m_currentBytes;
}

QString TransferringFile::getLastError()
{
    return m_lastError;
}

void TransferringFile::setLastError(const QString &err)
{
    m_lastError = err;
}

QFile* TransferringFile::getFile(){
    return &m_file;
}

