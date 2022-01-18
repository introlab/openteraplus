#include "DanceConfigWidget.h"
#include "ui_DanceConfigWidget.h"

DanceConfigWidget::DanceConfigWidget(ComManager *comManager, int projectId, int participantId, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DanceConfigWidget),
    m_comManager(comManager),
    m_idProject(projectId),
    m_idParticipant(participantId)
{
    ui->setupUi(this);

    m_danceComManager = new DanceComManager(comManager);
    m_uploadDialog = nullptr;
    m_transferDialog = nullptr;

    // Hide the playlist tab if no participant specified
    if (m_idParticipant <= 0){
        ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabPlaylist));
    }

    ui->tabMain->setCurrentIndex(0);

    connectSignals();

}

DanceConfigWidget::~DanceConfigWidget()
{
    delete ui;
    delete m_danceComManager;

    if (m_uploadDialog){
        m_uploadDialog->deleteLater();
    }

    if (m_transferDialog){
        m_transferDialog->deleteLater();
    }
}

void DanceConfigWidget::on_btnUpload_clicked()
{
    if (m_uploadDialog){
        m_uploadDialog->deleteLater();
    }

    m_uploadDialog = new FileUploaderDialog(dynamic_cast<QWidget*>(this));

    connect(m_uploadDialog, &FileUploaderDialog::finished, this, &DanceConfigWidget::fileUploaderFinished);

    m_uploadDialog->setModal(true);
    m_uploadDialog->show();

}

void DanceConfigWidget::processVideosReply(QList<QJsonObject> videos)
{

    for (const QJsonObject &video:qAsConst(videos)){
        QString video_name = video["video_label"].toString();
        int id_video = video["id_video"].toInt();

        if (!video_name.isEmpty()){
            QListWidgetItem* item = nullptr;

            for(int i=0; i<ui->lstVideos->count(); i++){
                if (ui->lstVideos->item(i)->data(Qt::UserRole).toInt() == id_video){
                    item = ui->lstVideos->item(i);
                    break;
                }
            }

            if (!item){
                item = new QListWidgetItem();
                item->setIcon(QIcon("://icons/data_video.png"));
                item->setData(Qt::UserRole, id_video);
            }
            item->setText(video_name);

            ui->lstVideos->addItem(item);
        }

    }
}

void DanceConfigWidget::handleNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code)
{
    Q_UNUSED(error)

    if (error == QNetworkReply::OperationCanceledError && op == QNetworkAccessManager::PostOperation){
        // Transfer was cancelled by user - no need to alert anyone!
        return;
    }

    if (error_msg.endsWith('\n'))
        error_msg = error_msg.left(error_msg.length()-1);

    error_msg = QTextDocumentFragment::fromHtml(error_msg).toPlainText();

    QString error_str;

    if (status_code > 0)
        error_str = tr("Erreur HTTP ") + QString::number(status_code) + ": " + error_msg;
    else
        error_str = tr("Erreur ") + QString::number(error) + ": " + error_msg;

    GlobalMessageBox msg;
    msg.showError(tr("Une erreur est survenue..."), error_msg);

}

void DanceConfigWidget::fileUploaderFinished(int result)
{
    if (result == QDialog::Accepted){
        // Do the upload process!
        QStringList files = m_uploadDialog->getFiles();
        QStringList labels = m_uploadDialog->getLabels();
        QVariantMap extra_headers;
        extra_headers["X-IdProject"] = m_idProject;

        for(int i=0; i<files.count(); i++){
            m_danceComManager->doUpload(DANCE_LIBRARY_VIDEOS_PATH, files.at(i), extra_headers, labels.at(i), true);
        }
    }

    m_uploadDialog->deleteLater();
    m_uploadDialog = nullptr;
}

void DanceConfigWidget::danceComUploadProgress(UploadingFile *file)
{
    if (!m_transferDialog){
        // New download request - create dialog and add file
        m_transferDialog = new TransferProgressDialog(this);
        m_transferDialog->show();
    }
    m_transferDialog->updateTransferringFile(file);
}

