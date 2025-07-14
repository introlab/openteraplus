#include "MOVEConfigWidget.h"
#include "ui_MOVEConfigWidget.h"

#include "MOVEWebAPI.h"
#include "WebAPI.h"

MOVEConfigWidget::MOVEConfigWidget(ComManager *comManager, int projectId, QString participantUuid, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MOVEConfigWidget),
    m_comManager(comManager),
    m_idProject(projectId),
    m_uuidParticipant(participantUuid)
{
    ui->setupUi(this);
    ui->cmbSessionTypes->setItemDelegate(new QStyledItemDelegate(ui->cmbSessionTypes));

    m_MOVEComManager = new MOVEComManager(comManager);
    m_uploadDialog = nullptr;
    m_transferDialog = nullptr;

    ui->tabMain->setCurrentIndex(0);
    ui->lblWarning->hide();
    ui->wdgMessages->hide();

    connectSignals();
}

MOVEConfigWidget::~MOVEConfigWidget()
{
    delete ui;
    delete m_MOVEComManager;

    if (m_uploadDialog){
        m_uploadDialog->deleteLater();
    }

    if (m_transferDialog){
        m_transferDialog->deleteLater();
    }
}

void MOVEConfigWidget::refresh()
{
    if (!m_MOVEComManager->isReady()){
        m_refreshRequested = true;
        return;
    }
    // Setup widget according to setted values
    if (m_uuidParticipant.isEmpty()){
        // Hide the playlist tab if no participant specified
        ui->tabMain->removeTab(ui->tabMain->indexOf(ui->tabPlaylist));
        // Refresh videos in library
        queryVideoLibrary();
    }else{
        // Query session types
        QUrlQuery query;
        query.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_idProject));
        m_comManager->doGet(WEB_SESSIONTYPE_PATH, query);
    }
}

void MOVEConfigWidget::on_btnUpload_clicked()
{
    if (m_uploadDialog){
        m_uploadDialog->deleteLater();
    }

    m_uploadDialog = new FileUploaderDialog(tr("Vidéos (*.mp4 *.m4v *.mpg *.webm *.mkv *.avi)"), dynamic_cast<QWidget*>(this));
    if (m_uploadDialog->result() == QDialog::Rejected){
        // No file selected - no need to upload anything!
        m_uploadDialog->deleteLater();
        m_uploadDialog = nullptr;
        return;
    }
    m_uploadDialog->setMinimumWidth(2*this->width()/3);

    connect(m_uploadDialog, &FileUploaderDialog::finished, this, &MOVEConfigWidget::fileUploaderFinished);

    m_uploadDialog->setModal(true);
    m_uploadDialog->show();

}

void MOVEConfigWidget::processVideosReply(QList<QJsonObject> videos)
{

    for (const QJsonObject &video:std::as_const(videos)){
        updateVideoInLibrary(video);
        updateVideoInPlaylist(video);
    }

    if (m_playlistIds.isEmpty() && !m_uuidParticipant.isEmpty() && ui->cmbSessionTypes->count()>0 && ui->cmbSessionTypes->currentIndex()!=-1){
        // Query playlist for that participant
        queryPlaylist();
    }
}

void MOVEConfigWidget::processPlaylistReply(QList<QJsonObject> playlists)
{
    // Check if for us!
    if (playlists.count()>0){
        if (playlists.first()["id_session_type"].toInt() != ui->cmbSessionTypes->currentData().toInt() ||
                playlists.first()["playlist_participant_uuid"].toString() != m_uuidParticipant){
            return;
        }
    }

    // Move all items from current playlist to available videos
    /*for (int i=ui->lstPlaylist->rowCount()-1; i>=0; i--){
        moveVideoItemToAvailable(ui->lstPlaylist->item(i,0));
    }*/

    // Reset playlist and controls
    ui->lstPlaylist->clearContents();
    ui->lstPlaylist->setRowCount(0);
    m_playlistIds.clear();
    checkDirty();

    for (const QJsonObject &playlist:std::as_const(playlists)){
        int id_video = playlist["id_video"].toInt();
        m_playlistIds.append(id_video);
        // Find video in available list
        for (int i=0; i<ui->lstAvailVideos->count(); i++){
            if (ui->lstAvailVideos->item(i)->data(Qt::UserRole).toInt() == id_video){
                //moveVideoItemToPlaylist(ui->lstAvailVideos->item(i));
                copyVideoItemToPlaylist(ui->lstAvailVideos->item(i));
                break;
            }
        }
    }
}

