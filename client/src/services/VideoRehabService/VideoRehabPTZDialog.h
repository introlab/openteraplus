#ifndef VIDEOREHABPTZDIALOG_H
#define VIDEOREHABPTZDIALOG_H

#include <QDialog>

namespace Ui {
class VideoRehabPTZDialog;
}

class VideoRehabPTZDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VideoRehabPTZDialog(QWidget *parent = nullptr);
    ~VideoRehabPTZDialog();

    void setCurrentValues(int current_src_index, QString url, int port, QString username, QString password);
    int getCurrentSrcIndex();
    QString getCurrentUrl();
    int getCurrentPort();
    QString getCurrentUsername();
    QString getCurrentPassword();

private slots:
    void validateValues();

    void on_btnOK_clicked();
    void on_btnCancel_clicked();

private:
    Ui::VideoRehabPTZDialog *ui;
};

#endif // VIDEOREHABPTZDIALOG_H
