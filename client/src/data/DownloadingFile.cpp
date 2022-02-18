#include "DownloadingFile.h"
#include <QDebug>

DownloadingFile::DownloadingFile(QObject *parent) : TransferringFile(parent)
{

}

DownloadingFile::DownloadingFile(const QString &save_path, const QString &file_name, QObject *parent)
    : TransferringFile(save_path, file_name, parent)
{


}

DownloadingFile::DownloadingFile(const DownloadingFile &copy, QObject *parent) : TransferringFile(copy, parent)
{
    *this = copy;
}

DownloadingFile &DownloadingFile::operator =(const DownloadingFile &other)
{
    TransferringFile::operator=(other);

    return *this;
}

void DownloadingFile::setNetworkReply(QNetworkReply *reply)
{
    TransferringFile::setNetworkReply(reply);
    connect(m_reply, &QNetworkReply::readyRead, this, &DownloadingFile::onDownloadDataReceived);
    connect(m_reply, &QNetworkReply::downloadProgress, this, &DownloadingFile::onTransferProgress);

}

void DownloadingFile::abortTransfer()
{
   TransferringFile::abortTransfer();
   // Transfer aborted - remove the downloaded file so far
   QDir dir;
   if (dir.exists(getFullFilename()))
       dir.remove(getFullFilename());

}

void DownloadingFile::onDownloadDataReceived()
{
    // Check if we have a file open for writing
    if (!m_file.isOpen()){
        QString header_info = m_reply->header(QNetworkRequest::ContentDispositionHeader).toString();
        QStringList header_info_parts = header_info.split(";");

        if (m_filename.isEmpty()){
            // Get filename from reply
            if (header_info_parts.count()>0){
                /*if (header_info_parts.first() != "attachment"){
                    m_lastError = tr("Impossible de télécharger un objet qui n'est pas de type 'attachment'.");
                    qDebug() << m_lastError;
                    abortTransfer();
                    return;
                }*/
                for (const QString &info: qAsConst(header_info_parts)){
                    if (info.trimmed().startsWith("filename=")){
                        QStringList file_parts = info.split("=");
                        if (file_parts.count() == 2)
                            m_filename = file_parts.last();
                        break;
                    }
                }
                if (m_filename.isEmpty()){
                    m_lastError = tr("Impossible de déterminer le nom du fichier à télécharger.");
                    m_status = TransferringFile::ERROR;
                    abortTransfer();
                    return;
                }
            }else{
                m_lastError = tr("Mauvaise en-tête pour le téléchargement du fichier.");
                m_status = TransferringFile::ERROR;
                abortTransfer();
                return;
            }
        }

        // Open the file for writing
        m_file.setFileName(getFullFilename());
        if (!m_file.open(QIODevice::WriteOnly)){
            m_lastError = tr("Impossible d'ouvrir le fichier '") + getFullFilename() + "': " + m_file.errorString();
            m_status = TransferringFile::ERROR;
            abortTransfer();
            return;
        }
        //qDebug() << "Saving to: " << getFullFilename();
    }
    QByteArray data = m_reply->readAll();
    m_file.write(data);
}


