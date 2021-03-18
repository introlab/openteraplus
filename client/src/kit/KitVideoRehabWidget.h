#ifndef KITVIDEOREHABWIDGET_H
#define KITVIDEOREHABWIDGET_H

#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QHBoxLayout>
#include <QMovie>
#include <QWebEngineSettings>

#include "services/VideoRehabService/VideoRehabWebPage.h"

#include "KitConfigManager.h"

#include "Utils.h"

#include "VirtualCameraThread.h"

#include "Logger.h"


namespace Ui {
class KitVideoRehabWidget;
}

class KitVideoRehabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KitVideoRehabWidget(KitConfigManager* kitConfig, QWidget *parent = nullptr);
    ~KitVideoRehabWidget();

    void initUi();

    void reload();

private slots:
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

    void loadConfiguredUrl();

    void setLoading(const bool& loading);
    void showError(const QString& title, const QString& context, const QString& error);

    void startVirtualCamera(const QString& src);
    void stopVirtualCamera();

    // UI items
    Ui::KitVideoRehabWidget*   ui;
    QWebEngineView*         m_webEngine;
    VideoRehabWebPage*      m_webPage;
    QMovie*                 m_loadingIcon;
    VirtualCameraThread*    m_virtualCamThread;
    KitConfigManager*       m_kitConfig;
};

#endif // KITVIDEOREHABWIDGET_H