void DanceConfigWidget::danceComUploadCompleted(UploadingFile *file)
{
    if (m_transferDialog){
        if (m_transferDialog->transferFileCompleted(file)){
            // If we are here, no more uploads are pending. Close transfer dialog.
            m_transferDialog->close();
            m_transferDialog->deleteLater();
            m_transferDialog = nullptr;
        }
    }
}

void DanceConfigWidget::danceComTransferAborted(TransferringFile *file)
{
    if (m_transferDialog){
        m_transferDialog->deleteLater();
        m_transferDialog = nullptr;
    }
}

void DanceConfigWidget::danceComDeleteOK(QString path, int id)
{
    if (path == DANCE_LIBRARY_PATH){
        for (int i=0; i<ui->lstVideos->count(); i++){
            if (ui->lstVideos->item(i)->data(Qt::UserRole).toInt() == id){
                delete ui->lstVideos->takeItem(i);
                break;
            }
        }
    }
}

void DanceConfigWidget::connectSignals()
{
    connect(m_danceComManager, &DanceComManager::videosReceived, this, &DanceConfigWidget::processVideosReply);

    connect(m_danceComManager, &DanceComManager::networkError, this, &DanceConfigWidget::handleNetworkError);
    connect(m_danceComManager, &DanceComManager::uploadProgress, this, &DanceConfigWidget::danceComUploadProgress);
    connect(m_danceComManager, &DanceComManager::uploadCompleted, this, &DanceConfigWidget::danceComUploadCompleted);
    connect(m_danceComManager, &DanceComManager::transferAborted, this, &DanceConfigWidget::danceComTransferAborted);

    connect(m_danceComManager, &DanceComManager::deleteResultsOK, this, &DanceConfigWidget::danceComDeleteOK);
}

void DanceConfigWidget::on_tabMain_currentChanged(int index)
{
    QWidget* current_tab = ui->tabMain->widget(index);

    if (current_tab == ui->tabVideos){
        // Refresh videos in library
        QUrlQuery query;
        query.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_idProject));
        m_danceComManager->doGet(DANCE_LIBRARY_PATH, query);
    }

}

void DanceConfigWidget::on_lstVideos_itemSelectionChanged()
{
    ui->btnRename->setEnabled(!ui->lstVideos->selectedItems().isEmpty());
    ui->btnDelete->setEnabled(!ui->lstVideos->selectedItems().isEmpty());
}


void DanceConfigWidget::on_btnDelete_clicked()
{
    GlobalMessageBox msg;
    QMessageBox::StandardButton conf = msg.showYesNo(tr("Suppression"), tr("Êtes-vous sûr de vouloir supprimer les éléments sélectionnées?"));

    if (conf == QMessageBox::Yes){
        // Delete all files
        QList<int> ids_to_del;
        for(int i=0; i<ui->lstVideos->selectedItems().count(); i++){
            ids_to_del.append(ui->lstVideos->selectedItems().at(i)->data(Qt::UserRole).toInt());

        }
        for(int id:ids_to_del){
            m_danceComManager->doDelete(DANCE_LIBRARY_PATH, id);
        }
    }

}


void DanceConfigWidget::on_btnRename_clicked()
{
    for(int i=0; i<ui->lstVideos->selectedItems().count(); i++){
        bool ok;
        QString current_label =  ui->lstVideos->selectedItems().at(i)->text();
        QString new_label = QInputDialog::getText(this, tr("Renommer"), tr("Nouveau libellé"), QLineEdit::Normal, current_label, &ok);

        if (ok && !new_label.isEmpty() && new_label != current_label){
            // Create and do post query
            QJsonDocument document;
            QJsonObject base_obj;
            QJsonObject data_obj;
            data_obj["id_video"] = ui->lstVideos->selectedItems().at(i)->data(Qt::UserRole).toInt();
            data_obj["video_label"] = new_label;
            base_obj.insert("video", data_obj);
            document.setObject(base_obj);

            m_danceComManager->doPost(DANCE_LIBRARY_PATH, document.toJson());
        }
    }

    ui->lstVideos->clearSelection();
}

