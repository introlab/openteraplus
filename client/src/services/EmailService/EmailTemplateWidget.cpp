#include "EmailTemplateWidget.h"
#include "ui_EmailTemplateWidget.h"

#include <QMenu>

EmailTemplateWidget::EmailTemplateWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EmailTemplateWidget)
{
    ui->setupUi(this);

    // Hide buttons... for now!
    ui->btnBold->hide();
    ui->btnItalic->hide();
    ui->btnColor->hide();
    ui->btnUnderline->hide();
    ui->frameTools->hide();

    initVariablesMenu();
}

EmailTemplateWidget::~EmailTemplateWidget()
{
    delete ui;
}

void EmailTemplateWidget::setEmailTemplate(const QString &text)
{
    ui->txtTemplate->setHtml(text);
    ui->txtTemplate->setProperty("original", ui->txtTemplate->toHtml());
    refreshPreview();
}

QString EmailTemplateWidget::getEmailTemplate() const
{
    return ui->txtTemplate->toHtml();
}

QTextEdit *EmailTemplateWidget::getPreview() const
{
    return ui->txtPreview;
}

void EmailTemplateWidget::setVariable(const QString &name, const QString &value)
{
    m_variablesValues[name] = value;
    refreshPreview();
}

bool EmailTemplateWidget::isEditing() const
{
    return ui->btnEdit->isChecked();
}

void EmailTemplateWidget::setEditing(const bool &editing)
{
    ui->btnEdit->setChecked(editing);
}

void EmailTemplateWidget::revert()
{
    ui->txtTemplate->setHtml(ui->txtTemplate->property("original").toString());
    ui->txtTemplate->setProperty("original", ui->txtTemplate->toHtml());
}

void EmailTemplateWidget::initVariablesMenu()
{
    if (!m_variablesMenu){
        m_variablesMenu = new QMenu(this);

        QAction* new_action = new QAction(tr("Nom participant"));
        new_action->setData("$participant");
        connect(new_action, &QAction::triggered, this, &EmailTemplateWidget::insertCurrentVariable);
        m_variablesMenu->addAction(new_action);
        m_variablesValues.insert(new_action->data().toString(), tr("Participant"));

        new_action = new QAction(tr("Nom utilisateur"));
        new_action->setData("$fullname");
        connect(new_action, &QAction::triggered, this, &EmailTemplateWidget::insertCurrentVariable);
        m_variablesMenu->addAction(new_action);
        m_variablesValues.insert(new_action->data().toString(), tr("Utilisateur"));

        new_action = new QAction(tr("Lien de connexion"));
        new_action->setData("$join_link");
        connect(new_action, &QAction::triggered, this, &EmailTemplateWidget::insertCurrentVariable);
        m_variablesMenu->addAction(new_action);
        m_variablesValues.insert(new_action->data().toString(), "<a href=\"#\">" + tr("cliquez ici") + "</a>");

        ui->btnVariables->setMenu(m_variablesMenu);
    }
}

void EmailTemplateWidget::refreshPreview()
{
    QString template_text = ui->txtTemplate->toHtml();

    // Replace variables in preview
    for(const QAction* action: m_variablesMenu->actions()){
        QString variable = action->data().toString();
        QString value = m_variablesValues.value(variable, "");
        template_text.replace(variable, value);
    }

    ui->txtPreview->setHtml(template_text);
}

void EmailTemplateWidget::insertCurrentVariable()
{
    QObject* sender = QObject::sender();
    if (!sender)
        return;

    QAction* action = dynamic_cast<QAction*>(sender);
    if (action){
        ui->txtTemplate->insertHtml(action->data().toString());
    }
}

void EmailTemplateWidget::on_txtTemplate_textChanged()
{
    bool is_dirty = ui->txtTemplate->toHtml() != ui->txtTemplate->property("original").toString();
    emit dirtyChanged(is_dirty);
}

void EmailTemplateWidget::on_btnEdit_clicked(bool checked)
{
    ui->frameTools->setVisible(checked);
    if (checked){
        ui->stackedEditor->setCurrentWidget(ui->pageEditor);
    }else{
        refreshPreview();
        ui->stackedEditor->setCurrentWidget(ui->pagePreview);
    }

    emit editingChanged(checked);
}

