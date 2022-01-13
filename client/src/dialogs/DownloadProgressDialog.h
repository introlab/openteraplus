#ifndef DOWNLOADPROGRESSWIDGET_H
#define DOWNLOADPROGRESSWIDGET_H

#include <QDialog>

#include <QTableWidgetItem>
#include "data/DownloadingFile.h"

namespace Ui {
class DownloadProgressDialog;
}

class DownloadProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DownloadProgressDialog(QWidget *parent = nullptr);
    ~DownloadProgressDialog();

    void updateDownloadingFile(DownloadingFile* file);
    bool downloadFileCompleted(DownloadingFile* file);

private:
    Ui::DownloadProgressDialog *ui;

    QMap<DownloadingFile*, QTableWidgetItem*> m_files;
};

#endif // DOWNLOADPROGRESSWIDGET_H
