#ifndef SESSIONLOBBYDIALOG_H
#define SESSIONLOBBYDIALOG_H

#include <QDialog>
#include "ComManager.h"

#include "services/VideoRehabService/VideoRehabSetupWidget.h"

namespace Ui {
class SessionLobbyDialog;
}

class SessionLobbyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SessionLobbyDialog(ComManager* comManager, TeraData &session_type, QWidget *parent = nullptr);
    ~SessionLobbyDialog();

private slots:
    void on_btnCancel_clicked();

    void on_btnStartSession_clicked();

private:
    Ui::SessionLobbyDialog *ui;
    ComManager*     m_comManager;
    TeraData        m_sessionType;

    void configureWidget();
    void setSetupWidget(QWidget *wdg);

    TeraSessionCategory::SessionTypeCategories getSessionTypeCategory();

};

#endif // SESSIONLOBBYDIALOG_H
