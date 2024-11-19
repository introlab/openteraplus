#ifndef EMAILTEMPLATEWIDGET_H
#define EMAILTEMPLATEWIDGET_H

#include <QWidget>

namespace Ui {
class EmailTemplateWidget;
}

class EmailTemplateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EmailTemplateWidget(QWidget *parent = nullptr);
    ~EmailTemplateWidget();

    void setEmailTemplate(const QString& text);
    QString getEmailTemplate() const;

    void setVariable(const QString& name, const QString& value);

    void revert();

private:
    Ui::EmailTemplateWidget *ui;

    QMenu*                  m_variablesMenu = nullptr;
    QHash<QString, QString> m_variablesValues;

    void initVariablesMenu();
    void refreshPreview();

private slots:
    void insertCurrentVariable();
    void on_txtTemplate_textChanged();

    void on_btnEdit_clicked(bool checked);

signals:
    void dirtyChanged(bool dirty);
    void editingChanged(bool editing);

};

#endif // EMAILTEMPLATEWIDGET_H
