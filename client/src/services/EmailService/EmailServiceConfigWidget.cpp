#include "EmailServiceConfigWidget.h"
#include "ui_EmailServiceConfigWidget.h"
#include <QStyledItemDelegate>

EmailServiceConfigWidget::EmailServiceConfigWidget(ComManager *comManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EmailServiceConfigWidget)
{
    m_comMan = comManager;

    ui->setupUi(this);
    ui->cmbTemplate->setItemDelegate(new QStyledItemDelegate(ui->cmbTemplate));
    ui->btnBold->hide();
    ui->btnItalic->hide();
    ui->btnColor->hide();
    ui->btnUnderline->hide();

    initVariablesMenu();

}

EmailServiceConfigWidget::~EmailServiceConfigWidget()
{
    delete ui;
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

