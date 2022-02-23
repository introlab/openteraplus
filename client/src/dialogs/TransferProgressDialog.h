#ifndef DOWNLOADPROGRESSWIDGET_H
#define DOWNLOADPROGRESSWIDGET_H

#include <QDialog>
#include <QTableWidgetItem>

#include "GlobalMessageBox.h"
#include "data/TransferringFile.h"
#include "data/DownloadingFile.h"
#include "data/UploadingFile.h"

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
    bool transferFileAborted(TransferringFile* file);

    bool hasErrors();
    bool hasAborted();

public slots:
    void reject() override;

private slots:
    void on_btnCancel_clicked();

signals:
    void transferAbortRequested();


private:
    Ui::TransferProgressDialog *ui;

    QMap<TransferringFile*, QTableWidgetItem*> m_files;
    QList<TransferringFile*> m_waitingFiles;

    bool m_aborting;
    int m_totalFiles;

    void addCompleted(const QString& filename);
    void addError(const QString& filename, const QString &error);

    void updateCancelButtonText();
    void updateTotalProgress();
};

#endif // DOWNLOADPROGRESSWIDGET_H
