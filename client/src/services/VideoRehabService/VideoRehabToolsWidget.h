#ifndef VIDEOREHABTOOLSWIDGET_H
#define VIDEOREHABTOOLSWIDGET_H

#include <QWidget>

#include "VideoRehabWidget.h"
#include "services/BaseServiceToolsWidget.h"

namespace Ui {
class VideoRehabToolsWidget;
}

class VideoRehabToolsWidget : public BaseServiceToolsWidget
{
    Q_OBJECT

public:
    explicit VideoRehabToolsWidget(ComManager* comMan, BaseServiceWidget *baseWidget, QWidget *parent = nullptr);
    ~VideoRehabToolsWidget();

    bool sessionCanBeEnded(const bool &displayConfirmation = true) override;

public slots:
    void setReadyState(bool ready_state) override;

private slots:
    void on_btnReconnect_clicked();

    void on_btnRecord_clicked();

    void on_btnPause_clicked();

private:
    void setupTools();

    Ui::VideoRehabToolsWidget *ui;

    bool m_isRecording;
    bool m_recordWarningShown;


};

#endif // VIDEOREHABTOOLSWIDGET_H
