#ifndef DANCECONFIGWIDGET_H
#define DANCECONFIGWIDGET_H

#include <QWidget>
#include <QTextDocumentFragment>
#include <QInputDialog>
#include <QListWidgetItem>
#include <QStyledItemDelegate>

#include "managers/ComManager.h"
#include "dialogs/FileUploaderDialog.h"
#include "dialogs/TransferProgressDialog.h"
#include "data/Message.h"

#include "GlobalMessageBox.h"

#include "DanceComManager.h"
#include "WebAPI.h"

namespace Ui {
class DanceConfigWidget;
}

class DanceConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DanceConfigWidget(ComManager* comManager, int projectId, QString participantUuid = QString(), QWidget *parent = nullptr);
    ~DanceConfigWidget();

private:
    Ui::DanceConfigWidget   *ui;
    FileUploaderDialog*     m_uploadDialog;
    TransferProgressDialog* m_transferDialog;

    ComManager* m_comManager;
    int         m_idProject;
    QString     m_uuidParticipant;

    QList<int>  m_playlistIds;

    DanceComManager* m_danceComManager;

    void connectSignals();
    void updateVideoInLibrary(const QJsonObject &video);
    void updateVideoInPlaylist(const QJsonObject &video);

    void queryVideoLibrary();
    void queryPlaylist();

    void moveVideoItemToAvailable(QTableWidgetItem* item);
    void moveVideoItemToPlaylist(QListWidgetItem* item);
    void copyVideoItemToPlaylist(QListWidgetItem* item);

    void checkDirty();

private slots:
    void processVideosReply(QList<QJsonObject> videos);
    void processPlaylistReply(QList<QJsonObject> playlists);
    void processSessionTypesReply(QList<TeraData> session_types);

    void handleNetworkError(QNetworkReply::NetworkError, QString, QNetworkAccessManager::Operation op, int status_code);

    void fileUploaderFinished(int result);

    void danceComUploadProgress(UploadingFile* file);
    void danceComUploadCompleted(UploadingFile* file);
    void danceComTransferAborted(TransferringFile *file);
    void danceComDeleteOK(QString path, int id);
    void danceComPostOK(QString path);

    void nextMessageWasShown(Message current_message);

    void on_tabMain_currentChanged(int index);
    void on_btnUpload_clicked();

    void on_lstVideos_itemSelectionChanged();
    void on_btnDelete_clicked();
    void on_btnRename_clicked();
    void on_lstAvailVideos_itemDoubleClicked(QListWidgetItem *item);
    void on_lstAvailVideos_itemSelectionChanged();
    void on_btnMoveUp_clicked();
    void on_btnAddVideo_clicked();
    void on_btnDelVideo_clicked();
    void on_btnMoveDown_clicked();
    void on_lstPlaylist_itemSelectionChanged();
    void on_lstPlaylist_itemDoubleClicked(QTableWidgetItem *item);
    void on_btnSave_clicked();
    void on_cmbSessionTypes_currentIndexChanged(int index);
    void on_btnCancel_clicked();
};

#endif // DANCECONFIGWIDGET_H
