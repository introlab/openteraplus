#ifndef EMAILINVITEDIALOG_H
#define EMAILINVITEDIALOG_H

#include <QDialog>
#include <QClipboard>
#include <QThread>
#include <QDesktopServices>

#include "managers/ComManager.h"
#include "services/EmailService/EmailComManager.h"

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

    void processTemplateReply(const QJsonObject email_template);
    void handleNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code);
    void emailSentSuccess();

    void on_cmbLanguage_currentIndexChanged(int index);

    void on_btnSend_clicked();

private:
    void connectSignals();
    void initLanguages();
    void queryTemplate();

    Ui::EmailInviteDialog *ui;

    ComManager*         m_comManager;
    EmailComManager*    m_emailComManager;
    TeraData*           m_participant;
};

#endif // EMAILINVITEDIALOG_H
