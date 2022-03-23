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
    explicit FileUploaderDialog(const QString &file_pattern, QWidget *parent = nullptr);
    ~FileUploaderDialog();

    QStringList getFiles();
    QStringList getLabels();

private slots:
    void on_btnAddFile_clicked();
    void on_btnUpload_clicked();
    void on_btnCancel_clicked();

    void on_tableFiles_itemSelectionChanged();

    void on_btnRemove_clicked();

private:
    Ui::FileUploaderDialog *ui;
    QString m_current_base_path;
    QString m_file_pattern;

    void addFileToTable(const QString& file_path, const QString &label);

protected:
    void showEvent(QShowEvent * event) override;
};

#endif // FILEUPLOADERDIALOG_H
