#include "TransferProgressDialog.h"
#include "ui_TransferProgressDialog.h"

#include <QProgressBar>

TransferProgressDialog::TransferProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransferProgressDialog)
{
    ui->setupUi(this);

    m_aborting = false;
    m_totalFiles = 0;

    //setWindowFlags(Qt::Popup);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(true);

    // Init UI
    ui->tabTransfers->setCurrentIndex(0);
    for (int i=1; i<ui->tabTransfers->count(); i++){
        ui->tabTransfers->setTabVisible(i, false);
    }

}

TransferProgressDialog::~TransferProgressDialog()
{
    delete ui;
}

void TransferProgressDialog::updateTransferringFile(TransferringFile *file)
{
    if (m_aborting)
        return;

    if (file->getStatus() == TransferringFile::ABORTED || file->getStatus() == TransferringFile::ERROR)
        return;

    if (file->getStatus() == TransferringFile::WAITING){
        m_waitingFiles.append(file);
        m_totalFiles++;
        updateTotalProgress();
        return;
    }

    QTableWidgetItem* item;
    QProgressBar* progress;

    if (m_waitingFiles.contains(file)){
        m_waitingFiles.removeOne(file);
        m_totalFiles--;
    }

    if (!m_files.contains(file)){
        // qDebug() << file->getFileName() << " -- Added";
        // Must create a new file in the list
        int current_row = ui->tableTransfers->rowCount();
        ui->tableTransfers->setRowCount(ui->tableTransfers->rowCount()+1);

        item = new QTableWidgetItem(file->getFileName());
        ui->tableTransfers->setItem(current_row,1,item);
        m_files[file] = item;

        progress = new QProgressBar();
        progress->setMinimum(0);
        progress->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->tableTransfers->setCellWidget(current_row, 0, progress);

        ui->tabTransfers->setTabText(0, tr("En cours") + " (" + QString::number(ui->tableTransfers->rowCount()) + ")");
        m_totalFiles++;
    }else{
        item = m_files[file];
        progress = dynamic_cast<QProgressBar*>(ui->tableTransfers->cellWidget(item->row(), 0));
    }

    // Update values
    if (progress){
        if (file->totalBytes()>std::numeric_limits<int>::max()){
            progress->setMaximum(static_cast<int>(file->totalBytes()/1024));
            progress->setValue(static_cast<int>(file->currentBytes()/1024));
            qDebug() << progress->maximum() << progress->value();
        }else{
            progress->setMaximum(static_cast<int>(file->totalBytes()));
            progress->setValue(static_cast<int>(file->currentBytes()));
        }
    }

    if (file->getStatus() == TransferringFile::INPROGRESS){
        // Make sure file is at the top of the list
        int max_simultaneous = 1;
        if (dynamic_cast<DownloadingFile*>(file)){
            max_simultaneous = MAX_SIMULTANEOUS_DOWNLOADS;
        }
        if (dynamic_cast<UploadingFile*>(file)){
            max_simultaneous = MAX_SIMULTANEOUS_UPLOADS;
        }
        if (item->row() >= max_simultaneous){
            int row = item->row();
            item = ui->tableTransfers->takeItem(row, 1);
            progress = new QProgressBar();
            progress->setMinimum(0);
            progress->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            ui->tableTransfers->removeRow(row); // Will delete "old" progress bar at the same time
            ui->tableTransfers->insertRow(0);
            ui->tableTransfers->setItem(0, 1, item);
            ui->tableTransfers->setCellWidget(0, 0, progress);
        }
    }

    updateTotalProgress();

}

bool TransferProgressDialog::transferFileCompleted(TransferringFile *file)
{
    if (m_aborting)
        return false;

    updateTransferringFile(file);

    // Remove file from list
    ui->tableTransfers->removeRow(m_files.value(file)->row());
    m_files.remove(file);

    addCompleted(file->getFileName());
    updateCancelButtonText();

    updateTotalProgress();

    return m_files.isEmpty() && ui->tableErrors->rowCount()==0 && m_waitingFiles.isEmpty(); // No more files to display?
}