void MOVEConfigWidget::processSessionTypesReply(QList<TeraData> session_types)
{
    int initial_count = ui->cmbSessionTypes->count();

    for (const TeraData &st:session_types){
        if (st.hasFieldName("session_type_service_key")){
            if (st.getFieldValue("session_type_service_key").toString() == "MOVEService"){
                int id_session_type = st.getId();
                int index = ui->cmbSessionTypes->findData(id_session_type);
                if (index == -1){
                    // Not already present
                    ui->cmbSessionTypes->addItem(st.getName(), id_session_type);
                }else{
                    // Update name
                    ui->cmbSessionTypes->setItemText(index, st.getName());
                }
            }
        }
    }

    // Manage selected item
    if (ui->cmbSessionTypes->count() == 0){
        // No session type! Hide everything and display warning
        ui->lblWarning->show();
        ui->framePlaylistManager->hide();
        ui->frameSessionType->hide();
        return;
    }else{
        // Only one element? If so, hides the combobox
        ui->frameSessionType->setVisible(ui->cmbSessionTypes->count()>1);
        if (ui->cmbSessionTypes->currentIndex() == -1){
            // Select first item
            ui->cmbSessionTypes->setCurrentIndex(0);
        }
    }

    // Query videos if none and if session types were changed
    if (ui->lstVideos->count() == 0 && initial_count != ui->cmbSessionTypes->count()){
        queryVideoLibrary();
    }
}

void MOVEConfigWidget::handleNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code)
{
    Q_UNUSED(error)

    if (error == QNetworkReply::OperationCanceledError && op == QNetworkAccessManager::PostOperation){
        // Transfer was cancelled by user - no need to alert anyone!
        return;
    }

    if (error_msg.endsWith('\n'))
        error_msg = error_msg.left(error_msg.length()-1);

    //error_msg = QTextDocumentFragment::fromHtml(error_msg).toPlainText();

    QString error_str;

    if (status_code > 0)
        error_str = tr("Erreur HTTP ") + QString::number(status_code) + ": " + error_msg;
    else
        error_str = tr("Erreur ") + QString::number(error) + ": " + error_msg;

    /*GlobalMessageBox msg;
    msg.showError(tr("Télédanse - Erreur"), error_msg);*/
    error_str = "MOVEService: " + error_str;
    error_str = error_str.replace('\n', " - ");
    error_str = error_str.replace('\r', "");
    ui->wdgMessages->addMessage(Message(Message::MESSAGE_ERROR, error_str));

}

void MOVEConfigWidget::fileUploaderFinished(int result)
{
    if (result == QDialog::Accepted){
        // Do the upload process!
        QStringList files = m_uploadDialog->getFiles();
        QStringList labels = m_uploadDialog->getLabels();
        QVariantMap extra_headers;
        extra_headers["X-IdProject"] = m_idProject;

        for(int i=0; i<files.count(); i++){
            m_MOVEComManager->doUpload(MOVE_LIBRARY_VIDEOS_PATH, files.at(i), extra_headers, labels.at(i), true);
        }
    }

    m_uploadDialog->deleteLater();
    m_uploadDialog = nullptr;
}

void MOVEConfigWidget::MOVEComUploadProgress(UploadingFile *file)
{
    if (!m_transferDialog){
        // New download request - create dialog and add file
        m_transferDialog = new TransferProgressDialog(this);
        connect(m_transferDialog, &TransferProgressDialog::finished, this, &MOVEConfigWidget::transferDialogCompleted);
        m_transferDialog->show();
    }
    m_transferDialog->updateTransferringFile(file);
}

void MOVEConfigWidget::MOVEComUploadCompleted(UploadingFile *file)
{
    if (m_transferDialog){
        if (m_transferDialog->transferFileCompleted(file)){
            // If we are here, no more uploads are pending. Close transfer dialog.
            transferDialogCompleted();
        }
    }
}

void MOVEConfigWidget::MOVEComTransferAborted(TransferringFile *file)
{
    /*if (m_transferDialog){
        m_transferDialog->deleteLater();
        m_transferDialog = nullptr;
    }*/
    if (m_transferDialog){
        m_transferDialog->transferFileAborted(file);
    }
}

void MOVEConfigWidget::MOVEComReady(bool ready)
{
    if (ready){
        if (m_refreshRequested){
            m_refreshRequested = false;
            refresh();
        }
    }
}

void MOVEConfigWidget::transferDialogCompleted()
{
    if (m_transferDialog){
        //m_transferDialog->close();
        m_transferDialog->deleteLater();
        m_transferDialog = nullptr;
    }
}

