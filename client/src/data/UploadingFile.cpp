#include "UploadingFile.h"

UploadingFile::UploadingFile(QObject *parent)
    : TransferringFile{parent}
{
}

UploadingFile::UploadingFile(const QString &file_name, QObject *parent)
    : TransferringFile()
{
    setFile(file_name);
}

UploadingFile::UploadingFile(const UploadingFile &copy, QObject *parent)
{
    *this = copy;
}

UploadingFile::~UploadingFile()
{
}

bool UploadingFile::setFile(const QString &file_name)
{

    // Get informations about file
    if (!QFile::exists(file_name)){
        m_lastError = tr("Impossible d'envoyer le fichier") + " " + file_name + ": " + tr("le fichier est introuvable.");
        LOG_ERROR(m_lastError, "UploadingFile::setFile");
        return false;
    }

    m_file.setFileName(file_name);

    QFileInfo info(file_name);
    m_filename = info.fileName();
    m_filepath = info.filePath();

    m_currentBytes = 0;
    m_totalBytes = m_file.size();

    return true;
}


void UploadingFile::setNetworkReply(QNetworkReply *reply)
{
    TransferringFile::setNetworkReply(reply);
    connect(m_reply, &QNetworkReply::uploadProgress, this, &UploadingFile::onTransferProgress);

}
