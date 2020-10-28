﻿#ifndef VIDEOREHABSETUPWIDGET_H
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
#include "ComManager.h"
#include "Utils.h"


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

    int                     m_id_service_config;

    void initUI();
    void connectSignals();

    /*void selectVideoSrcByName(const QString& name);
    void selectAudioSrcByName(const QString& name);*/

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
    void processFormsReply(QString form_type, QString data);

    void refreshWebpageSettings();
    void on_btnRefresh_clicked();
    void on_btnAdvancedConfig_clicked();
    void on_btnSaveConfig_clicked();

    void setupFormDirtyChanged(bool dirty);
};

#endif // VIDEOREHABSETUPWIDGET_H
