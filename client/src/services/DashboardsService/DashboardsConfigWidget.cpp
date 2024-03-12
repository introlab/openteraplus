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
    ui->wdgMessages->hide();

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
    connect(m_dashComManager, &DashboardsComManager::networkError, this, &DashboardsConfigWidget::handleNetworkError);
    connect(m_dashComManager, &DashboardsComManager::deleteResultsOK, this, &DashboardsConfigWidget::dashComDeleteOK);
    connect(m_dashComManager, &DashboardsComManager::postResultsOK, this, &DashboardsConfigWidget::dashComPostOK);
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
        m_listDashboards_projects.clear();
        m_listDashboards_sites.clear();

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
                    int id_site = site_obj["id_site"].toInt();
                    if (!checked)
                        checked = (id_site == m_idSite);
                    if (m_listDashboards_sites.contains(id_dashboard)){
                        m_listDashboards_sites[id_dashboard].append(id_site);
                    }else{
                        m_listDashboards_sites[id_dashboard] = QList<int>() << id_site;
                    }
                }
            }

            if (m_idProject>0){
                QJsonArray projects = dash["dashboard_projects"].toArray();
                for(QJsonValueRef project:projects){
                    QJsonObject proj_obj = project.toObject();
                    int id_project = proj_obj["id_project"].toInt();
                    if (!checked)
                        checked = (id_project == m_idProject);
                    if (m_listDashboards_projects.contains(id_dashboard)){
                        m_listDashboards_projects[id_dashboard].append(id_project);
                    }else{
                        m_listDashboards_projects[id_dashboard] = QList<int>() << id_project;
                    }
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

void DashboardsConfigWidget::handleNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code)
{
    Q_UNUSED(error)

    if (error == QNetworkReply::OperationCanceledError && op == QNetworkAccessManager::PostOperation){
        // Transfer was cancelled by user - no need to alert anyone!
        return;
    }

    if (error_msg.endsWith('\n'))
        error_msg = error_msg.left(error_msg.length()-1);

    //error_msg = QTextDocumentFragment::fromHtml(error_msg).toPlainText();

    QString error_str;

    if (status_code > 0)
        error_str = tr("Erreur HTTP ") + QString::number(status_code) + ": " + error_msg;
    else
        error_str = tr("Erreur ") + QString::number(error) + ": " + error_msg;

    error_str = "DashboardsService: " + error_str;
    error_str = error_str.replace('\n', " - ");
    error_str = error_str.replace('\r', "");
    ui->wdgMessages->addMessage(Message(Message::MESSAGE_ERROR, error_str));
}

void DashboardsConfigWidget::dashComDeleteOK(QString path, int id)
{

}

void DashboardsConfigWidget::dashComPostOK(QString path)
{
    ui->wdgMessages->addMessage(Message(Message::MESSAGE_OK, tr("Données sauvegardées")));
}

void DashboardsConfigWidget::on_lstDashboards_itemChanged(QListWidgetItem *item)
{
    ui->frameSpecific->setVisible(item->checkState() == Qt::Checked);

    // Do post to update projects and sites
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonObject data_obj;
    QJsonArray attached_items;

    int id_dashboard = item->data(Qt::UserRole).toInt();
    data_obj["id_dashboard"] = id_dashboard;

    QString update_field_name = "dashboard_sites";
    QString update_field_id_name = "id_site";
    if (m_idProject>0){
        update_field_name = "dashboard_projects";
        update_field_id_name = "id_project";

        if (item->checkState() == Qt::Checked){
            if (m_listDashboards_projects[id_dashboard].contains(m_idProject))
                return; // No need to update - all set!
            m_listDashboards_projects[id_dashboard].append(m_idProject);

        }else{
            if (!m_listDashboards_projects[id_dashboard].contains(m_idProject))
                return; // No need to update - all set!
            m_listDashboards_projects[id_dashboard].remove(m_idProject);
        }
        for(int item_id: m_listDashboards_projects[id_dashboard]){
            QJsonObject item;
            item[update_field_id_name] = item_id;
            attached_items.append(item);
        }
    }else{
        if (item->checkState() == Qt::Checked){
            if (m_listDashboards_sites[id_dashboard].contains(m_idSite))
                return; // No need to update - all set!
            m_listDashboards_sites[id_dashboard].append(m_idSite);

        }else{
            if (!m_listDashboards_sites[id_dashboard].contains(m_idSite))
                return; // No need to update - all set!
            m_listDashboards_sites[id_dashboard].remove(m_idSite);
        }
        for(int item_id: m_listDashboards_sites[id_dashboard]){
            QJsonObject item;
            item[update_field_id_name] = item_id;
            attached_items.append(item);
        }
    }
    data_obj[update_field_name] = attached_items;

    base_obj.insert("dashboard", data_obj);
    document.setObject(base_obj);
    m_dashComManager->doPost(DASHBOARDS_USER_PATH, document.toJson());
}

void DashboardsConfigWidget::on_lstDashboards_itemClicked(QListWidgetItem *item)
{
    // Query informations for that dashboard
    QUrlQuery args;
    int id_dashboard = m_listDashboards_items.key(item);
    args.addQueryItem(WEB_QUERY_ID_DASHBOARD, QString::number(id_dashboard));
    args.addQueryItem(WEB_QUERY_ALL_VERSIONS, "1");
    ui->frameEditor->hide();
    m_dashComManager->doGet(DASHBOARDS_USER_PATH, args);
}


void DashboardsConfigWidget::on_cmbVersion_currentIndexChanged(int index)
{
    ui->txtDefinition->setText(ui->cmbVersion->currentData().toString());
}

