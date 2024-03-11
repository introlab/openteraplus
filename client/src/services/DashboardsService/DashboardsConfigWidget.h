#ifndef DASHBOARDSCONFIGWIDGET_H
#define DASHBOARDSCONFIGWIDGET_H

#include <QWidget>
#include "DashboardsComManager.h"

namespace Ui {
class DashboardsConfigWidget;
}

class DashboardsConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardsConfigWidget(ComManager* comManager, const int& id_site, const int& id_project = 0, QWidget *parent = nullptr);
    ~DashboardsConfigWidget();

private:
    Ui::DashboardsConfigWidget *ui;

    ComManager*             m_comManager;
    DashboardsComManager*   m_dashComManager;

    int m_idProject;
    int m_idSite;

};

#endif // DASHBOARDSCONFIGWIDGET_H
