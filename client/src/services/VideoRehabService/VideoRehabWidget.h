#ifndef VIDEOREHABWIDGET_H
#define VIDEOREHABWIDGET_H

#include "services/BaseServiceWidget.h"
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QHBoxLayout>
#include <QMovie>
#include <QWebEngineSettings>

#include "VideoRehabWebPage.h"

#include "Utils.h"
//#include "VirtualCameraThread.h"
#include "JoinSessionEvent.pb.h"

namespace Ui {
class VideoRehabWidget;
}

class VideoRehabWidget : public BaseServiceWidget
{
    Q_OBJECT

public:
    explicit VideoRehabWidget(ComManager* comMan, QWidget *parent = nullptr);
    ~VideoRehabWidget();

    void initUI();

    bool handleJoinSessionEvent(const JoinSessionEvent &event) override;

    void reload();

    void startRecording();
    void stopRecording();
    void pauseRecording();

    void setDataSavePath() override; // Will update download path based on the user settings

private slots:
    void on_txtURL_returnPressed();
    void webEngineURLChanged(QUrl url);
    void webEngineDownloadRequested(QWebEngineDownloadRequest* item);
    void webEngineDownloadCompleted();

    void webPageLoaded(bool ok);
    void webPageReady();
    void webPageVideoError(QString context, QString error);
    void webPageAudioError(QString context, QString error);
    void webPageGeneralError(QString context, QString error);

    void virtualCameraDisconnected();

    void on_btnRefresh_clicked();

private:
    void connectSignals();
    void refreshWebpageSettings();

    void processSessionConfig();

    void setLoading(const bool& loading);
    void showError(const QString& title, const QString& context, const QString& error);

    void startVirtualCamera(const QString& src);
    void stopVirtualCamera();

    // UI items
    Ui::VideoRehabWidget*   ui;
    QWebEngineView*         m_webEngine;
    VideoRehabWebPage*      m_webPage;
    QMovie*                 m_loadingIcon;
   // VirtualCameraThread*    m_virtualCamThread;

 signals:
    void fileDownloading(bool downloading);

     // QWidget interface
 protected:
     void resizeEvent(QResizeEvent *event) override;
};

#endif // VIDEOREHABWIDGET_H
