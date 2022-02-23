#include "AssetsWidget.h"
#include "ui_AssetsWidget.h"

AssetsWidget::AssetsWidget(ComManager *comMan, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AssetsWidget)
{
    ui->setupUi(this);

    m_comAssetManager = nullptr;
    m_fileTransferServiceInfos = nullptr;
    m_uploadDialog = nullptr;
    m_transferDialog = nullptr;
    m_idProject = -1;
    m_idSession = -1;
    m_viewMode = VIEWMODE_UNKNOWN;
    setComManager(comMan);

    // Initialize refresh access token timer
    m_refreshTokenTimer.setSingleShot(true);
    m_refreshTokenTimer.setInterval(29 * 60 * 1000); // 29 minutes interval
    //m_refreshTokenTimer.setInterval(2000); // 2 seconds interval for testing!

    connectSignals();

    // Hide add button by default
    ui->btnNew->hide();
    ui->wdgMessages->hide();
    ui->btnExpandAll->hide();

    // Columns not used for general assets
    ui->treeAssets->hideColumn(AssetColumn::ASSET_SIZE);
    ui->treeAssets->hideColumn(AssetColumn::ASSET_DURATION);

    // Catch keyboard events
    ui->treeAssets->installEventFilter(this);

    // Initialize default directory for file dialog
    QStringList documents_path = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);

    if (!documents_path.isEmpty())
        m_fileDialog.setDirectory(documents_path.first());


}

AssetsWidget::~AssetsWidget()
{
    delete ui;
    if (m_comAssetManager)
        m_comAssetManager->deleteLater();
    if (m_fileTransferServiceInfos)
        delete m_fileTransferServiceInfos;
    qDeleteAll(m_assets);

    if (m_uploadDialog){
        m_uploadDialog->deleteLater();
    }

    if (m_transferDialog){
        m_transferDialog->deleteLater();
    }
}

void AssetsWidget::setComManager(ComManager *comMan)
{
    m_comManager = comMan;

    if (m_comManager){
        connect(m_comManager, &ComManager::assetsReceived, this, &AssetsWidget::processAssetsReply);
        connect(m_comManager, &ComManager::servicesReceived, this, &AssetsWidget::processServicesReply);

        m_comAssetManager = new AssetComManager(m_comManager);

        connect(m_comAssetManager, &AssetComManager::networkError, this, &AssetsWidget::assetComNetworkError);
        connect(m_comAssetManager, &AssetComManager::uploadProgress, this, &AssetsWidget::assetComUploadProgress);
        connect(m_comAssetManager, &AssetComManager::uploadCompleted, this, &AssetsWidget::assetComUploadCompleted);
        connect(m_comAssetManager, &AssetComManager::downloadProgress, this, &AssetsWidget::assetComDownloadProgress);
        connect(m_comAssetManager, &AssetComManager::downloadCompleted, this, &AssetsWidget::assetComDownloadCompleted);
        connect(m_comAssetManager, &AssetComManager::transferAborted, this, &AssetsWidget::assetComTransferAborted);
        connect(m_comAssetManager, &AssetComManager::deleteUuidResultOK, this, &AssetsWidget::assetComDeleteOK);
        connect(m_comAssetManager, &AssetComManager::postResultsOK, this, &AssetsWidget::assetComPostOK);

        connect(m_comAssetManager, &AssetComManager::assetsInfosReceived, this, &AssetsWidget::processAssetsInfos);

    }
}

void AssetsWidget::setAssociatedProject(const int &id_project)
{
    m_idProject = id_project;
}

bool AssetsWidget::isEmpty() const
{
    return m_assets.isEmpty();
}

void AssetsWidget::clearAssets()
{
    ui->treeAssets->clear();
    qDeleteAll(m_assets);
    m_assets.clear();
    m_treeAssets.clear();
    m_treeSessions.clear();
    m_treeParticipants.clear();
    m_assetsSessions.clear();
}

void AssetsWidget::enableNewAssets(const bool &enable)
{
    if (enable && !m_fileTransferServiceInfos && m_idProject >= 0 && m_comManager){
        // Query FileTransferService infos
        QUrlQuery query;
        query.addQueryItem(WEB_QUERY_SERVICE_KEY, "FileTransferService");
        m_comManager->doGet(WEB_SERVICEINFO_PATH, query);
        return;
    }
    if (m_fileTransferServiceInfos && enable){
        ui->btnNew->setVisible(true);
        return;
    }
    // Default - no setted id project or enable = false
    ui->btnNew->setVisible(false);
}

