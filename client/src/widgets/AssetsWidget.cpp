#include "AssetsWidget.h"
#include "ui_AssetsWidget.h"

AssetsWidget::AssetsWidget(ComManager *comMan, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AssetsWidget)
{
    ui->setupUi(this);

    m_comAssetManager = nullptr;
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
    qDeleteAll(m_assets);
}

void AssetsWidget::setComManager(ComManager *comMan)
{
    m_comManager = comMan;

    if (m_comManager){
        connect(m_comManager, &ComManager::assetsReceived, this, &AssetsWidget::processAssetsReply);

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
            QUrl asset_info_url = asset->getFieldValue("asset_infos_url").toString();
            services_assets.insert(asset_info_url.url(QUrl::RemoveQuery), asset->getUuid());
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

void AssetsWidget::processAssetsReply(QList<TeraData> assets, QUrlQuery reply_query)
{
    foreach(TeraData asset, assets){
        QTreeWidgetItem* item = new QTreeWidgetItem();
        QTreeWidgetItem* parent_item = nullptr;

        // Participant name
        if (reply_query.hasQueryItem(WEB_QUERY_ID_PARTICIPANT)){
            int id_participant = reply_query.queryItemValue(WEB_QUERY_ID_PARTICIPANT).toInt();
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
        item->setText(AssetColumn::ASSET_NAME, asset.getName());
        item->setIcon(AssetColumn::ASSET_NAME, QIcon(TeraAsset::getIconForContentType(asset.getFieldValue("asset_type").toString())));
        item->setForeground(AssetColumn::ASSET_NAME, QColor(255,255,102));
        QFont font = item->font(AssetColumn::ASSET_NAME);
        font.setBold(true);
        item->setFont(AssetColumn::ASSET_NAME, font);

        item->setText(AssetColumn::ASSET_DATETIME, asset.getFieldValue("asset_datetime").toDateTime().toLocalTime().toString("dd-MM-yyyy hh:mm:ss"));
        if (asset.hasFieldName("asset_service_owner_name"))
            item->setText(AssetColumn::ASSET_SERVICE, asset.getFieldValue("asset_service_owner_name").toString());

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
        QToolButton* btnView = new QToolButton();
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
        QToolButton* btnDownload = new QToolButton();
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

    //resizeAssetsColumnsToContent();

    // Query extra information
    queryAssetsInfos();

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
    //ui->wdgMessages->addMessage(Message(Message::MESSAGE_OK, tr("Données sauvegardées")));
}

void AssetsWidget::processAssetsInfos(QList<QJsonObject> infos, QUrlQuery reply_query, QString reply_path)
{
    for (const QJsonObject &asset_info:qAsConst(infos)){
        QString asset_uuid = asset_info["asset_uuid"].toString();

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
