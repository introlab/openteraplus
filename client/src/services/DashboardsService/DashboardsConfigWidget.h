#ifndef DASHBOARDSCONFIGWIDGET_H
#define DASHBOARDSCONFIGWIDGET_H

#include <QWidget>
#include <QListWidgetItem>

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

    QHash<int, QListWidgetItem*>    m_listDashboards_items;

    int m_idProject;
    int m_idSite;

    void connectSignals();

private slots:
    void processDashboardsReply(QList<QJsonObject> dashboards, QUrlQuery reply_query);

    /*void on_lstDashboards_itemChanged(QListWidgetItem *item);
    void on_btnUpdateDashboards_clicked();*/
    void on_lstDashboards_itemClicked(QListWidgetItem *item);
    void on_cmbVersion_currentIndexChanged(int index);
};

#endif // DASHBOARDSCONFIGWIDGET_H
