#ifndef KITINSESSIONDIALOG_H
#define KITINSESSIONDIALOG_H

#include <QDialog>

#include "GlobalMessageBox.h"

#include "KitConfigManager.h"
#include "KitVideoRehabWidget.h"

#include "managers/ParticipantComManager.h"

namespace Ui {
class KitInSessionDialog;
}

class KitInSessionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KitInSessionDialog(KitConfigManager *kitConfig, ParticipantComManager* partCom, QWidget *parent = nullptr);
    ~KitInSessionDialog();

private slots:
    void on_btnLogout_clicked();

private:
    void initUi();
    void setMainWidget(QWidget* wdg);


    Ui::KitInSessionDialog *ui;

    KitConfigManager*       m_kitConfig;
    KitVideoRehabWidget*    m_serviceWidget;

    ParticipantComManager*  m_partComManager;
};

#endif // KITINSESSIONDIALOG_H
