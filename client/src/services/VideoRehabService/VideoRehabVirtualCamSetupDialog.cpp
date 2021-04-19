#include "VideoRehabVirtualCamSetupDialog.h"
#include "ui_VideoRehabVirtualCamSetupDialog.h"

VideoRehabVirtualCamSetupDialog::VideoRehabVirtualCamSetupDialog(QString current_src, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VideoRehabVirtualCamSetupDialog)
{
    ui->setupUi(this);
    setCurrentSource(current_src);

    connect(ui->txtVCamSrc, &QLineEdit::textChanged, this, &VideoRehabVirtualCamSetupDialog::validateValues);
    connect(ui->txtPassword, &QLineEdit::textChanged, this, &VideoRehabVirtualCamSetupDialog::validateValues);
    connect(ui->txtURL, &QLineEdit::textChanged, this, &VideoRehabVirtualCamSetupDialog::validateValues);
    connect(ui->txtUsername, &QLineEdit::textChanged, this, &VideoRehabVirtualCamSetupDialog::validateValues);
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

void VideoRehabVirtualCamSetupDialog::setCursorPosition(int pos)
{
    ui->txtVCamSrc->setCursorPosition(pos);
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

void VideoRehabVirtualCamSetupDialog::validateValues()
{
    bool valid = false;
    if (!ui->chkManual->isChecked()){
        valid = !ui->txtVCamSrc->text().isEmpty();
    }else{
        valid = !ui->txtURL->text().isEmpty() && !ui->txtPassword->text().isEmpty() && !ui->txtUsername->text().isEmpty();
        if (valid)
            buildCamSrc();
    }

    ui->btnOK->setEnabled(valid);
}

void VideoRehabVirtualCamSetupDialog::buildCamSrc()
{
    if (ui->cmbSource->currentIndex()==0){ // Vivotek camera
        QString src = "rtsp://" + ui->txtUsername->text() + ":" + ui->txtPassword->text() + "@" + ui->txtURL->text() + ":554/live.sdp";
        ui->txtVCamSrc->setText(src);
    }
}

