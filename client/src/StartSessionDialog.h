#ifndef STARTSESSIONDIALOG_H
#define STARTSESSIONDIALOG_H

#include <QDialog>
#include <QMovie>
#include "ComManager.h"

namespace Ui {
class StartSessionDialog;
}

class StartSessionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StartSessionDialog(QString title, ComManager* comManager, QWidget *parent = nullptr);
    ~StartSessionDialog();

private:
    Ui::StartSessionDialog *ui;

    ComManager* m_comManager;

    // UI items
    QMovie*         m_loadingIcon;

private slots:
    void closeRequest();
    void on_btnCancel_clicked();
};

#endif // STARTSESSIONDIALOG_H
