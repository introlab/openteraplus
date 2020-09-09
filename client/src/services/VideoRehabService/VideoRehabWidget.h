#ifndef VIDEOREHABWIDGET_H
#define VIDEOREHABWIDGET_H

#include "services/BaseServiceWidget.h"
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QHBoxLayout>

#include "VideoRehabWebPage.h"

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

private slots:
    void on_txtURL_returnPressed();
    void webEngineURLChanged(QUrl url);

private:
    void connectSignals();

    Ui::VideoRehabWidget*   ui;
    QWebEngineView*         m_webEngine;
    VideoRehabWebPage*      m_webPage;
};

#endif // VIDEOREHABWIDGET_H
