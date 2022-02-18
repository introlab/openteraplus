#ifndef ASSETSWIDGET_H
#define ASSETSWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>

#include "managers/ComManager.h"
#include "managers/AssetComManager.h"

#include "dialogs/FileUploaderDialog.h"
#include "dialogs/TransferProgressDialog.h"

#include "data/Message.h"
#include "Utils.h"

#include "TeraAsset.h"

namespace Ui {
class AssetsWidget;
}

class AssetsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AssetsWidget(ComManager* comMan = nullptr, QWidget *parent = nullptr);
    ~AssetsWidget();

    void setComManager(ComManager* comMan);
    void setAssociatedProject(const int &id_project);

    bool isEmpty() const;
    void clearAssets();
    void enableNewAssets(const bool &enable);

    void displayAssetsForSession(const int &id_session);

private:
    typedef enum {
        ASSET_PARTICIPANT=0,
        ASSET_SESSION=1,
        ASSET_NAME=2,
        ASSET_DATETIME=3,
        ASSET_SERVICE=4,
        ASSET_SIZE=5,
        ASSET_DURATION=6}
    AssetColumn;

    Ui::AssetsWidget *ui;

    ComManager*         m_comManager;
    AssetComManager*    m_comAssetManager;

    FileUploaderDialog*     m_uploadDialog;
    TransferProgressDialog* m_transferDialog;
    QFileDialog             m_fileDialog; // Defined here to remember last directory

    int                 m_idProject;
    int                 m_idSession;
    TeraData*           m_fileTransferServiceInfos;
    QString             m_accessToken;
    QTimer              m_refreshTokenTimer;
    QUrlQuery           m_dataQuery;

    QMap<QString, QTreeWidgetItem*>     m_treeAssets;
    QMap<int, QTreeWidgetItem*>         m_treeSessions;
    QMap<int, QTreeWidgetItem*>         m_treeParticipants;
    QMap<QString, TeraData*>            m_assets;

    void connectSignals();

    void resizeAssetsColumnsToContent();

    bool eventFilter(QObject* o, QEvent* e);

    void queryAssetsInfos();
    void updateAsset(const TeraData& asset, const int& id_participant = -1, const bool& emit_count_update_signal=true);

    QString getRelativePathForAsset(const QString &uuid);
    QString getFilenameForAsset(const QString &uuid);

    void startAssetsDownload(const QStringList& assets_to_download);


private slots:
    void processAssetsReply(QList<TeraData> assets, QUrlQuery reply_query);
    void processAssetAccessTokenReply(QString asset_token, QUrlQuery reply_query);

    void processServicesReply(QList<TeraData> services, QUrlQuery reply_query);

    void nextMessageWasShown(Message current_message);
    void fileUploaderFinished(int result);

    void assetComNetworkError(QNetworkReply::NetworkError, QString, QNetworkAccessManager::Operation op, int status_code);
    void assetComUploadProgress(UploadingFile* file);
    void assetComUploadCompleted(UploadingFile* file);
    void assetComDownloadProgress(DownloadingFile* file);
    void assetComDownloadCompleted(DownloadingFile* file);
    void assetComTransferAborted(TransferringFile* file);
    void transferDialogCompleted();

    void assetComDeleteOK(QString path, QString uuid);
    void assetComPostOK(QString path);

    void processAssetsInfos(QList<QJsonObject> infos, QUrlQuery reply_query, QString reply_path);

    void refreshAccessToken();

    void on_treeAssets_itemSelectionChanged();
    void on_btnNew_clicked();
    void on_btnDelete_clicked();

    void on_btnDownload_clicked();

    void on_btnDownloadAll_clicked();

signals:
    void assetCountChanged(int new_count);

};

#endif // ASSETSWIDGET_H