void AssetsWidget::displayAssetsForSession(const int &id_session)
{
    if (!m_comManager)
        return;

    clearAssets();

    setViewMode(VIEWMODE_SESSION);

    setLoading(true, tr("Chargement des données en cours..."), true);

    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(id_session));
    m_dataQuery = query;

    query.addQueryItem(WEB_QUERY_WITH_URLS, "1");
    query.addQueryItem(WEB_QUERY_FULL, "1"); // Include full information, including service name
    m_comManager->doGet(WEB_ASSETINFO_PATH, query);

    m_idSession = id_session;
}

void AssetsWidget::displayAssetsForParticipant(const int &id_participant)
{
    if (!m_comManager)
        return;

    clearAssets();

    setViewMode(VIEWMODE_PARTICIPANT);

    setLoading(true, tr("Chargement des données en cours..."), true);

    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_PARTICIPANT, QString::number(id_participant));
    m_dataQuery = query;

    query.addQueryItem(WEB_QUERY_WITH_URLS, "1");
    query.addQueryItem(WEB_QUERY_FULL, "1"); // Include full information, including service name
    m_comManager->doGet(WEB_ASSETINFO_PATH, query);

}

void AssetsWidget::connectSignals()
{
    // Com related signals are set in setComManager
    connect(ui->wdgMessages, &ResultMessageWidget::nextMessageShown, this, &AssetsWidget::nextMessageWasShown);
    connect(&m_refreshTokenTimer, &QTimer::timeout, this, &AssetsWidget::refreshAccessToken);
}

void AssetsWidget::resizeAssetsColumnsToContent()
{
  for (int i=0; i<ui->treeAssets->columnCount(); i++)
      ui->treeAssets->resizeColumnToContents(i);
}

bool AssetsWidget::eventFilter(QObject *o, QEvent *e)
{
    if( o == ui->treeAssets && e->type() == QEvent::KeyRelease )
    {
        QKeyEvent* key_event = dynamic_cast<QKeyEvent*>(e);
        if (key_event){
            if (key_event->key() == Qt::Key_Delete){
                on_btnDelete_clicked();
            }
        }
    }

    return QWidget::eventFilter(o,e);
}

void AssetsWidget::queryAssetsInfos()
{
    if (m_assets.isEmpty()){
        setLoading(false);
        return;
    }

    setLoading(true, tr("Chargement des informations de données en cours..."), true);

    QMultiMap<QString, QString> services_assets;
    QMultiMap<QString, QString> services_assets_tokens;

    // Group each assets by access url
    for(TeraData* asset: qAsConst(m_assets)){
        if (asset->hasFieldName("asset_infos_url")){
            QString asset_info_str = asset->getFieldValue("asset_infos_url").toString();
            if (!asset_info_str.isEmpty()){
                QString asset_info_url = QUrl(asset_info_str).url(QUrl::RemoveQuery);
                services_assets.insert(asset_info_url, asset->getUuid());
                services_assets_tokens.insert(asset_info_url, asset->getFieldValue("access_token").toString());
            }
        }
    }

    // Query each service for their assets
    QStringList service_urls = services_assets.uniqueKeys();
    for(const QString &service_url: qAsConst(service_urls)){
        QStringList asset_uuids = services_assets.values(service_url);
        QStringList asset_tokens = services_assets_tokens.values(service_url);
        QJsonDocument document;
        QJsonObject base_obj;
        QJsonArray assets_array;

        for(int i=0; i<asset_uuids.count(); i++){
            QJsonObject asset_obj;
            asset_obj["asset_uuid"] = asset_uuids.at(i);
            asset_obj["access_token"] = asset_tokens.at(i);
            assets_array.append(asset_obj);
        }

        base_obj.insert("assets", assets_array);

        document.setObject(base_obj);

        m_comAssetManager->doPost(QUrl(service_url), document.toJson(), true);
    }
}