void MOVEConfigWidget::transferDialogAbortRequested()
{
    m_MOVEComManager->abortTransfers();
}

void MOVEConfigWidget::MOVEComDeleteOK(QString path, int id)
{
    if (path == MOVE_LIBRARY_PATH){
        // Delete video in library
        for (int i=0; i<ui->lstVideos->count(); i++){
            if (ui->lstVideos->item(i)->data(Qt::UserRole).toInt() == id){
                delete ui->lstVideos->takeItem(i);
                break;
            }
        }

        // Also delete video in available list and in playlist, if needed
        for (int i=0; i<ui->lstAvailVideos->count(); i++){
            if (ui->lstAvailVideos->item(i)->data(Qt::UserRole).toInt() == id){
                delete ui->lstAvailVideos->takeItem(i);
                break;
            }
        }
        for (int i=ui->lstPlaylist->rowCount()-1; i>=0; i--){
            if (ui->lstPlaylist->item(i,0)->data(Qt::UserRole).toInt() == id){
                ui->lstPlaylist->removeRow(i);
            }
        }
        m_playlistIds.removeAll(id);
        ui->wdgMessages->addMessage(Message(Message::MESSAGE_OK, tr("Suppression complétée")));
    }

}

void MOVEConfigWidget::MOVEComPostOK(QString path)
{
    ui->wdgMessages->addMessage(Message(Message::MESSAGE_OK, tr("Données sauvegardées")));
}

void MOVEConfigWidget::nextMessageWasShown(Message current_message)
{
    if (current_message.getMessageType()==Message::MESSAGE_NONE){
        ui->wdgMessages->hide();
    }else{
        ui->wdgMessages->show();
    }
}

void MOVEConfigWidget::connectSignals()
{
    connect(m_MOVEComManager, &MOVEComManager::readyChanged, this, &MOVEConfigWidget::MOVEComReady);
    connect(m_MOVEComManager, &MOVEComManager::videosReceived, this, &MOVEConfigWidget::processVideosReply);
    connect(m_MOVEComManager, &MOVEComManager::playlistReceived, this, &MOVEConfigWidget::processPlaylistReply);

    connect(m_comManager, &ComManager::sessionTypesReceived, this, &MOVEConfigWidget::processSessionTypesReply);

    connect(m_MOVEComManager, &MOVEComManager::networkError, this, &MOVEConfigWidget::handleNetworkError);
    connect(m_MOVEComManager, &MOVEComManager::uploadProgress, this, &MOVEConfigWidget::MOVEComUploadProgress);
    connect(m_MOVEComManager, &MOVEComManager::uploadCompleted, this, &MOVEConfigWidget::MOVEComUploadCompleted);
    connect(m_MOVEComManager, &MOVEComManager::transferAborted, this, &MOVEConfigWidget::MOVEComTransferAborted);

    connect(m_MOVEComManager, &MOVEComManager::deleteResultsOK, this, &MOVEConfigWidget::MOVEComDeleteOK);
    connect(m_MOVEComManager, &MOVEComManager::postResultsOK, this, &MOVEConfigWidget::MOVEComPostOK);

    connect(ui->wdgMessages, &ResultMessageWidget::nextMessageShown, this, &MOVEConfigWidget::nextMessageWasShown);
}

void MOVEConfigWidget::updateVideoInLibrary(const QJsonObject &video)
{
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
            item->setIcon(QIcon("://icons/assets/video.png"));
            item->setData(Qt::UserRole, id_video);
        }
        item->setText(video_name);

        ui->lstVideos->addItem(item);
    }
}

void MOVEConfigWidget::updateVideoInPlaylist(const QJsonObject &video)
{
    QString video_name = video["video_label"].toString();
    int id_video = video["id_video"].toInt();

    if (!video_name.isEmpty()){
        QListWidgetItem* item = nullptr;

        // Available videos
        for(int i=0; i<ui->lstAvailVideos->count(); i++){
            if (ui->lstAvailVideos->item(i)->data(Qt::UserRole).toInt() == id_video){
                item = ui->lstAvailVideos->item(i);
                break;
            }
        }

        if (!item){
            item = new QListWidgetItem();
            item->setIcon(QIcon("://icons/assets/video.png"));
            item->setData(Qt::UserRole, id_video);
        }
        item->setText(video_name);

        ui->lstAvailVideos->addItem(item);

        // Videos in playlist
        QTableWidgetItem* playlist_item = nullptr;
        for(int i=0; i<ui->lstPlaylist->rowCount(); i++){
            if (ui->lstPlaylist->item(i,0)->data(Qt::UserRole).toInt() == id_video){
                playlist_item = ui->lstPlaylist->item(i,0);
                playlist_item->setText(video_name);
            }
        }
    }
}

