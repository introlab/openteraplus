#include "DanceConfigWidget.h"
#include "ui_DanceConfigWidget.h"

DanceConfigWidget::DanceConfigWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DanceConfigWidget)
{
    ui->setupUi(this);
}

DanceConfigWidget::~DanceConfigWidget()
{
    delete ui;
}

void DanceConfigWidget::on_btnUpload_clicked()
{

}

