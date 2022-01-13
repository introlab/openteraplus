#include "TransferringFile.h"

TransferringFile::TransferringFile(QObject *parent)
    : QObject{parent}
{
    m_reply = nullptr;
}

TransferringFile::TransferringFile(const QString &file_path, const QString &file_name, QObject *parent)
{
    m_filename = file_name;
    m_filepath = file_path;
}

TransferringFile::TransferringFile(const TransferringFile &copy, QObject *parent)
{
    *this = copy;
}

TransferringFile &TransferringFile::operator =(const TransferringFile &other)
{
    //m_file = other.m_file;
    m_filename = other.m_filename;
    m_reply = other.m_reply;
    m_filepath = other.m_filepath;

    return *this;
}

void TransferringFile::setNetworkReply(QNetworkReply *reply)
{
    m_reply = reply;
    connect(m_reply, &QNetworkReply::finished, this, &TransferringFile::onTransferCompleted);
}

void TransferringFile::abortTransfer()
{
    qDebug() << "Aborting transfer...";
    if (m_reply)
        m_reply->abort();

    if (m_file.isOpen()){
        m_file.close();
    }
}

void TransferringFile::onTransferCompleted()
{
    m_file.close();
    qDebug() << "Transfer completed.";
    emit transferComplete(this);
}

QString TransferringFile::getFullFilename(){
    return m_filepath + "/" + m_filename;
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
