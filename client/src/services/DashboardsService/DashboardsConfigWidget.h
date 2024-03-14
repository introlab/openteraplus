#ifndef DASHBOARDSCONFIGWIDGET_H
#define DASHBOARDSCONFIGWIDGET_H

#include <QWidget>
#include <QListWidgetItem>

#include "DashboardsComManager.h"
#include "data/Message.h"

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
    void queryDashboard(const int& id_dashboard);
    void clearDetails();
    void validateDetails();

private slots:
    void processDashboardsReply(QList<QJsonObject> dashboards, QUrlQuery reply_query);
    void handleNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code);
    void dashComDeleteOK(QString path, int id);
    void dashComPostOK(QString path, QString data);

    void nextMessageWasShown(Message current_message);

    void on_lstDashboards_itemChanged(QListWidgetItem *item);
    void on_lstDashboards_itemClicked(QListWidgetItem *item);
    void on_cmbVersion_currentIndexChanged(int index);
    void on_btnEdit_toggled(bool checked);
    void on_btnCancel_clicked();
    void on_btnSave_clicked();
    void on_btnNewVersion_clicked();
    void on_btnNewDashboard_clicked();
    void on_txtName_textChanged(const QString &arg1);
    void on_txtDefinition_textChanged();
    void on_btnDeleteDashboard_clicked();
    void on_lstDashboards_currentRowChanged(int currentRow);
};

#endif // DASHBOARDSCONFIGWIDGET_H