void AssetsWidget::updateAsset(const TeraData &asset, const int &id_participant, const bool &emit_count_update_signal)
{
    QTreeWidgetItem* item;
    bool has_asset_infos_url = false;
    bool has_asset_download_url = false;

    if (asset.hasFieldName("asset_infos_url")){
        has_asset_infos_url = !asset.getFieldValue("asset_infos_url").toString().isEmpty();
    }

    if (asset.hasFieldName("asset_url")){
        has_asset_download_url = !asset.getFieldValue("asset_url").toString().isEmpty();
    }

    //btnDownload = dynamic_cast<QToolButton*>(ui->tableSessions->cellWidget(name_item->row(), 6)->layout()->itemAt(2)->widget());
    QToolButton* btnDownload;
    QToolButton* btnView;

    QString asset_uuid = asset.getUuid();

    // Check to display asset correctly
    if (m_treeAssets.contains(asset_uuid)){
        item = m_treeAssets[asset_uuid];
        btnDownload = dynamic_cast<QToolButton*>(ui->treeAssets->itemWidget(item, ui->treeAssets->columnCount()-1)->layout()->itemAt(2)->widget());
        btnView = dynamic_cast<QToolButton*>(ui->treeAssets->itemWidget(item, ui->treeAssets->columnCount()-1)->layout()->itemAt(0)->widget());
    }else{
        // Add assets info to internal list
        if (!m_assets.contains(asset_uuid))
            m_assets.insert(asset_uuid, new TeraData(asset));

        QTreeWidgetItem* parent_item = nullptr;

        // Participant name
        QTreeWidgetItem* participant_item = nullptr;
        /*if (id_participant>=0){
            if (m_treeParticipants.contains(id_participant)){
                parent_item = m_treeParticipants[id_participant];
            }
        }*/

        // Session name
        if (m_viewMode == VIEWMODE_PARTICIPANT || m_viewMode == VIEWMODE_PROJECT){
            int id_session = asset.getFieldValue("id_session").toInt();
            QTreeWidgetItem* session_item;
            if (m_treeSessions.contains(id_session)){
                session_item = m_treeSessions[id_session];
            }else{
                session_item = new QTreeWidgetItem();
                QString session_name = tr("Séance ") + QString::number(id_session);
                if (asset.hasFieldName("asset_session_name")){
                    session_name = asset.getFieldValue("asset_session_name").toString();
                }
                session_item->setText(AssetColumn::ASSET_NAME, session_name);
                session_item->setIcon(AssetColumn::ASSET_NAME, QIcon("://icons/group.png"));
                session_item->setFirstColumnSpanned(true);
                session_item->setForeground(AssetColumn::ASSET_NAME, Qt::cyan);
                session_item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

                if (participant_item){
                    participant_item->addChild(session_item);
                }else{
                    ui->treeAssets->addTopLevelItem(session_item);
                }

                m_treeSessions[id_session] = session_item;
            }
            parent_item = session_item;

            if (!m_assetsSessions.values(id_session).contains(asset_uuid)){
                m_assetsSessions.insert(id_session, asset_uuid);
            }
        }



        if (parent_item){
            if (!parent_item->isExpanded())
                return; // If parent item is not expanded, don't display the asset!
        }

        item = new QTreeWidgetItem();

        // Asset name and infos
        item->setForeground(AssetColumn::ASSET_NAME, QColor(255,255,102));
        QFont font = item->font(AssetColumn::ASSET_NAME);
        font.setBold(true);
        item->setFont(AssetColumn::ASSET_NAME, font);

        if (!parent_item){
            ui->treeAssets->addTopLevelItem(item);
        }else{
            parent_item->addChild(item);
        }

        // Create action buttons
        QFrame* action_frame = new QFrame();
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        layout->setAlignment(Qt::AlignLeft);
        action_frame->setLayout(layout);

        // View asset
        btnView = new QToolButton();
        btnView->setIcon(QIcon(":/icons/search.png"));
        btnView->setIconSize(QSize(24,24));
        btnView->setProperty("asset_uuid", asset.getUuid());
        btnView->setCursor(Qt::PointingHandCursor);
        btnView->setMaximumWidth(32);
        btnView->setToolTip(tr("Ouvrir"));
        //connect(btnView, &QToolButton::clicked, this, &ParticipantWidget::btnViewSession_clicked);
        layout->addWidget(btnView);

        // Delete
        QToolButton* btnDelete = new QToolButton();
        btnDelete->setIcon(QIcon(":/icons/delete_old.png"));
        btnDelete->setIconSize(QSize(24,24));
        btnDelete->setProperty("asset_uuid", asset.getUuid());
        btnDelete->setCursor(Qt::PointingHandCursor);
        btnDelete->setMaximumWidth(32);
        btnDelete->setToolTip(tr("Supprimer"));
        connect(btnDelete, &QToolButton::clicked, this, &AssetsWidget::on_btnDelete_clicked);
        layout->addWidget(btnDelete);

        // Download data
        btnDownload = new QToolButton();
        btnDownload->setIcon(QIcon(":/icons/download.png"));
        btnDownload->setIconSize(QSize(24,24));
        btnDownload->setProperty("asset_uuid", asset.getUuid());
        btnDownload->setCursor(Qt::PointingHandCursor);
        btnDownload->setMaximumWidth(32);
        btnDownload->setToolTip(tr("Télécharger les données"));
        connect(btnDownload, &QToolButton::clicked, this, &AssetsWidget::on_btnDownload_clicked);
        layout->addWidget(btnDownload);

        ui->treeAssets->setItemWidget(item, ui->treeAssets->columnCount()-1, action_frame);

        // Update internal lists
        m_treeAssets.insert(asset.getUuid(), item);

        // Emit update signal
        if (emit_count_update_signal)
            emit assetCountChanged(m_assets.count());
    }

    // Update informations
    item->setText(AssetColumn::ASSET_NAME, asset.getName());
    item->setIcon(AssetColumn::ASSET_NAME, QIcon(TeraAsset::getIconForContentType(asset.getFieldValue("asset_type").toString())));
    item->setText(AssetColumn::ASSET_DATETIME, asset.getFieldValue("asset_datetime").toDateTime().toLocalTime().toString("dd-MM-yyyy hh:mm:ss"));
    if (asset.hasFieldName("asset_service_owner_name"))
        item->setText(AssetColumn::ASSET_SERVICE, asset.getFieldValue("asset_service_owner_name").toString());
    btnDownload->setEnabled(has_asset_download_url);
    btnView->setEnabled(has_asset_infos_url);
}

