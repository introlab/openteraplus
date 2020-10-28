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

private slots:
    void on_btnReconnect_clicked();

private:
    Ui::VideoRehabToolsWidget *ui;
};

#endif // VIDEOREHABTOOLSWIDGET_H