bool TransferProgressDialog::transferFileAborted(TransferringFile *file)
{
    if (m_aborting)
        return false;

    // Remove file from list
    if (m_files.contains(file)){
        ui->tableTransfers->removeRow(m_files.value(file)->row());
        m_files.remove(file);
        addError(file->getFileName(), file->getLastError());
        updateCancelButtonText();
        updateTotalProgress();
    }

    return m_files.isEmpty() && ui->tableErrors->rowCount()==0; // No more files to display?
}

bool TransferProgressDialog::hasErrors()
{
    return ui->tableErrors->rowCount()>0;
}

bool TransferProgressDialog::hasAborted()
{
    return m_aborting;
}

void TransferProgressDialog::reject()
{
    if (!m_files.isEmpty() && !m_aborting){
        on_btnCancel_clicked();
        return;
    }
    QDialog::reject();
}

void TransferProgressDialog::on_btnCancel_clicked()
{
    if (!m_files.isEmpty() || !m_waitingFiles.isEmpty()){
        GlobalMessageBox msg;
        GlobalMessageBox::StandardButton conf = msg.showYesNo(tr("Annuler les transferts"), tr("Les transferts en cours seront annulés. Êtes-vous sûrs de vouloir poursuivre?"));

        if (conf == GlobalMessageBox::Yes){
            // Set aborting flag to prevent further updates
            m_aborting = true;

            /*for(QTableWidgetItem* item:qAsConst(m_files)){
                m_files.key(item)->abortTransfer();
            }
            for(TransferringFile* file:qAsConst(m_waitingFiles)){
                file->abortTransfer();
            }*/
            emit transferAbortRequested();
        }

        reject();
    }else{
        accept();
    }
}

void TransferProgressDialog::addCompleted(const QString &filename)
{
    QListWidgetItem* item = new QListWidgetItem(QIcon("://icons/ok2.png"), filename);
    ui->lstCompleted->addItem(item);

    ui->tabTransfers->setTabText(1, tr("Complétés") + " (" + QString::number(ui->lstCompleted->count()) + ")");
    ui->tabTransfers->setTabVisible(1, true);

    ui->tabTransfers->setTabText(0, tr("En cours") + " (" + QString::number(ui->tableTransfers->rowCount()) + ")");
}

void TransferProgressDialog::addError(const QString &filename, const QString &error)
{
    QTableWidgetItem* file_item = new QTableWidgetItem(QIcon("://icons/error2.png"), filename);
    ui->tableErrors->setRowCount(ui->tableErrors->rowCount()+1);
    int current_row = ui->tableErrors->rowCount()-1;
    ui->tableErrors->setItem(current_row, 0, file_item);

    QTableWidgetItem* error_item = new QTableWidgetItem(error);
    ui->tableErrors->setItem(current_row, 1, error_item);

    ui->tabTransfers->setTabText(2, tr("Erreurs") + " (" + QString::number(ui->tableErrors->rowCount()) + ")");
    ui->tabTransfers->setTabVisible(2, true);

    ui->tableErrors->resizeColumnsToContents();

    ui->tabTransfers->setTabText(0, tr("En cours") + " (" + QString::number(ui->tableTransfers->rowCount()) + ")");

    ui->tabTransfers->setCurrentWidget(ui->tabErrors); // Select current errors tabs when errors
}

void TransferProgressDialog::updateCancelButtonText()
{
    if (m_files.isEmpty() && m_waitingFiles.isEmpty()){
        ui->btnCancel->setText(tr("Fermer"));
    }else{
        ui->btnCancel->setText(tr("Annuler"));
    }
}

void TransferProgressDialog::updateTotalProgress()
{
    ui->progTotalTransfer->setMaximum(m_totalFiles);
    ui->progTotalTransfer->setValue(ui->lstCompleted->count() + ui->tableErrors->rowCount());
}


