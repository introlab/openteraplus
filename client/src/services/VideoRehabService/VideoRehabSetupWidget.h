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
    void connectSignals();

    void selectVideoSrcByName(const QString& name);
    void selectAudioSrcByName(const QString& name);

    void setLoading(const bool& loading);


private slots:
    void webPageLoaded(bool ok);
    void webPageReady();
    void webPageCameraChanged();

    void processServiceConfigsReply(QList<TeraData> configs, QUrlQuery query);
    void refreshWebpageSettings();
};

#endif // VIDEOREHABSETUPWIDGET_H
