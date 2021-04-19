#ifndef VIDEOREHABVIRTUALCAMSETUPDIALOG_H
#define VIDEOREHABVIRTUALCAMSETUPDIALOG_H

#include <QDialog>

namespace Ui {
class VideoRehabVirtualCamSetupDialog;
}

class VideoRehabVirtualCamSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VideoRehabVirtualCamSetupDialog(QString current_src, QWidget *parent = nullptr);
    ~VideoRehabVirtualCamSetupDialog();

    QString getCurrentSource();
    void setCurrentSource(QString& src);
    void setCursorPosition(int pos);

private slots:
    void on_btnOK_clicked();

    void on_btnCancel_clicked();

    void on_chkManual_stateChanged(int arg1);

    void validateValues();

private:
    Ui::VideoRehabVirtualCamSetupDialog *ui;

    void buildCamSrc();
};

#endif // VIDEOREHABVIRTUALCAMSETUPDIALOG_H
