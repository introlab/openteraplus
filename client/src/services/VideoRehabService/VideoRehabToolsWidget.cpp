#include "VideoRehabToolsWidget.h"
#include "ui_VideoRehabToolsWidget.h"

VideoRehabToolsWidget::VideoRehabToolsWidget(ComManager* comMan, BaseServiceWidget* baseWidget, QWidget *parent) :
    BaseServiceToolsWidget(comMan, baseWidget, parent),
    ui(new Ui::VideoRehabToolsWidget)
{
    ui->setupUi(this);
}

VideoRehabToolsWidget::~VideoRehabToolsWidget()
{
    delete ui;
}

void VideoRehabToolsWidget::on_btnReconnect_clicked()
{
    dynamic_cast<VideoRehabWidget*>(m_baseWidget)->reload();

}
