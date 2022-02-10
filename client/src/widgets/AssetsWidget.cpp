#include "AssetsWidget.h"
#include "ui_AssetsWidget.h"

AssetsWidget::AssetsWidget(ComManager *comMan, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AssetsWidget)
{
    ui->setupUi(this);

    m_comAssetManager = nullptr;
    setComManager(comMan);

    // Hide add button by default
    ui->btnNew->hide();
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

    ui->treeAssets->hideColumn(0); // No need to display the first column (participant) for that case
    ui->treeAssets->hideColumn(1); // No need to display the second column (session) for that case

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
            services_assets.insert(asset->getFieldValue("asset_infos_url").toString(), asset->getUuid());
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
        item->setText(2, asset.getName());
        item->setIcon(2, QIcon(TeraAsset::getIconForContentType(asset.getFieldValue("asset_type").toString())));
        item->setForeground(2, QColor(255,255,102));
        QFont font = item->font(2);
        font.setBold(true);
        item->setFont(2, font);

        item->setText(3, asset.getFieldValue("asset_datetime").toDateTime().toLocalTime().toString("dd-MM-yyyy hh:mm:ss"));
        if (asset.hasFieldName("asset_service_owner_name"))
            item->setText(4, asset.getFieldValue("asset_service_owner_name").toString());

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
        m_treeAssets.insert(asset.getId(), item);
        m_assets.insert(asset.getId(), new TeraData(asset));
    }

    resizeAssetsColumnsToContent();

    // Query extra information
    queryAssetsInfos();

}
