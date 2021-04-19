#include "VideoRehabPTZDialog.h"
#include "ui_VideoRehabPTZDialog.h"

VideoRehabPTZDialog::VideoRehabPTZDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VideoRehabPTZDialog)
{
    ui->setupUi(this);

    connect(ui->cmbSource, &QComboBox::currentTextChanged, this, &VideoRehabPTZDialog::validateValues);
    connect(ui->txtPassword, &QLineEdit::textChanged, this, &VideoRehabPTZDialog::validateValues);
    connect(ui->txtURL, &QLineEdit::textChanged, this, &VideoRehabPTZDialog::validateValues);
    connect(ui->txtUsername, &QLineEdit::textChanged, this, &VideoRehabPTZDialog::validateValues);
    connect(ui->spinPort, &QSpinBox::textChanged, this, &VideoRehabPTZDialog::validateValues);
}

VideoRehabPTZDialog::~VideoRehabPTZDialog()
{
    delete ui;
}

void VideoRehabPTZDialog::setCurrentValues(int current_src_index, QString url, int port, QString username, QString password)
{
    ui->cmbSource->setCurrentIndex(current_src_index);
    ui->txtURL->setText(url);
    ui->spinPort->setValue(port);
    ui->txtUsername->setText(username);
    ui->txtPassword->setText(password);

}

int VideoRehabPTZDialog::getCurrentSrcIndex()
{
    return ui->cmbSource->currentIndex();
}

QString VideoRehabPTZDialog::getCurrentUrl()
{
    return ui->txtURL->text();
}

int VideoRehabPTZDialog::getCurrentPort()
{
    return ui->spinPort->value();
}

QString VideoRehabPTZDialog::getCurrentUsername()
{
    return ui->txtUsername->text();
}

QString VideoRehabPTZDialog::getCurrentPassword()
{
    return ui->txtPassword->text();
}

void VideoRehabPTZDialog::validateValues()
{
    bool valid = false;
    valid = !ui->txtURL->text().isEmpty() && !ui->txtPassword->text().isEmpty() && !ui->txtUsername->text().isEmpty()
            && ui->cmbSource->currentIndex() >= 0 && ui->spinPort->value()>0;

    ui->btnOK->setEnabled(valid);
}

void VideoRehabPTZDialog::on_btnOK_clicked()
{
    accept();
}

void VideoRehabPTZDialog::on_btnCancel_clicked()
{
    reject();
}
