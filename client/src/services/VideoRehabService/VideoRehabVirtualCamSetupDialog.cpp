#include "VideoRehabVirtualCamSetupDialog.h"
#include "ui_VideoRehabVirtualCamSetupDialog.h"

VideoRehabVirtualCamSetupDialog::VideoRehabVirtualCamSetupDialog(QString current_src, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VideoRehabVirtualCamSetupDialog)
{
    ui->setupUi(this);
    setCurrentSource(current_src);
}

VideoRehabVirtualCamSetupDialog::~VideoRehabVirtualCamSetupDialog()
{
    delete ui;
}

QString VideoRehabVirtualCamSetupDialog::getCurrentSource()
{
    return ui->txtVCamSrc->text();
}

void VideoRehabVirtualCamSetupDialog::setCurrentSource(QString &src)
{
    ui->txtVCamSrc->setText(src);
}

void VideoRehabVirtualCamSetupDialog::on_btnOK_clicked()
{
    accept();
}

void VideoRehabVirtualCamSetupDialog::on_btnCancel_clicked()
{
    reject();
}

void VideoRehabVirtualCamSetupDialog::on_chkManual_stateChanged(int arg1)
{
    Q_UNUSED(arg1)

    if (!ui->chkManual->isChecked()){
        ui->wdgSetup->setCurrentWidget(ui->pageManualSetup);
    }else{
        ui->wdgSetup->setCurrentWidget(ui->pageAutoSetup);
    }
}

void VideoRehabVirtualCamSetupDialog::on_txtVCamSrc_textChanged(const QString &arg1)
{

}

void VideoRehabVirtualCamSetupDialog::on_btnTestParams_clicked()
{

}
