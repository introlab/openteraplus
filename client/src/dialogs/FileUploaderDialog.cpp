#include "FileUploaderDialog.h"
#include "ui_FileUploaderDialog.h"

#include <QDebug>

FileUploaderDialog::FileUploaderDialog(const QString &file_pattern, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileUploaderDialog),
    m_file_pattern(file_pattern)
{
    ui->setupUi(this);

    // Set default path to documents
    QStringList documents_path = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);

    if (!documents_path.isEmpty())
        m_current_base_path = documents_path.first();

    setResult(QDialog::Accepted); // Accepted by default - so it could be rejected properly if no initial file selection

    if (ui->tableFiles->rowCount()==0)
        on_btnAddFile_clicked();
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

    QStringList files_to_upload = dlg.getOpenFileNames(this, tr("Choisir le(s) fichier(s) à envoyer"), m_current_base_path, m_file_pattern);

    for (const QString &file:std::as_const(files_to_upload)){
        QFileInfo file_info(file);
        QString filename = file_info.fileName();
        QString default_label = filename.left(filename.length() - filename.split(".").last().length() - 1);
        if (file == files_to_upload.first()){
            // Update current path
            m_current_base_path = file_info.path();
        }
        addFileToTable(file, default_label);
    }

    ui->tableFiles->resizeColumnsToContents();

    if (ui->tableFiles->rowCount() == 0){
        // No file added, list still empty, close!
        qDebug() << "No file - closing!";
        reject();
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
    QTableWidgetItem* item = new QTableWidgetItem(QIcon("://icons/data.png"), file_path);
    ui->tableFiles->setItem(current_row, 0, item);

    // Create label for it
    QLineEdit* item_label = new QLineEdit(label);
    ui->tableFiles->setCellWidget(current_row, 1, item_label);

    ui->btnUpload->setEnabled(ui->tableFiles->rowCount()>0);
}

void FileUploaderDialog::showEvent(QShowEvent *event)
{
     QDialog::showEvent( event );

}

void FileUploaderDialog::on_tableFiles_itemSelectionChanged()
{
    ui->btnRemove->setEnabled(!ui->tableFiles->selectedItems().isEmpty());
}


void FileUploaderDialog::on_btnRemove_clicked()
{
    for(int i=0; i<ui->tableFiles->selectedItems().count(); i++){
        int row = ui->tableFiles->selectedItems().at(i)->row();
        ui->tableFiles->removeRow(row);
    }

    ui->btnUpload->setEnabled(ui->tableFiles->rowCount()>0);
}

