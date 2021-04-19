#ifndef STARTSESSIONDIALOG_H
#define STARTSESSIONDIALOG_H

#include <QDialog>
#include <QMovie>
#include "managers/ComManager.h"

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
    QTimer      m_timer;
    int         m_timeout = 10; // Timeout in seconds
    int         m_count = 0; // Current count value

    // UI items
    QMovie*         m_loadingIcon;

private slots:
    void closeRequest();
    void on_btnCancel_clicked();
    void timerTimeOut();

signals:
    void timeout();
};

#endif // STARTSESSIONDIALOG_H
