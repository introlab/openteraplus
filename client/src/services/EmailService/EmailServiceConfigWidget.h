#ifndef EMAILSERVICECONFIGWIDGET_H
#define EMAILSERVICECONFIGWIDGET_H

#include <QWidget>
#include <QMenu>
#include "managers/ComManager.h"
#include "services/EmailService/EmailComManager.h"
#include "data/Message.h"

namespace Ui {
class EmailServiceConfigWidget;
}

class EmailServiceConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EmailServiceConfigWidget(ComManager* comManager, const int& id_site = -1, const int& id_project = -1, QWidget *parent = nullptr);
    ~EmailServiceConfigWidget();

    void setSiteId(const int& id_site);
    void setProjectId(const int& id_project);

private slots:
    void insertCurrentVariable();
    void processTemplateReply(const QJsonObject email_template);
    void handleNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code);
    void emailComPostOK(QString path, QString data);

    void nextMessageWasShown(Message current_message);

    void on_txtTemplate_textChanged();
    void on_btnSave_clicked();
    void on_btnUndo_clicked();
    void on_cmbLanguage_currentIndexChanged(int index);

private:
    Ui::EmailServiceConfigWidget    *ui;
    EmailComManager*                m_emailComManager;

    int m_idSite = -1;
    int m_idProject = -1;

    ComManager*     m_comManager;
    QMenu*          m_variablesMenu = nullptr;
    QJsonObject     m_currentTemplate;

    void connectSignals();

    void initVariablesMenu();
    void initTemplates();

    void queryTemplate(const QString& key);
};

#endif // EMAILSERVICECONFIGWIDGET_H