QString AssetsWidget::getRelativePathForAsset(const QString &uuid)
{
    QString path = "/";
    int id_session = m_assets[uuid]->getFieldValue("id_session").toInt();
    if (m_treeSessions.contains(id_session)){
        QString clean_text = Utils::removeAccents(m_treeSessions[id_session]->text(0));
        clean_text = Utils::removeNonAlphanumerics(clean_text);
        path = "/" + clean_text + path;
    }

    // TODO: handle participants too!

   /* QTreeWidgetItem* current_item = m_treeAssets[uuid]->parent();
    while(current_item){
        QString clean_text = Utils::removeAccents(current_item->text(0));
        clean_text = Utils::removeNonAlphanumerics(clean_text);
        path = "/" + clean_text + path;
        current_item = current_item->parent();
    }*/
    return path;

}

QString AssetsWidget::getFilenameForAsset(const QString &uuid)
{
    QString filename = tr("Inconnue");

    if (!m_assets.contains(uuid)){
        LOG_WARNING("Tried to get asset " + uuid + ", but not found", "AssetsWidget::getFilenameForAsset");
        return filename;
    }

    TeraData* asset = m_assets[uuid];
    if (asset->hasFieldName("asset_original_filename")){
        filename = asset->getFieldValue("asset_original_filename").toString();
    }else{
        if (asset->hasFieldName("file_name")){
            filename = asset->getFieldValue("file_name").toString();
        }else
            filename = asset->getName();
    }
    return filename;
}

void AssetsWidget::startAssetsDownload(const QStringList &assets_to_download)
{
    if (assets_to_download.isEmpty())
        return; // Should not happen...

    // Ask where to save (file if one asset, directory otherwise)
    QString full_path;

    if (assets_to_download.count() == 1){
        TeraData* asset = m_assets[assets_to_download.first()];
        QString filename = getFilenameForAsset(asset->getUuid());
        m_fileDialog.selectFile(filename);
        m_fileDialog.setFileMode(QFileDialog::AnyFile);
        m_fileDialog.setWindowTitle(tr("Fichier à enregistrer"));
        //m_fileDialog.setLabelText(QFileDialog::Accept, tr("Enregistrer"));
        m_fileDialog.setAcceptMode(QFileDialog::AcceptSave);

        if (m_fileDialog.exec()){

            QStringList selected_files = m_fileDialog.selectedFiles();
            if (selected_files.isEmpty())
                return;

            full_path = selected_files.first();
        }else{
            return;
        }
    }else{
        full_path = m_fileDialog.getExistingDirectory(this, tr("Répertoire où les données seront téléchargées"));
        if (full_path.isEmpty())
            return;

        // Check if the directory is empty - if not, warn and delete everything!
        QDir save_dir(full_path);
        if (save_dir.exists() && !save_dir.isEmpty()){
            GlobalMessageBox msg_box;
            if (msg_box.showYesNo(tr("Confirmation d'écrasement"), tr("Le répertoire contient des fichiers. Tous les fichiers existants seront supprimés. Souhaitez-vous poursuivre?")) != GlobalMessageBox::Yes){
                return;
            }
            // Delete everything!
            save_dir.removeRecursively();
        }
    }

    for(const QString &asset_uuid:assets_to_download){
        TeraData* asset = m_assets[asset_uuid];
        QString file_path;
        QString file_name;
        if (assets_to_download.count() == 1){
            // Only one file - use the info entered by the user
            QFileInfo fileInfo(full_path);
            file_path = fileInfo.absolutePath();
            file_name = fileInfo.fileName();
        }else{
            // Multiple files - create save structure
            file_path = full_path + getRelativePathForAsset(asset_uuid);
            file_name = getFilenameForAsset(asset_uuid);

            // Check if file exists and if the size is the same as original file
            if (asset->hasFieldName("asset_file_size")){
                QString full_filename = file_path + "/" + file_name;
                if (QFile::exists(full_filename)){
                    QFileInfo file_info(full_filename);
                    if (file_info.size() == asset->getFieldValue("asset_file_size").toLongLong()){
                        LOG_DEBUG("Skipping file : " + full_filename, "AssetsWidget::startAssetsDownload");
                        continue;
                    }
                }
            }
        }
        // Send download request
        if (asset->hasFieldName("asset_url")){
            QUrlQuery query;
            query.addQueryItem("access_token", asset->getFieldValue("access_token").toString());
            query.addQueryItem("asset_uuid", asset_uuid);
            m_comAssetManager->doDownload(asset->getFieldValue("asset_url").toUrl(),
                                          file_path, asset_uuid, file_name,
                                          query);
        }else{
            LOG_WARNING("No asset url for asset " + asset->getUuid() + " - skipping download.", "AssetsWidget::on_btnDownload_clicked");
        }
    }
}

