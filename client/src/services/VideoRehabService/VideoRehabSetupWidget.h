#ifndef VIDEOREHABSETUPWIDGET_H
#define VIDEOREHABSETUPWIDGET_H

#include <QWidget>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QHBoxLayout>
#include <QComboBox>

#include <QAudioDeviceInfo>
#include <QCameraInfo>

#include "VideoRehabWebPage.h"
#include "ComManager.h"
#include "Utils.h"


namespace Ui {
class VideoRehabSetupWidget;
}

class VideoRehabSetupWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoRehabSetupWidget(ComManager* comManager, QWidget *parent = nullptr);
    ~VideoRehabSetupWidget();

private:
    Ui::VideoRehabSetupWidget *ui;

    ComManager*             m_comManager;
    QWebEngineView*         m_webEngine;
    VideoRehabWebPage*      m_webPage;

    void initUI();
    void refreshAudioVideoDevices();
    void connectSignals();

    void selectVideoSrcByName(const QString& name);
    void selectAudioSrcByName(const QString& name);

    void setLoading(const bool& loading);
    void showError(const QString& title, const QString& context, const QString& error);


private slots:
    void webPageLoaded(bool ok);
    void webPageReady();
    void webPageCameraChanged();
    void webPageVideoError(QString context, QString error);
    void webPageAudioError(QString context, QString error);
    void webPageGeneralError(QString context, QString error);

    void processServiceConfigsReply(QList<TeraData> configs, QUrlQuery query);
    void refreshWebpageSettings();
    void on_btnRefresh_clicked();
};

#endif // VIDEOREHABSETUPWIDGET_H
