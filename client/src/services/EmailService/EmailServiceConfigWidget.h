#ifndef EMAILSERVICECONFIGWIDGET_H
#define EMAILSERVICECONFIGWIDGET_H

#include <QWidget>
#include <QMenu>
#include "managers/ComManager.h"

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

private:
    void initVariablesMenu();

    Ui::EmailServiceConfigWidget *ui;

    ComManager*     m_comMan;
    QMenu*          m_variablesMenu = nullptr;
};

#endif // EMAILSERVICECONFIGWIDGET_H
