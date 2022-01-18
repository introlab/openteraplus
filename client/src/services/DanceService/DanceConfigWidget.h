#ifndef DANCECONFIGWIDGET_H
#define DANCECONFIGWIDGET_H

#include <QWidget>
#include <QTextDocumentFragment>
#include <QInputDialog>

#include "managers/ComManager.h"
#include "dialogs/FileUploaderDialog.h"
#include "dialogs/TransferProgressDialog.h"

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
    explicit DanceConfigWidget(ComManager* comManager, int projectId, int participantId = -1, QWidget *parent = nullptr);
    ~DanceConfigWidget();

private:
    Ui::DanceConfigWidget   *ui;
    FileUploaderDialog*     m_uploadDialog;
    TransferProgressDialog* m_transferDialog;

    ComManager* m_comManager;
    int         m_idProject;
    int         m_idParticipant;

    DanceComManager* m_danceComManager;

    void connectSignals();

private slots:
    void processVideosReply(QList<QJsonObject> videos);
    void handleNetworkError(QNetworkReply::NetworkError, QString, QNetworkAccessManager::Operation op, int status_code);

    void fileUploaderFinished(int result);

    void danceComUploadProgress(UploadingFile* file);
    void danceComUploadCompleted(UploadingFile* file);
    void danceComTransferAborted(TransferringFile *file);
    void danceComDeleteOK(QString path, int id);

    void on_tabMain_currentChanged(int index);
    void on_btnUpload_clicked();

    void on_lstVideos_itemSelectionChanged();
    void on_btnDelete_clicked();
    void on_btnRename_clicked();
};

#endif // DANCECONFIGWIDGET_H