void AssetsWidget::showTransferDialog()
{
    if (!m_transferDialog){
        m_transferDialog = new TransferProgressDialog(this);
        connect(m_transferDialog, &TransferProgressDialog::finished, this, &AssetsWidget::transferDialogCompleted);
        connect(m_transferDialog, &TransferProgressDialog::transferAbortRequested, this, &AssetsWidget::transferDialogAbortRequested);
        m_transferDialog->show();
    }
}

void AssetsWidget::setViewMode(const ViewMode &new_mode)
{
    m_viewMode = new_mode;

    ui->btnExpandAll->setVisible(m_viewMode == VIEWMODE_PARTICIPANT || m_viewMode == VIEWMODE_PROJECT);
}

void AssetsWidget::setLoading(const bool &loading, const QString &msg, const bool &hide_ui)
{
    ui->treeAssets->setEnabled(!loading);
    ui->frameButtons->setEnabled(!loading);

    if (hide_ui && loading){
        ui->treeAssets->setVisible(false);
        ui->frameButtons->setVisible(false);
    }else{
        ui->treeAssets->setVisible(true);
        ui->frameButtons->setVisible(true);
    }

    if (loading){
        ui->wdgMessages->addMessage(Message(Message::MESSAGE_WORKING, msg));
    }else{
        ui->wdgMessages->clearMessages();
    }
}

void AssetsWidget::processAssetsReply(QList<TeraData> assets, QUrlQuery reply_query)
{
    if (reply_query.hasQueryItem(WEB_QUERY_WITH_ONLY_TOKEN)){
        // Only update access token!
        foreach(TeraData asset, assets){
            QString asset_uuid = asset.getUuid();
            QString access_token = asset.getFieldValue("access_token").toString();
            if (m_assets.contains(asset_uuid)){
                m_assets[asset_uuid]->setFieldValue("access_token", access_token);
            }

            // Update token in all pending download requests
            m_comAssetManager->updateWaitingDownloadsQueryParameter(asset_uuid, "access_token", access_token);

            // Start refresh timer
            m_refreshTokenTimer.start();
        }
    }else{
        bool empty_assets = m_assets.isEmpty();
        foreach(TeraData asset, assets){
            // Participant name
            int id_participant = -1;
            if (reply_query.hasQueryItem(WEB_QUERY_ID_PARTICIPANT)){
                id_participant = reply_query.queryItemValue(WEB_QUERY_ID_PARTICIPANT).toInt();
            }
            updateAsset(asset, id_participant, !empty_assets); // No count change signal on new query
        }

        //resizeAssetsColumnsToContent();

        if (ui->treeAssets->topLevelItemCount()>0)
            ui->btnDownloadAll->setEnabled(true);

        ui->lblAssetsCount->setText(QString::number(m_assets.count()) + " " + tr("données"));
        ui->btnExpandAll->setEnabled(m_assets.count()<=100);
        if (!ui->btnExpandAll->isEnabled()){
            ui->btnExpandAll->setToolTip(tr("Trop de données - fonctionnalité désactivée"));
        }else{
            ui->btnExpandAll->setToolTip(tr("Tout voir"));
        }

        // Query extra information
        queryAssetsInfos();

        if (!m_refreshTokenTimer.isActive())
            m_refreshTokenTimer.start();
    }

}