void MOVEConfigWidget::queryVideoLibrary()
{
    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_idProject));
    m_MOVEComManager->doGet(MOVE_LIBRARY_PATH, query);
}

void MOVEConfigWidget::queryPlaylist()
{
    if (m_uuidParticipant.isEmpty())
        return;

    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_UUID_PARTICIPANT, m_uuidParticipant);
    query.addQueryItem(WEB_QUERY_ID_SESSION_TYPE, ui->cmbSessionTypes->currentData().toString());
    m_MOVEComManager->doGet(MOVE_PLAYLIST_PATH, query);
}

void MOVEConfigWidget::moveVideoItemToAvailable(QTableWidgetItem *item)
{
    int current_row = item->row();
    QTableWidgetItem* table_item = ui->lstPlaylist->takeItem(current_row, 0);
    ui->lstPlaylist->removeRow(current_row);
    QListWidgetItem* list_item = new QListWidgetItem(QIcon("://icons/assets/video.png"), table_item->text());
    list_item->setData(Qt::UserRole, table_item->data(Qt::UserRole));
    delete table_item;

    ui->lstAvailVideos->addItem(list_item);
}

void MOVEConfigWidget::moveVideoItemToPlaylist(QListWidgetItem *item)
{
    QTableWidgetItem* table_item = new QTableWidgetItem(QIcon("://icons/assets/video.png"), item->text());
    table_item->setData(Qt::UserRole, item->data(Qt::UserRole));
    delete item;

    ui->lstPlaylist->setRowCount(ui->lstPlaylist->rowCount()+1);
    ui->lstPlaylist->setItem(ui->lstPlaylist->rowCount()-1, 0, table_item);
}

void MOVEConfigWidget::copyVideoItemToPlaylist(QListWidgetItem *item)
{
    QTableWidgetItem* table_item = new QTableWidgetItem(QIcon("://icons/assets/video.png"), item->text());
    table_item->setData(Qt::UserRole, item->data(Qt::UserRole));

    ui->lstPlaylist->setRowCount(ui->lstPlaylist->rowCount()+1);
    ui->lstPlaylist->setItem(ui->lstPlaylist->rowCount()-1, 0, table_item);
}

void MOVEConfigWidget::checkDirty()
{
    bool dirty = ui->lstPlaylist->rowCount() != m_playlistIds.count();

    if (!dirty){ // Yet... count is good, check elements to be sure
        for (int i=0; i<ui->lstPlaylist->rowCount(); i++){
            int video_id = ui->lstPlaylist->item(i,0)->data(Qt::UserRole).toInt();

            if (!m_playlistIds.contains(video_id)){
                dirty = true;
                break;
            }

            if (m_playlistIds.count() > i){
                if (m_playlistIds.at(i) != video_id){
                    dirty = true;
                    break;
                }
            }
        }
    }

    ui->framePlaylistControls->setEnabled(dirty);
}

void MOVEConfigWidget::on_tabMain_currentChanged(int index)
{
    QWidget* current_tab = ui->tabMain->widget(index);

    if (current_tab == ui->tabVideos){

    }

}

void MOVEConfigWidget::on_lstVideos_itemSelectionChanged()
{
    ui->btnRename->setEnabled(!ui->lstVideos->selectedItems().isEmpty());
    ui->btnDelete->setEnabled(!ui->lstVideos->selectedItems().isEmpty());
}


void MOVEConfigWidget::on_btnDelete_clicked()
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
            m_MOVEComManager->doDelete(MOVE_LIBRARY_PATH, id);
        }
    }

}


void MOVEConfigWidget::on_btnRename_clicked()
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

            m_MOVEComManager->doPost(MOVE_LIBRARY_PATH, document.toJson());
        }
    }

    ui->lstVideos->clearSelection();
}


void MOVEConfigWidget::on_lstAvailVideos_itemDoubleClicked(QListWidgetItem *item)
{
    on_btnAddVideo_clicked();
}


void MOVEConfigWidget::on_lstAvailVideos_itemSelectionChanged()
{
    ui->btnAddVideo->setEnabled(ui->lstAvailVideos->currentItem() != nullptr);
}


