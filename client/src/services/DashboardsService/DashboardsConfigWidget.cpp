#include "DashboardsConfigWidget.h"
#include "ui_DashboardsConfigWidget.h"

DashboardsConfigWidget::DashboardsConfigWidget(ComManager *comManager, const int &id_site, const int &id_project, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DashboardsConfigWidget)
{
    ui->setupUi(this);
    m_comManager = comManager;
    m_dashComManager = new DashboardsComManager(comManager);

    m_idSite = id_site;
    m_idProject = id_project;

    connectSignals();

    // Init ui
    ui->frameEditor->hide();

    // Query dashboards
    QUrlQuery args;
    //args.addQueryItem(WEB_QUERY_LIST, "1");
    m_dashComManager->doGet(DASHBOARDS_USER_PATH, args);
}

DashboardsConfigWidget::~DashboardsConfigWidget()
{
    delete ui;
    delete m_dashComManager;
}

void DashboardsConfigWidget::connectSignals()
{
    connect(m_dashComManager, &DashboardsComManager::dashboardsReceived, this, &DashboardsConfigWidget::processDashboardsReply);
}

void DashboardsConfigWidget::processDashboardsReply(QList<QJsonObject> dashboards, QUrlQuery reply_query)
{
    if (reply_query.hasQueryItem(WEB_QUERY_ID_DASHBOARD)){
        // Querying specific dashboard
        if (dashboards.count() != 1){
            qWarning() << "DashboardsConfigWidget::processDashboardsReply - Was expecting one dashboard - got more!";
            return;
        }
        ui->txtName->setText(dashboards.first()["dashboard_name"].toString());
        ui->txtDescription->setPlainText(dashboards.first()["dashboard_description"].toString());
        ui->cmbVersion->clear();
        ui->cmbFixedVersion->clear();
        ui->cmbFixedVersion->addItem(tr("Aucune"));
        QJsonArray versions = dashboards.first()["versions"].toArray();
        for (const QJsonValueRef &version:versions){
            QJsonObject version_obj = version.toObject();
            QString version_str = QString::number(version_obj["dashboard_version"].toInt());
            ui->cmbVersion->addItem(version_str, version_obj["dashboard_definition"].toString());
            ui->cmbFixedVersion->addItem(version_str);
        }

        ui->chkEnabled->setChecked(false);
        QJsonArray projects = dashboards.first()["dashboard_projects"].toArray();
        for (const QJsonValueRef &project:projects){
            QJsonObject proj_obj = project.toObject();
            if (proj_obj["id_project"] == m_idProject){
                ui->chkEnabled->setChecked(proj_obj["dashboard_enabled"].toBool());
                int required_version = proj_obj["dashboard_required_version"].toInt();
                ui->cmbFixedVersion->setCurrentIndex(required_version);
                break;
            }
        }

        QJsonArray sites = dashboards.first()["dashboard_sites"].toArray();
        for (const QJsonValueRef &site:sites){
            QJsonObject site_obj = site.toObject();
            if (site_obj["id_site"] == m_idSite){
                ui->chkEnabled->setChecked(site_obj["dashboard_enabled"].toBool());
                int required_version = site_obj["dashboard_required_version"].toInt();
                ui->cmbFixedVersion->setCurrentIndex(required_version);
                break;
            }
        }

        if (ui->lstDashboards->currentItem())
            ui->frameSpecific->setVisible(ui->lstDashboards->currentItem()->checkState() == Qt::Checked);

        ui->frameEditor->show();

    }else{
        ui->lstDashboards->clear();
        m_listDashboards_items.clear();

        for (const QJsonObject &dash:std::as_const(dashboards)){
            QListWidgetItem* item = new QListWidgetItem();
            int id_dashboard = dash["id_dashboard"].toInt();
            item->setIcon(QIcon("://icons/history.png"));
            item->setData(Qt::UserRole, id_dashboard);
            item->setText(dash["dashboard_name"].toString());

            bool checked = false;/*dash["dashboard_sites"].toArray().contains(m_idSite) ||
                           dash["dashboard_projects"].toArray().contains(m_idProject);*/
            if (m_idSite>0){
                QJsonArray sites = dash["dashboard_sites"].toArray();
                for(QJsonValueRef site:sites){
                    QJsonObject site_obj = site.toObject();
                    checked = site_obj["id_site"].toInt() == m_idSite;
                    if (checked)
                        break;
                }
            }

            if (m_idProject>0){
                QJsonArray projects = dash["dashboard_projects"].toArray();
                for(QJsonValueRef project:projects){
                    QJsonObject proj_obj = project.toObject();
                    checked = proj_obj["id_project"].toInt() == m_idProject;
                    if (checked)
                        break;
                }
            }

            if (checked){
                item->setCheckState(Qt::Checked);
            }else{
                item->setCheckState(Qt::Unchecked);
            }
            ui->lstDashboards->addItem(item);
            m_listDashboards_items.insert(id_dashboard, item);
        }
    }
}

/*void DashboardsConfigWidget::on_lstDashboards_itemChanged(QListWidgetItem *item)
{
    bool has_changes = false;
    if (m_listDashboards_items.key(item) > 0 && item->checkState() == Qt::Unchecked){
        // Item deselected
        has_changes = true;
    }else{
        if (m_listDashboards_items.key(item, -1) <= 0 && item->checkState() == Qt::Checked){
            // Item selected
            has_changes = true;
        }
    }

    ui->btnUpdateDashboards->setEnabled(has_changes);
}*/

void DashboardsConfigWidget::on_lstDashboards_itemClicked(QListWidgetItem *item)
{
    // Query informations for that dashboard
    QUrlQuery args;
    int id_dashboard = m_listDashboards_items.key(item);
    args.addQueryItem(WEB_QUERY_ID_DASHBOARD, QString::number(id_dashboard));
    args.addQueryItem(WEB_QUERY_ALL_VERSIONS, "1");
    ui->frameSpecific->setVisible(item->checkState() == Qt::Checked);
    ui->frameEditor->hide();
    m_dashComManager->doGet(DASHBOARDS_USER_PATH, args);
}


void DashboardsConfigWidget::on_cmbVersion_currentIndexChanged(int index)
{
    ui->txtDefinition->setText(ui->cmbVersion->currentData().toString());
}

