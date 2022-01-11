#ifndef DANCECONFIGWIDGET_H
#define DANCECONFIGWIDGET_H

#include <QWidget>
#include "managers/ComManager.h"
#include "dialogs/FileUploaderDialog.h"

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
    explicit DanceConfigWidget(ComManager* comManager, int projectId, QWidget *parent = nullptr);
    ~DanceConfigWidget();

private slots:
    void processVideosReply(QList<QJsonObject> videos);
    void handleNetworkError(QNetworkReply::NetworkError, QString, QNetworkAccessManager::Operation op, int status_code);

    void fileUploaderFinished(int result);

    void on_tabMain_currentChanged(int index);
    void on_btnUpload_clicked();

private:
    Ui::DanceConfigWidget   *ui;
    FileUploaderDialog*     m_uploadDialog;

    ComManager* m_comManager;
    int         m_idProject;

    DanceComManager* m_danceComManager;

    void connectSignals();
};

#endif // DANCECONFIGWIDGET_H
