#include "EmailServiceConfigWidget.h"
#include "ui_EmailServiceConfigWidget.h"
#include "EmailServiceWebAPI.h"

#include <QStyledItemDelegate>

EmailServiceConfigWidget::EmailServiceConfigWidget(ComManager *comManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EmailServiceConfigWidget)
{
    m_comManager = comManager;
    m_emailComManager = new EmailComManager(comManager);

    ui->setupUi(this);
    ui->cmbTemplate->setItemDelegate(new QStyledItemDelegate(ui->cmbTemplate));
    ui->btnBold->hide();
    ui->btnItalic->hide();
    ui->btnColor->hide();
    ui->btnUnderline->hide();

    initVariablesMenu();
    initTemplates();
    connectSignals();

    // Only one template for now... query it!
    if (ui->cmbTemplate->count() > 0){
        queryTemplate(ui->cmbTemplate->currentData().toString());
    }


}

EmailServiceConfigWidget::~EmailServiceConfigWidget()
{
    delete ui;
    delete m_emailComManager;
}

void EmailServiceConfigWidget::insertCurrentVariable()
{
    QObject* sender = QObject::sender();
    if (!sender)
        return;

    QAction* action = dynamic_cast<QAction*>(sender);
    if (action){
        ui->txtTemplate->insertHtml(action->data().toString());
    }
}

void EmailServiceConfigWidget::processTemplateReply(const QJsonObject email_template)
{
    if (!email_template.empty()){
        QString template_html = email_template["email_template"].toString();
        ui->txtTemplate->setHtml(template_html);
        ui->txtTemplate->setProperty("original", ui->txtTemplate->toHtml());
        ui->frameEditor->show();
        ui->frameSave->show();
        ui->frameSave->setEnabled(false);
    }
}

void EmailServiceConfigWidget::connectSignals()
{
    connect(m_emailComManager, &EmailComManager::emailTemplateReceived, this, &EmailServiceConfigWidget::processTemplateReply);
}

void EmailServiceConfigWidget::initVariablesMenu()
{
    if (!m_variablesMenu){
        m_variablesMenu = new QMenu(this);

        QAction* new_action = new QAction(tr("Nom participant"));
        new_action->setData("$participant");
        connect(new_action, &QAction::triggered, this, &EmailServiceConfigWidget::insertCurrentVariable);
        m_variablesMenu->addAction(new_action);

        new_action = new QAction(tr("Nom utilisateur"));
        new_action->setData("$fullname");
        connect(new_action, &QAction::triggered, this, &EmailServiceConfigWidget::insertCurrentVariable);
        m_variablesMenu->addAction(new_action);

        new_action = new QAction(tr("Lien de connexion"));
        new_action->setData("$join_link");
        connect(new_action, &QAction::triggered, this, &EmailServiceConfigWidget::insertCurrentVariable);
        m_variablesMenu->addAction(new_action);

        ui->btnVariables->setMenu(m_variablesMenu);
    }
}

void EmailServiceConfigWidget::initTemplates()
{
    ui->cmbTemplate->clear();
    ui->cmbTemplate->addItem(tr("Courriel d'invitation"), "INVITE_EMAIL");
}

void EmailServiceConfigWidget::queryTemplate(const QString &key)
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_KEY, key);
    args.addQueryItem(WEB_QUERY_EMAIL_LANG, m_comManager->getCurrentPreferences().getLanguage());
    ui->frameEditor->hide();
    ui->frameSave->hide();
    m_emailComManager->doGet(EMAIL_TEMPLATE_PATH, args);
}


void EmailServiceConfigWidget::on_txtTemplate_textChanged()
{
    bool has_changes = ui->txtTemplate->toHtml() != ui->txtTemplate->property("original").toString();
    ui->frameSave->setEnabled(has_changes);
}


void EmailServiceConfigWidget::on_btnSave_clicked()
{

}


void EmailServiceConfigWidget::on_btnUndo_clicked()
{
    ui->txtTemplate->setHtml(ui->txtTemplate->property("original").toString());
    ui->txtTemplate->setProperty("original", ui->txtTemplate->toHtml());
}

