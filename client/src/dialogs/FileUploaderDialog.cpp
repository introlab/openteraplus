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

void FileUploaderDialog::on_btnBrowse_clicked()
{
    QFileDialog dlg;
    QStringList movie_paths = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation);
    QString base_path;

    if (!movie_paths.isEmpty())
        base_path = movie_paths.first();

    QString file_to_upload = dlg.getOpenFileName(this, tr("Choisir le fichier à envoyer"), base_path, tr("Vidéos (*.mp4 *.webm)"));

    ui->txtFilePath->setText(file_to_upload);

}


void FileUploaderDialog::on_btnUpload_clicked()
{
    accept();
}


void FileUploaderDialog::on_btnCancel_clicked()
{
    reject();
}

void FileUploaderDialog::updateUploadButtonState()
{
    bool file_exists = false;

    if (!ui->txtFilePath->text().isEmpty()){
        file_exists = QFile::exists(ui->txtFilePath->text());
    }

    ui->btnUpload->setEnabled(!ui->txtFilePath->text().isEmpty() && file_exists &&
                              !ui->txtFileLabel->text().isEmpty());
}


void FileUploaderDialog::on_txtFileLabel_textChanged(const QString &arg1)
{
    updateUploadButtonState();
}


void FileUploaderDialog::on_txtFilePath_textChanged(const QString &arg1)
{
    updateUploadButtonState();
}