void AssetsWidget::processServicesReply(QList<TeraData> services, QUrlQuery reply_query)
{
    for(const TeraData &service:services){
       if (service.getFieldValue("service_key").toString() == "FileTransferService"){
           // Check if project is in service_fields
           if (service.hasFieldName("service_projects")){
               QVariantList projects = service.getFieldValue("service_projects").toList();
               for (const QVariant &project:qAsConst(projects)){
                   QVariantMap project_info = project.toMap();
                   if (project_info["id_project"].toInt() == m_idProject){
                       // Ok, we are allowed to use file transfer service
                       if (m_fileTransferServiceInfos)
                           delete m_fileTransferServiceInfos;
                       m_fileTransferServiceInfos = new TeraData(service);
                       ui->btnNew->setVisible(true);
                       break;
                   }
               }
           }
       }
    }
}

void AssetsWidget::nextMessageWasShown(Message current_message)
{
    if (current_message.getMessageType()==Message::MESSAGE_NONE){
        ui->wdgMessages->hide();
    }else{
        ui->wdgMessages->show();
    }
}

void AssetsWidget::assetComNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code)
{
    Q_UNUSED(error)

    if (error == QNetworkReply::OperationCanceledError && op == QNetworkAccessManager::PostOperation){
        // Transfer was cancelled by user - no need to alert anyone!
        return;
    }

    if (m_transferDialog)
        return; // Will be handled by that dialog

    if (error_msg.endsWith('\n'))
        error_msg = error_msg.left(error_msg.length()-1);

    //error_msg = QTextDocumentFragment::fromHtml(error_msg).toPlainText();

    QString error_str;

    if (status_code > 0)
        error_str = tr("Erreur HTTP ") + QString::number(status_code) + ": " + error_msg;
    else
        error_str = tr("Erreur ") + QString::number(error) + ": " + error_msg;

    error_msg = error_msg.replace('\n', " - ");
    error_msg = error_msg.replace('\r', "");
    ui->wdgMessages->addMessage(Message(Message::MESSAGE_ERROR, error_msg));
}

void AssetsWidget::assetComUploadProgress(UploadingFile *file)
{
    if (!m_transferDialog){
        // New upload request - create dialog and add file
        showTransferDialog();
    }
    m_transferDialog->updateTransferringFile(file);
}

void AssetsWidget::assetComUploadCompleted(UploadingFile *file)
{
    if (m_transferDialog){
        if (m_transferDialog->transferFileCompleted(file)){
            // If we are here, no more uploads are pending. Close transfer dialog.
            transferDialogCompleted();
        }
    }
}

void AssetsWidget::assetComDownloadProgress(DownloadingFile *file)
{
    if (!m_transferDialog){
        // New download request - create dialog and add file
        showTransferDialog();
    }
    m_transferDialog->updateTransferringFile(file);
}

void AssetsWidget::assetComDownloadCompleted(DownloadingFile *file)
{
    if (m_transferDialog){
        if (m_transferDialog->transferFileCompleted(file)){
            // If we are here, no more downloads are pending. Close transfer dialog.
            transferDialogCompleted();
        }
    }
}

void AssetsWidget::assetComTransferAborted(TransferringFile *file)
{
    /*if (m_transferDialog){
        m_transferDialog->deleteLater();
        m_transferDialog = nullptr;
    }*/
    if (m_transferDialog){
        m_transferDialog->transferFileAborted(file);
    }
}

void AssetsWidget::transferDialogCompleted()
{

    if (m_transferDialog){
        // Inform user
        if (!m_transferDialog->hasErrors() && !m_transferDialog->hasAborted())
            ui->wdgMessages->addMessage(Message(Message::MESSAGE_OK, tr("Transfert de données complété")));

        //m_transferDialog->close();
        m_transferDialog->deleteLater();
        m_transferDialog = nullptr;
    }

    // Clear all selection
    ui->treeAssets->clearSelection();
}

void AssetsWidget::transferDialogAbortRequested()
{
    m_comAssetManager->abortTransfers();
}

void AssetsWidget::assetComDeleteOK(QString path, QString uuid)
{
    if (uuid.isEmpty())
        return;

    if (path.endsWith("/assets")){
        // Remove asset
        QTreeWidgetItem* asset_item = m_treeAssets[uuid];
        if (asset_item){
            m_treeAssets.remove(uuid);
            delete m_assets.take(uuid);
            delete asset_item;

            // Emit update signal
            emit assetCountChanged(m_assets.count());
            ui->btnDownloadAll->setEnabled(!isEmpty());
            //ui->wdgMessages->addMessage(Message(Message::MESSAGE_OK, tr("Suppression complétée")));
        }
    }

}

