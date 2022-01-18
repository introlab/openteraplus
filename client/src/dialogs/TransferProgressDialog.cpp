#include "TransferProgressDialog.h"
#include "ui_TransferProgressDialog.h"

#include <QProgressBar>

TransferProgressDialog::TransferProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransferProgressDialog)
{
    ui->setupUi(this);
    //setWindowFlags(Qt::Popup);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(true);

}

TransferProgressDialog::~TransferProgressDialog()
{
    delete ui;
}

void TransferProgressDialog::updateTransferringFile(TransferringFile *file)
{
    QTableWidgetItem* item;
    QProgressBar* progress;
    if (!m_files.contains(file)){
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
    }else{
        item = m_files[file];
        progress = dynamic_cast<QProgressBar*>(ui->tableTransfers->cellWidget(item->row(), 0));
    }

    // Update values
    if (progress){
        progress->setMaximum(static_cast<int>(file->totalBytes()));
        progress->setValue(static_cast<int>(file->currentBytes()));
    }

}

bool TransferProgressDialog::transferFileCompleted(TransferringFile *file)
{
    updateTransferringFile(file);

    // Remove file from list
    ui->tableTransfers->removeRow(m_files.value(file)->row());
    m_files.remove(file);

    return m_files.isEmpty(); // No more files to display?
}

void TransferProgressDialog::reject()
{
    if (!m_files.isEmpty()){
        on_btnCancel_clicked();
        return;
    }
    QDialog::reject();
}

void TransferProgressDialog::on_btnCancel_clicked()
{
    GlobalMessageBox msg;
    GlobalMessageBox::StandardButton conf = msg.showYesNo(tr("Annuler les transferts"), tr("Les transferts en cours seront annulés. Êtes-vous sûrs de vouloir poursuivre?"));

    if (conf == GlobalMessageBox::Yes){
        for(QTableWidgetItem* item:qAsConst(m_files)){
            m_files.key(item)->abortTransfer();
        }
    }
    ui->tableTransfers->clearContents();
    m_files.clear();

    reject();
}

