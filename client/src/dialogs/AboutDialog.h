#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QWebEngineView>
#include <QSizePolicy>

#include "AboutDialogPage.h"
#include "managers/ComManager.h"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(ComManager* comManager, QWidget *parent = nullptr);
    ~AboutDialog();

private slots:
    void on_btnOk_clicked();
    void on_lblAbout_clicked();
    void onPageLoaded();

    void processServerSettings(QVariantHash settings);

private:
    void connectSignals();

    Ui::AboutDialog *ui;

    QWebEngineView*         m_webEngine;
    AboutDialogPage*        m_webPage;

    ComManager*             m_comManager;
};

#endif // ABOUTDIALOG_H