void AssetsWidget::assetComPostOK(QString path)
{
    if (!path.endsWith("/assets/infos"))
        ui->wdgMessages->addMessage(Message(Message::MESSAGE_OK, tr("Données sauvegardées")));
}

void AssetsWidget::processAssetsInfos(QList<QJsonObject> infos, QUrlQuery reply_query, QString reply_path)
{
    for (const QJsonObject &asset_info:qAsConst(infos)){
        QString asset_uuid = asset_info["asset_uuid"].toString();

        if (!m_treeAssets.contains(asset_uuid)){
            // Create a new asset, if possible (should happen if we posted a new asset)
            TeraData asset(TERADATA_ASSET, asset_info);
            updateAsset(asset); // TODO: manage participant association, if required by the view
        }

        if (m_treeAssets.contains(asset_uuid)){
            QTreeWidgetItem* asset_item = m_treeAssets[asset_uuid];

            if (asset_info.contains("asset_file_size")){

                asset_item->setText(AssetColumn::ASSET_SIZE, Utils::formatFileSize(asset_info["asset_file_size"].toInt()));
                ui->treeAssets->showColumn(AssetColumn::ASSET_SIZE);
            }

            if (asset_info.contains("file_size")){
                asset_item->setText(AssetColumn::ASSET_SIZE, Utils::formatFileSize(asset_info["file_size"].toInt()));
                ui->treeAssets->showColumn(AssetColumn::ASSET_SIZE);
            }

            if (asset_info.contains("video_duration")){
                asset_item->setText(AssetColumn::ASSET_DURATION, Utils::formatDuration(asset_info["video_duration"].toString()));
                ui->treeAssets->showColumn(AssetColumn::ASSET_DURATION);
            }
        }

        if (m_assets.contains(asset_uuid)){
            // Append received fields to current asset
            TeraData* asset = m_assets[asset_uuid];
            asset->updateFrom(asset_info);
        }
    }
    resizeAssetsColumnsToContent();

    setLoading(false);
}

void AssetsWidget::refreshAccessToken()
{
    if (!m_comManager)
        return;

    QUrlQuery query = m_dataQuery; // Original data query

    query.addQueryItem(WEB_QUERY_WITH_ONLY_TOKEN, "1");
    m_comManager->doGet(WEB_ASSETINFO_PATH, query);
}

void AssetsWidget::on_treeAssets_itemSelectionChanged()
{
    bool at_least_one_asset_selected = false;

    // Make sure we don't have only "labels" items (session, participant, ...) selected
    const QList<QTreeWidgetItem*> selected_items = ui->treeAssets->selectedItems();
    for(QTreeWidgetItem* item:selected_items){
        if (!m_treeAssets.key(item).isEmpty() ){
            at_least_one_asset_selected = true;
            break;
        }
    }
    ui->btnDelete->setEnabled(at_least_one_asset_selected);
    ui->btnDownload->setEnabled(at_least_one_asset_selected);
}


void AssetsWidget::on_btnNew_clicked()
{
    // Upload a new asset to the FileTransfer service
    if (!m_fileTransferServiceInfos || !m_comAssetManager || !m_comManager){
        return; // Should not happen at this point.
    }

    // Open upload dialog
    m_uploadDialog = new FileUploaderDialog(tr("Tous (*.*)"), dynamic_cast<QWidget*>(this));

    connect(m_uploadDialog, &FileUploaderDialog::finished, this, &AssetsWidget::fileUploaderFinished);

    m_uploadDialog->setModal(true);
    m_uploadDialog->show();


}

void AssetsWidget::fileUploaderFinished(int result)
{
    if (result == QDialog::Accepted){
        // Do the upload process!
        QStringList files = m_uploadDialog->getFiles();
        QStringList labels = m_uploadDialog->getLabels();

        // Create JSON
        QJsonDocument doc;
        QJsonObject data_obj;
        data_obj["id_session"] = m_idSession;

        // Get path to upload to
        if (!m_fileTransferServiceInfos)
            return; // Shouldn't happen.

        QString path = m_fileTransferServiceInfos->getFieldValue("service_clientendpoint").toString() + "/api/assets";

        for(int i=0; i<files.count(); i++){
            data_obj["asset_name"] = labels.at(i);
            doc.setObject(data_obj);
            m_comAssetManager->doUploadWithMultiPart(path, files.at(i), "file_asset", doc.toJson());
        }

    }

    m_uploadDialog->deleteLater();
    m_uploadDialog = nullptr;
}


