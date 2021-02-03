#ifndef VIDEOREHABVIRTUALCAMSETUPDIALOG_H
#define VIDEOREHABVIRTUALCAMSETUPDIALOG_H

#include <QDialog>
#include "VirtualCamera.h"

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

private slots:
    void on_btnOK_clicked();

    void on_btnCancel_clicked();

    void on_chkManual_stateChanged(int arg1);

    void on_txtVCamSrc_textChanged(const QString &arg1);

    void on_btnTestParams_clicked();

private:
    Ui::VideoRehabVirtualCamSetupDialog *ui;

    VirtualCamera* m_virtualCam;
};

#endif // VIDEOREHABVIRTUALCAMSETUPDIALOG_H