void MOVEConfigWidget::on_btnMoveUp_clicked()
{
    if (!ui->lstPlaylist->currentItem())
        return;

    int current_row = ui->lstPlaylist->currentRow();
    QTableWidgetItem* item = ui->lstPlaylist->takeItem(current_row, 0);
    ui->lstPlaylist->removeRow(current_row);
    ui->lstPlaylist->insertRow(current_row-1);

    ui->lstPlaylist->setItem(current_row-1, 0, item);
    ui->lstPlaylist->setCurrentItem(item);

    checkDirty();
}


void MOVEConfigWidget::on_btnAddVideo_clicked()
{
    if (!ui->lstAvailVideos->currentItem())
        return;

    //moveVideoItemToPlaylist(ui->lstAvailVideos->currentItem());
    copyVideoItemToPlaylist(ui->lstAvailVideos->currentItem());

    ui->lstPlaylist->clearSelection();
    ui->lstAvailVideos->clearSelection();
    ui->btnAddVideo->setEnabled(false);

    checkDirty();
}


void MOVEConfigWidget::on_btnDelVideo_clicked()
{
    if (!ui->lstPlaylist->currentItem())
        return;

    //moveVideoItemToAvailable(ui->lstPlaylist->currentItem());
    int current_row = ui->lstPlaylist->currentRow();

    ui->lstPlaylist->takeItem(current_row, 0);
    ui->lstPlaylist->removeRow(current_row);

    ui->lstPlaylist->clearSelection();
    ui->btnDelVideo->setEnabled(false);
    ui->btnMoveDown->setEnabled(false);
    ui->btnMoveUp->setEnabled(false);

    checkDirty();
}


void MOVEConfigWidget::on_btnMoveDown_clicked()
{
    if (!ui->lstPlaylist->currentItem())
        return;

    int current_row = ui->lstPlaylist->currentRow();
    QTableWidgetItem* item = ui->lstPlaylist->takeItem(current_row,0);
    ui->lstPlaylist->removeRow(current_row);
    ui->lstPlaylist->insertRow(current_row+1);

    ui->lstPlaylist->setItem(current_row+1, 0, item);
    ui->lstPlaylist->setCurrentItem(item);

    checkDirty();

}


void MOVEConfigWidget::on_lstPlaylist_itemSelectionChanged()
{
    QTableWidgetItem* current_item = ui->lstPlaylist->currentItem();
    ui->btnDelVideo->setEnabled(current_item != nullptr);

    if (current_item){
        ui->btnMoveDown->setEnabled(current_item != ui->lstPlaylist->item(ui->lstPlaylist->rowCount()-1, 0));
        ui->btnMoveUp->setEnabled(current_item != ui->lstPlaylist->item(0,0));
    }else{
        ui->btnMoveDown->setEnabled(false);
        ui->btnMoveUp->setEnabled(false);
    }

}


void MOVEConfigWidget::on_lstPlaylist_itemDoubleClicked(QTableWidgetItem *item)
{
    on_btnDelVideo_clicked();
}


void MOVEConfigWidget::on_btnSave_clicked()
{
    // Send current playlist
    // Create and do post query
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonArray videos;

    int id_session_type = ui->cmbSessionTypes->currentData().toInt();
    for (int i=0; i<ui->lstPlaylist->rowCount(); i++){
        QJsonObject data_obj;
        data_obj["id_video"] = ui->lstPlaylist->item(i,0)->data(Qt::UserRole).toInt();
        data_obj["id_session_type"] = id_session_type;
        data_obj["playlist_participant_uuid"] = m_uuidParticipant;
        data_obj["playlist_order"] = i;

        videos.append(data_obj);
    }

    base_obj.insert("playlist", videos);
    document.setObject(base_obj);

    m_MOVEComManager->doPost(MOVE_PLAYLIST_PATH, document.toJson());
}


void MOVEConfigWidget::on_cmbSessionTypes_currentIndexChanged(int index)
{
    if (ui->lstVideos->count() > 0)
        // Requery playlist for current session
        queryPlaylist();
}


void MOVEConfigWidget::on_btnCancel_clicked()
{
    // Reset videos in the playlist
    ui->lstPlaylist->clearContents();
    ui->lstPlaylist->setRowCount(0);

    for (const int& id:std::as_const(m_playlistIds)){
        // Find video in available
        for(int i=0; i<ui->lstAvailVideos->count(); i++){
            if (ui->lstAvailVideos->item(i)->data(Qt::UserRole).toInt() == id){
                copyVideoItemToPlaylist(ui->lstAvailVideos->item(i));
                break;
            }
        }
    }

    checkDirty();
}