void AssetsWidget::on_btnDelete_clicked()
{
    // Check if the sender is a QToolButton (from the action column)
    QToolButton* action_btn = dynamic_cast<QToolButton*>(sender());
    if (action_btn && action_btn != ui->btnDelete){
        // Select row according to the session id of that button
        QString asset_uuid = action_btn->property("asset_uuid").toString();
        QTreeWidgetItem* asset_item = m_treeAssets[asset_uuid];
        ui->treeAssets->clearSelection();
        if (asset_item)
            asset_item->setSelected(true);
    }

    if (ui->treeAssets->selectedItems().count()==0)
        return;

    GlobalMessageBox diag;
    QMessageBox::StandardButton answer = QMessageBox::No;
    if (ui->treeAssets->selectedItems().count() == 1){
        QTreeWidgetItem* base_item = ui->treeAssets->selectedItems().first();
        answer = diag.showYesNo(tr("Suppression?"), tr("Êtes-vous sûrs de vouloir supprimer """) + base_item->text(ASSET_NAME) + """?");
    }else{
        answer = diag.showYesNo(tr("Suppression?"), tr("Êtes-vous sûrs de vouloir supprimer toutes les données sélectionnées?"));
    }

    if (answer == QMessageBox::Yes){
        // We must delete!
        const QList<QTreeWidgetItem*> items = ui->treeAssets->selectedItems();
        for(QTreeWidgetItem* asset:items){
            QString asset_uuid = m_treeAssets.key(asset);
            QString asset_url_str = m_assets[asset_uuid]->getFieldValue("asset_url").toString();
            QString access_token =  m_assets[asset_uuid]->getFieldValue("access_token").toString();
            QUrl asset_url(asset_url_str);
            m_comAssetManager->doDelete(asset_url.url(QUrl::RemoveQuery), asset_uuid, access_token);
        }
    }
}




void AssetsWidget::on_btnDownload_clicked()
{
    // Check if the sender is a QToolButton (from the action column)
    QToolButton* action_btn = dynamic_cast<QToolButton*>(sender());
    QStringList assets_to_download;
    if (action_btn && action_btn != ui->btnDownload){
        // Select row according to the session id of that button
        QString asset_uuid = action_btn->property("asset_uuid").toString();
        assets_to_download.append(asset_uuid);
    }else{
        // Download all selected assets
        const QList<QTreeWidgetItem*> items = ui->treeAssets->selectedItems();
        for(QTreeWidgetItem* asset:items){
            assets_to_download.append(m_treeAssets.key(asset));
        }
    }

    startAssetsDownload(assets_to_download);

}

void AssetsWidget::on_btnDownloadAll_clicked()
{
    QStringList assets_to_download;

    // Download all assets
    for(TeraData* asset:qAsConst(m_assets)){
        assets_to_download.append(asset->getUuid());
    }

    startAssetsDownload(assets_to_download);
}


void AssetsWidget::on_treeAssets_itemExpanded(QTreeWidgetItem *item)
{
    if (item->childCount() == 0){
        int id_session = m_treeSessions.key(item, -1);

        if (id_session != -1){
            // Display all assets for that session
            setLoading(true, tr("Affichage des données en cours..."));
            QCoreApplication::processEvents();
            QStringList assets_uuids = m_assetsSessions.values(id_session);
            for (const QString &asset_uuid: qAsConst(assets_uuids)){
                if (m_assets.contains(asset_uuid)){
                    updateAsset(*m_assets[asset_uuid]);
                }
            }
            setLoading(false);
        }
    }
    resizeAssetsColumnsToContent();
}


void AssetsWidget::on_treeAssets_itemCollapsed(QTreeWidgetItem *item)
{
    if (item->childCount() > 0){
        int id_session = m_treeSessions.key(item, -1);

        if (id_session != -1){
            // Remove all assets from display
            QList<QTreeWidgetItem*> assets_items = item->takeChildren();
            for(QTreeWidgetItem* asset_item: qAsConst(assets_items)){
                QString asset_uuid = m_treeAssets.key(asset_item);
                m_treeAssets.remove(asset_uuid);
                delete asset_item;
            }
        }
    }

    resizeAssetsColumnsToContent();

}


void AssetsWidget::on_btnExpandAll_clicked()
{
    if (ui->btnExpandAll->isChecked()){
        ui->treeAssets->expandAll();
        ui->btnExpandAll->setToolTip(tr("Tout masquer"));
    }else{
        ui->treeAssets->collapseAll();
        ui->btnExpandAll->setToolTip(tr("Tout voir"));
    }
}

