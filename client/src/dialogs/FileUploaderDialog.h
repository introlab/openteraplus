#ifndef FILEUPLOADERDIALOG_H
#define FILEUPLOADERDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QStandardPaths>
#include <QLineEdit>

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
    void on_btnAddFile_clicked();
    void on_btnUpload_clicked();
    void on_btnCancel_clicked();

    void on_tableFiles_itemSelectionChanged();

    void on_btnRemove_clicked();

private:
    Ui::FileUploaderDialog *ui;
    QString current_base_path;

    void addFileToTable(const QString& file_path, const QString &label);
};

#endif // FILEUPLOADERDIALOG_H
