#include "FileUploaderDialog.h"
#include "ui_FileUploaderDialog.h"

FileUploaderDialog::FileUploaderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileUploaderDialog)
{
    ui->setupUi(this);
}

FileUploaderDialog::~FileUploaderDialog()
{
    delete ui;
}
