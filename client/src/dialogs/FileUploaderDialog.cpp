#include "FileUploaderDialog.h"
#include "ui_FileUploaderDialog.h"

FileUploaderDialog::FileUploaderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileUploaderDialog)
{
    ui->setupUi(this);

    // Set default path to documents
    QStringList documents_path = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);

    if (!documents_path.isEmpty())
        current_base_path = documents_path.first();
}

FileUploaderDialog::~FileUploaderDialog()
{
    delete ui;
}

QStringList FileUploaderDialog::getFiles()
{
    QStringList files;
    for(int i=0; i<ui->tableFiles->rowCount(); i++){
        files.append(ui->tableFiles->item(i,0)->text());
    }
    return files;
}

QStringList FileUploaderDialog::getLabels()
{
    QStringList labels;
    for (int i=0; i<ui->tableFiles->rowCount(); i++){
        QLineEdit* editor = dynamic_cast<QLineEdit*>(ui->tableFiles->cellWidget(i,1));
        if (editor){
            labels.append(editor->text());
        }
    }
    return labels;
}

void FileUploaderDialog::on_btnAddFile_clicked()
{
    QFileDialog dlg;

    QStringList files_to_upload = dlg.getOpenFileNames(this, tr("Choisir le(s) fichier(s) à envoyer"), current_base_path, tr("Vidéos (*.mp4 *.webm)"));

    for (const QString &file:qAsConst(files_to_upload)){
        QFileInfo file_info(file);
        QString default_label = file_info.fileName().split(".").first();
        if (file == files_to_upload.first()){
            // Update current path
            current_base_path = file_info.path();
        }
        addFileToTable(file, default_label);
    }

}


void FileUploaderDialog::on_btnUpload_clicked()
{
    accept();
}


void FileUploaderDialog::on_btnCancel_clicked()
{
    reject();
}

void FileUploaderDialog::addFileToTable(const QString &file_path, const QString &label)
{
    // Check if file is already in the table - if so, skips!
    if (!ui->tableFiles->findItems(file_path, Qt::MatchExactly).isEmpty()){
        return;
    }

    // Add file
    ui->tableFiles->setRowCount(ui->tableFiles->rowCount()+1);
    int current_row = ui->tableFiles->rowCount()-1;
    QTableWidgetItem* item = new QTableWidgetItem(QIcon("://icons/data_video.png"), file_path);
    ui->tableFiles->setItem(current_row, 0, item);

    // Create label for it
    QLineEdit* item_label = new QLineEdit(label);
    ui->tableFiles->setCellWidget(current_row, 1, item_label);

    ui->btnUpload->setEnabled(ui->tableFiles->rowCount()>0);

}

void FileUploaderDialog::on_tableFiles_itemSelectionChanged()
{
    ui->btnRemove->setEnabled(!ui->tableFiles->selectedItems().isEmpty());
}


void FileUploaderDialog::on_btnRemove_clicked()
{
    for(const QTableWidgetItem* item:ui->tableFiles->selectedItems()){
        ui->tableFiles->removeRow(item->row());
    }

    ui->btnUpload->setEnabled(ui->tableFiles->rowCount()>0);
}

