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
    QHash<int, QList<int>>          m_listDashboards_projects;
    QHash<int, QList<int>>          m_listDashboards_sites;

    int m_idProject;
    int m_idSite;

    void connectSignals();

private slots:
    void processDashboardsReply(QList<QJsonObject> dashboards, QUrlQuery reply_query);
    void handleNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code);
    void dashComDeleteOK(QString path, int id);
    void dashComPostOK(QString path);

    void on_lstDashboards_itemChanged(QListWidgetItem *item);

    void on_lstDashboards_itemClicked(QListWidgetItem *item);
    void on_cmbVersion_currentIndexChanged(int index);
};

#endif // DASHBOARDSCONFIGWIDGET_H
