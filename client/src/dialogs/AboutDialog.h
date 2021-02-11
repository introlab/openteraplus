#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QWebEngineView>
#include <QSizePolicy>

#include "AboutDialogPage.h"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QUrl server_url, QWidget *parent = nullptr);
    ~AboutDialog();

private slots:
    void on_btnOk_clicked();
    void on_lblAbout_clicked();

private:
    Ui::AboutDialog *ui;

    QWebEngineView*         m_webEngine;
    AboutDialogPage*        m_webPage;
};

#endif // ABOUTDIALOG_H
