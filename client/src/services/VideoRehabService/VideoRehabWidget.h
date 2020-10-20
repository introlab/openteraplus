#ifndef VIDEOREHABWIDGET_H
#define VIDEOREHABWIDGET_H

#include "services/BaseServiceWidget.h"
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QHBoxLayout>
#include <QMovie>
#include <QWebEngineSettings>

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

    void reload();

private slots:
    void on_txtURL_returnPressed();
    void webEngineURLChanged(QUrl url);

    void webPageLoaded(bool ok);
    void webPageReady();
    void webPageVideoError(QString context, QString error);
    void webPageAudioError(QString context, QString error);
    void webPageGeneralError(QString context, QString error);

private:
    void connectSignals();
    void refreshWebpageSettings();

    void setLoading(const bool& loading);
    void showError(const QString& title, const QString& context, const QString& error);

    // UI items
    Ui::VideoRehabWidget*   ui;
    QWebEngineView*         m_webEngine;
    VideoRehabWebPage*      m_webPage;
    QMovie*         m_loadingIcon;
};

#endif // VIDEOREHABWIDGET_H
