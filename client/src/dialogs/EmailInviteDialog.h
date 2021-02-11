#ifndef EMAILINVITEDIALOG_H
#define EMAILINVITEDIALOG_H

#include <QDialog>
#include <QClipboard>
#include <QThread>
#include <QDesktopServices>
#include "GlobalMessageBox.h"

#include "ComManager.h"

namespace Ui {
class EmailInviteDialog;
}

class EmailInviteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EmailInviteDialog(ComManager* comMan, TeraData* participant, QWidget *parent = nullptr);
    ~EmailInviteDialog();

    void setFieldValues(const QHash<QString, QString> &fields);

private slots:
    void on_btnOk_clicked();

    void on_btnCopy_clicked();

    void on_btnSendLocalEmail_clicked();

private:
    Ui::EmailInviteDialog *ui;

    ComManager*     m_comManager;
    TeraData*       m_participant;
    QHash<QString, QString> m_fields;
};

#endif // EMAILINVITEDIALOG_H
