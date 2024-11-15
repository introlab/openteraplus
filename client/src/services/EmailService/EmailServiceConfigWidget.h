#ifndef EMAILSERVICECONFIGWIDGET_H
#define EMAILSERVICECONFIGWIDGET_H

#include <QWidget>
#include <QMenu>
#include "managers/ComManager.h"
#include "services/EmailService/EmailComManager.h"

namespace Ui {
class EmailServiceConfigWidget;
}

class EmailServiceConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EmailServiceConfigWidget(ComManager* comManager, QWidget *parent = nullptr);
    ~EmailServiceConfigWidget();

private slots:
    void insertCurrentVariable();
    void processTemplateReply(const QJsonObject email_template);

    void on_txtTemplate_textChanged();

    void on_btnSave_clicked();

    void on_btnUndo_clicked();

private:
    Ui::EmailServiceConfigWidget    *ui;
    EmailComManager*                m_emailComManager;

    ComManager*     m_comManager;
    QMenu*          m_variablesMenu = nullptr;

    void connectSignals();

    void initVariablesMenu();
    void initTemplates();

    void queryTemplate(const QString& key);
};

#endif // EMAILSERVICECONFIGWIDGET_H
