#ifndef FILEUPLOADERDIALOG_H
#define FILEUPLOADERDIALOG_H

#include <QDialog>

namespace Ui {
class FileUploaderDialog;
}

class FileUploaderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileUploaderDialog(QWidget *parent = nullptr);
    ~FileUploaderDialog();

private:
    Ui::FileUploaderDialog *ui;
};

#endif // FILEUPLOADERDIALOG_H
