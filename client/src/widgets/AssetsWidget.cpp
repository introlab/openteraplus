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
    setComManager(comMan);

    connectSignals();

    // Hide add button by default
    ui->btnNew->hide();
    ui->wdgMessages->hide();

    // Columns not used for general assets
    ui->treeAssets->hideColumn(AssetColumn::ASSET_SIZE);
    ui->treeAssets->hideColumn(AssetColumn::ASSET_DURATION);
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
        //connect(m_comAssetManager, &AssetComManager::uploadProgress, this, &AssetsWidget::assetComUploadProgress);
        //connect(m_comAssetManager, &AssetComManager::uploadCompleted, this, &AssetsWidget::assetComUploadCompleted);
        //connect(m_comAssetManager, &AssetComManager::transferAborted, this, &AssetsWidget::assetComTransferAborted);
        connect(m_comAssetManager, &AssetComManager::deleteResultsOK, this, &AssetsWidget::assetComDeleteOK);
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
    }
    // Default - no setted id project or enable = false
    ui->btnNew->setVisible(false);
}

void AssetsWidget::displayAssetsForSession(const int &id_session)
{
    if (!m_comManager)
        return;

    clearAssets();

    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(id_session));
    query.addQueryItem(WEB_QUERY_WITH_URLS, "1");
    m_comManager->doGet(WEB_ASSETINFO_PATH, query);

    ui->treeAssets->hideColumn(AssetColumn::ASSET_PARTICIPANT); // No need to display the first column (participant) for that case
    ui->treeAssets->hideColumn(AssetColumn::ASSET_SESSION); // No need to display the second column (session) for that case

    m_idSession = id_session;

}

void AssetsWidget::connectSignals()
{
    // Com related signals are set in setComManager
    connect(ui->wdgMessages, &ResultMessageWidget::nextMessageShown, this, &AssetsWidget::nextMessageWasShown);
}

void AssetsWidget::resizeAssetsColumnsToContent()
{
  for (int i=0; i<ui->treeAssets->columnCount(); i++)
      ui->treeAssets->resizeColumnToContents(i);
}

void AssetsWidget::queryAssetsInfos()
{
    QMultiMap<QString, QString> services_assets;
    QString access_token;

    // Group each assets by access url
    foreach(TeraData* asset, m_assets){
        if (asset->hasFieldName("access_token")){
            access_token = asset->getFieldValue("access_token").toString();
        }
        if (asset->hasFieldName("asset_infos_url")){
            QString asset_info_str = asset->getFieldValue("asset_infos_url").toString();
            if (!asset_info_str.isEmpty()){
                QUrl asset_info_url = QUrl(asset_info_str);
                services_assets.insert(asset_info_url.url(QUrl::RemoveQuery), asset->getUuid());
            }
        }
    }

    // Query each service for their assets
    QStringList service_urls = services_assets.uniqueKeys();
    foreach(QString service_url, service_urls){
        QStringList asset_uuids = services_assets.values(service_url);
        QJsonDocument document;
        QJsonObject base_obj;

        base_obj.insert("access_token", access_token);
        base_obj.insert("asset_uuids", QJsonArray::fromStringList(asset_uuids));

        document.setObject(base_obj);

        m_comAssetManager->doPost(QUrl(service_url), document.toJson(), true);
    }
}

void AssetsWidget::updateAsset(const TeraData &asset, const int &id_participant)
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

    if (m_treeAssets.contains(asset.getUuid())){
        item = m_treeAssets[asset.getUuid()];
        btnDownload = dynamic_cast<QToolButton*>(ui->treeAssets->itemWidget(item, ui->treeAssets->columnCount()-1)->layout()->itemAt(2)->widget());
        btnView = dynamic_cast<QToolButton*>(ui->treeAssets->itemWidget(item, ui->treeAssets->columnCount()-1)->layout()->itemAt(0)->widget());
    }else{
        item = new QTreeWidgetItem();
        QTreeWidgetItem* parent_item = nullptr;

        // Participant name
        if (id_participant>=0){
            if (m_treeParticipants.contains(id_participant)){
                parent_item = m_treeParticipants[id_participant];
            }
        }

        // Session name
        int id_session = asset.getFieldValue("id_session").toInt();
        if (m_treeSessions.contains(id_session)){
            parent_item = m_treeSessions[id_session];
        }

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
        //connect(btnDelete, &QToolButton::clicked, this, &ParticipantWidget::btnDeleteSession_clicked);
        layout->addWidget(btnDelete);

        // Download data
        btnDownload = new QToolButton();
        btnDownload->setIcon(QIcon(":/icons/download.png"));
        btnDownload->setIconSize(QSize(24,24));
        btnDownload->setProperty("asset_uuid", asset.getUuid());
        btnDownload->setCursor(Qt::PointingHandCursor);
        btnDownload->setMaximumWidth(32);
        btnDownload->setToolTip(tr("Télécharger les données"));
        //connect(btnDownload, &QToolButton::clicked, this, &ParticipantWidget::btnDownloadSession_clicked);
        layout->addWidget(btnDownload);

        ui->treeAssets->setItemWidget(item, ui->treeAssets->columnCount()-1, action_frame);

        // Update internal lists
        m_treeAssets.insert(asset.getUuid(), item);
        m_assets.insert(asset.getUuid(), new TeraData(asset));
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

void AssetsWidget::processAssetsReply(QList<TeraData> assets, QUrlQuery reply_query)
{
    foreach(TeraData asset, assets){
        // Participant name
        int id_participant = -1;
        if (reply_query.hasQueryItem(WEB_QUERY_ID_PARTICIPANT)){
            id_participant = reply_query.queryItemValue(WEB_QUERY_ID_PARTICIPANT).toInt();
        }
        updateAsset(asset, id_participant);
    }

    //resizeAssetsColumnsToContent();

    if (ui->treeAssets->topLevelItemCount()>0)
        ui->btnDownload->setEnabled(true);

    // Query extra information
    queryAssetsInfos();

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

    if (error_msg.endsWith('\n'))
        error_msg = error_msg.left(error_msg.length()-1);

    error_msg = QTextDocumentFragment::fromHtml(error_msg).toPlainText();

    QString error_str;

    if (status_code > 0)
        error_str = tr("Erreur HTTP ") + QString::number(status_code) + ": " + error_msg;
    else
        error_str = tr("Erreur ") + QString::number(error) + ": " + error_msg;

    /*GlobalMessageBox msg;
    msg.showError(tr("Télédanse - Erreur"), error_msg);*/
    error_msg = error_msg.replace('\n', " - ");
    error_msg = error_msg.replace('\r', "");
    ui->wdgMessages->addMessage(Message(Message::MESSAGE_ERROR, error_msg));
}

void AssetsWidget::assetComDeleteOK(QString path, int id)
{
    ui->wdgMessages->addMessage(Message(Message::MESSAGE_OK, tr("Suppression complétée")));
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
    }
    resizeAssetsColumnsToContent();
}

void AssetsWidget::on_treeAssets_itemSelectionChanged()
{
    ui->btnDelete->setEnabled(!ui->treeAssets->selectedItems().isEmpty());
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

