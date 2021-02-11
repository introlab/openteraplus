#ifndef VIDEOREHABSETUPWIDGET_H
#define VIDEOREHABSETUPWIDGET_H

#include <QWidget>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QHBoxLayout>
#include <QComboBox>

#include <QAudioDeviceInfo>
#include <QCameraInfo>

#include "services/BaseServiceSetupWidget.h"

#include "VideoRehabWebPage.h"
#include "VideoRehabVirtualCamSetupDialog.h"
#include "VideoRehabPTZDialog.h"
#include "ComManager.h"
#include "Utils.h"

#include "VirtualCameraThread.h"


namespace Ui {
class VideoRehabSetupWidget;
}

class VideoRehabSetupWidget : public BaseServiceSetupWidget
{
    Q_OBJECT

public:
    explicit VideoRehabSetupWidget(ComManager* comManager, QWidget *parent = nullptr);
    ~VideoRehabSetupWidget();

    QJsonDocument getSetupConfig();

private:
    Ui::VideoRehabSetupWidget *ui;

    QWebEngineView*         m_webEngine;
    VideoRehabWebPage*      m_webPage;

    VirtualCameraThread*    m_virtualCamThread;

    int                     m_id_service_config;

    bool                    m_valueJustChanged;

    void initUI();
    void connectSignals();

    /*void selectVideoSrcByName(const QString& name);
    void selectAudioSrcByName(const QString& name);*/

    void setLoading(const bool& loading);
    void showError(const QString& title, const QString& context, const QString& error, bool hide_retry = false);

    void startVirtualCamera(const QString& src);
    void stopVirtualCamera();

    void showPTZDialog();


private slots:
    void webPageLoaded(bool ok);
    void webPageReady();
    void webPageCameraChanged();
    void webPageVideoError(QString context, QString error);
    void webPageAudioError(QString context, QString error);
    void webPageGeneralError(QString context, QString error);

    void processServiceConfigsReply(QList<TeraData> configs, QUrlQuery query);
    void processFormsReply(QString form_type, QString data);

    void refreshWebpageSettings();
    void on_btnRefresh_clicked();
    void on_btnAdvancedConfig_clicked();
    void on_btnSaveConfig_clicked();

    void setupFormDirtyChanged(bool dirty);
    void setupFormValueChanged(QWidget* wdg, QVariant value);

    void virtualCameraDisconnected();
};

#endif // VIDEOREHABSETUPWIDGET_H
