#ifndef DOWNLOADPROGRESSWIDGET_H
#define DOWNLOADPROGRESSWIDGET_H

#include <QDialog>
#include <QTableWidgetItem>

#include "GlobalMessageBox.h"
#include "data/TransferringFile.h"
#include "data/DownloadingFile.h"

namespace Ui {
class TransferProgressDialog;
}

class TransferProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TransferProgressDialog(QWidget *parent = nullptr);
    ~TransferProgressDialog();

    void updateTransferringFile(TransferringFile* file);
    bool transferFileCompleted(TransferringFile* file);

public slots:
    void reject() override;

private slots:
    void on_btnCancel_clicked();


private:
    Ui::TransferProgressDialog *ui;

    QMap<TransferringFile*, QTableWidgetItem*> m_files;
};

#endif // DOWNLOADPROGRESSWIDGET_H