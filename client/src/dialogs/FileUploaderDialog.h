#ifndef FILEUPLOADERDIALOG_H
#define FILEUPLOADERDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QStandardPaths>

namespace Ui {
class FileUploaderDialog;
}

class FileUploaderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileUploaderDialog(QWidget *parent = nullptr);
    ~FileUploaderDialog();

private slots:
    void on_btnBrowse_clicked();

    void on_btnUpload_clicked();

    void on_btnCancel_clicked();

    void on_txtFileLabel_textChanged(const QString &arg1);

    void on_txtFilePath_textChanged(const QString &arg1);

private:
    Ui::FileUploaderDialog *ui;

    void updateUploadButtonState();
};

#endif // FILEUPLOADERDIALOG_H
