#ifndef VIDEOREHABSETUPWIDGET_H
#define VIDEOREHABSETUPWIDGET_H

#include <QWidget>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QHBoxLayout>

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
};

#endif // VIDEOREHABSETUPWIDGET_H
