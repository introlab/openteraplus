#include "DashboardsConfigWidget.h"
#include "ui_DashboardsConfigWidget.h"
#include "DashboardsWebAPI.h"

#include "GlobalMessageBox.h"

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
    bool is_super = m_comManager->isCurrentUserSuperAdmin();
    ui->frameEditor->hide();
    ui->wdgMessages->hide();
    ui->frameDashboardButtons->setVisible(is_super);
    ui->frameButtons->hide();
    ui->frameVersionsInfos->setVisible(is_super);
    ui->frameVersionsButtons->hide();
    ui->btnDeleteVersion->hide();
    m_highlighter = new QSourceHighlite::QSourceHighliter(ui->txtDefinition->document(), QSourceHighlite::QSourceHighliter::Monokai);
    m_highlighter->setCurrentLanguage(QSourceHighlite::QSourceHighliter::CodeJSON);


}

DashboardsConfigWidget::~DashboardsConfigWidget()
{
    delete ui;
    delete m_dashComManager;
    m_highlighter->deleteLater();
}

void DashboardsConfigWidget::refresh()
{
    ui->frameDashboardButtons->hide();

    if (!m_dashComManager->isReady()){
        m_refreshRequested = true;
        return;
    }

    // Query dashboards
    QUrlQuery args;
    //args.addQueryItem(WEB_QUERY_LIST, "1");
    args.addQueryItem(WEB_QUERY_ENABLED, "0"); // Include dashboards that are not enabled
    if (!m_comManager->isCurrentUserSuperAdmin()){
        // Only query dashboards associated with that item
        if (m_idSite > 0)
            args.addQueryItem(WEB_QUERY_ID_SITE, QString::number(m_idSite));
        if (m_idProject > 0)
            args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_idProject));
    }
    m_dashComManager->doGet(DASHBOARDS_USER_PATH, args);
}

void DashboardsConfigWidget::connectSignals()
{
    connect(m_dashComManager, &DashboardsComManager::readyChanged, this, &DashboardsConfigWidget::dashComReady);
    connect(m_dashComManager, &DashboardsComManager::dashboardsReceived, this, &DashboardsConfigWidget::processDashboardsReply);
    connect(m_dashComManager, &DashboardsComManager::networkError, this, &DashboardsConfigWidget::handleNetworkError);
    connect(m_dashComManager, &DashboardsComManager::deleteResultsOK, this, &DashboardsConfigWidget::dashComDeleteOK);
    connect(m_dashComManager, &DashboardsComManager::postResultsOK, this, &DashboardsConfigWidget::dashComPostOK);

    connect(ui->wdgMessages, &ResultMessageWidget::nextMessageShown, this, &DashboardsConfigWidget::nextMessageWasShown);
}

void DashboardsConfigWidget::queryDashboard(const int &id_dashboard)
{
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_DASHBOARD, QString::number(id_dashboard));
    args.addQueryItem(WEB_QUERY_ALL_VERSIONS, "1");
    ui->frameEditor->hide();
    m_dashComManager->doGet(DASHBOARDS_USER_PATH, args);
}

void DashboardsConfigWidget::clearDetails()
{
    ui->txtName->clear();
    ui->txtDescription->clear();
    ui->cmbVersion->clear();
    ui->txtDefinition->clear();

}

void DashboardsConfigWidget::validateDetails()
{
    bool valid = false;
    valid = !ui->txtName->text().isEmpty() && !ui->txtDefinition->toPlainText().isEmpty();

    ui->btnSave->setEnabled(valid);
}

void DashboardsConfigWidget::processDashboardsReply(QList<QJsonObject> dashboards, QUrlQuery reply_query)
{
    ui->frameDashboardButtons->show();
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

        // Select latest version
        ui->cmbVersion->setCurrentIndex(ui->cmbVersion->count()-1);

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

        if (ui->lstDashboards->currentItem()){
            ui->frameSpecific->setVisible(ui->lstDashboards->currentItem()->checkState() == Qt::Checked ||
                                          !ui->lstDashboards->currentItem()->flags().testFlags(Qt::ItemIsUserCheckable));
        }

        ui->frameEditor->show();

    }else{
        for (const QJsonObject &dash:std::as_const(dashboards)){
            int id_dashboard = dash["id_dashboard"].toInt();
            QListWidgetItem* item;
            if (m_listDashboards_items.contains(id_dashboard)){
                item = m_listDashboards_items[id_dashboard];
            }else{
                item = new QListWidgetItem();

                item->setIcon(QIcon("://icons/history.png"));
                item->setData(Qt::UserRole, id_dashboard);
                m_listDashboards_items.insert(id_dashboard, item);
            }
            item->setText(dash["dashboard_name"].toString());

            bool checked = false;
            bool checkable = false;
            if (m_idSite>0){
                QJsonArray sites = dash["dashboard_sites"].toArray();
                if (m_listDashboards_sites.contains(id_dashboard)){
                    m_listDashboards_sites[id_dashboard].clear();
                }
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
                checkable = sites.count() > 1 || m_comManager->isCurrentUserSuperAdmin();
            }

            if (m_idProject>0){
                QJsonArray projects = dash["dashboard_projects"].toArray();
                if (m_listDashboards_projects.contains(id_dashboard)){
                    m_listDashboards_projects[id_dashboard].clear();
                }
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
                checkable = projects.count() > 1 || m_comManager->isCurrentUserSuperAdmin();
            }

            if (checkable){
                if (checked){
                    item->setCheckState(Qt::Checked);
                }else{
                    item->setCheckState(Qt::Unchecked);
                }
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
            }else{
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            }
            ui->lstDashboards->addItem(item);
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
    if (path == DASHBOARDS_USER_PATH && m_listDashboards_items.contains(id)){
        delete m_listDashboards_items[id];
        m_listDashboards_items.remove(id);
        m_listDashboards_projects.remove(id);
        m_listDashboards_sites.remove(id);
        ui->lstDashboards->setCurrentRow(-1);
        ui->frameEditor->hide();
    }
}

void DashboardsConfigWidget::dashComPostOK(QString path, QString data)
{
    ui->wdgMessages->addMessage(Message(Message::MESSAGE_OK, tr("Données sauvegardées")));
    ui->btnEdit->setChecked(false);
    ui->frameDashboards->show();

    // Ensure item is selected in list
    QJsonParseError json_error;
    QJsonDocument data_list = QJsonDocument::fromJson(data.toUtf8(), &json_error);
    if (json_error.error == QJsonParseError::NoError){
        int id_dashboard = data_list.object()["id_dashboard"].toInt();
        if (m_listDashboards_items.contains(id_dashboard)){
            ui->lstDashboards->setCurrentItem(m_listDashboards_items[id_dashboard]);
        }
    }
}

void DashboardsConfigWidget::dashComReady(bool ready)
{
    if (ready){
        if (m_refreshRequested){
            m_refreshRequested = false;
            refresh();
        }
    }
}

void DashboardsConfigWidget::nextMessageWasShown(Message current_message)
{
    if (current_message.getMessageType()==Message::MESSAGE_NONE){
        ui->wdgMessages->hide();
    }else{
        ui->wdgMessages->show();
    }
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
            m_listDashboards_projects[id_dashboard].removeAll(m_idProject);
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
            m_listDashboards_sites[id_dashboard].removeAll(m_idSite);
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
    int id_dashboard = m_listDashboards_items.key(item);
    queryDashboard(id_dashboard);
}

void DashboardsConfigWidget::on_cmbVersion_currentIndexChanged(int index)
{
    ui->txtDefinition->setText(ui->cmbVersion->currentData().toString());
    ui->txtDefinition->setReadOnly(!(index == ui->cmbVersion->count()-1 && ui->btnEdit->isChecked())); // Only allow editing of latest version
    if (ui->txtDefinition->isReadOnly()){
        ui->txtDefinition->setStyleSheet("background-color: rgba(255,255,255,40%);");
    }else{
        ui->txtDefinition->setStyleSheet("background-color: rgba(255,255,255,25%); color: black;");
    }
}

void DashboardsConfigWidget::on_btnEdit_toggled(bool checked)
{
    ui->frameDashboards->setDisabled(checked);
    if (m_comManager->isCurrentUserSuperAdmin()){
        ui->frameDashboardDetails->setEnabled(checked);
        ui->frameVersionsButtons->setVisible(checked);
        ui->txtDefinition->setReadOnly(!(checked && ui->cmbVersion->currentIndex() == ui->cmbVersion->count()-1));
        if (ui->txtDefinition->isReadOnly()){
            ui->txtDefinition->setStyleSheet(" ");
        }else{
            ui->txtDefinition->setStyleSheet("background-color: rgba(255,255,255,50%); color: black;");
        }
    }
    ui->frameSpecific->setEnabled(checked);
    ui->frameButtons->setVisible(checked);
    ui->btnEdit->setVisible(!checked);
    validateDetails();
}

void DashboardsConfigWidget::on_btnCancel_clicked()
{
    // Refresh data
    ui->btnEdit->setChecked(false);
    if (ui->lstDashboards->currentItem()){
        int id_dashboard = m_listDashboards_items.key(ui->lstDashboards->currentItem());
        queryDashboard(id_dashboard);
    }else{
        ui->frameDashboards->show();
        ui->frameEditor->hide();
    }
}

void DashboardsConfigWidget::on_btnSave_clicked()
{
    QJsonDocument document;
    QJsonObject base_obj;
    QJsonObject data_obj;
    QJsonArray attached_items;

    QListWidgetItem* item = ui->lstDashboards->currentItem();

    int id_dashboard = 0;
    if (item){
        id_dashboard = item->data(Qt::UserRole).toInt(); // Editing item
    }

    data_obj["id_dashboard"] = id_dashboard;

    if (m_comManager->isCurrentUserSuperAdmin()){
        data_obj["dashboard_name"] = ui->txtName->text();
        data_obj["dashboard_description"] = ui->txtDescription->toPlainText();

        // Updated versions
        if (ui->cmbVersion->currentData().toString() != ui->txtDefinition->toPlainText() || id_dashboard == 0){
            data_obj["dashboard_version"] = ui->cmbVersion->count();
            data_obj["dashboard_definition"] = ui->txtDefinition->toPlainText();
        }
    }

    if (ui->frameSpecific->isVisible()){
        // Generate projects / sites list with enabled and fixed version
        if (m_idProject>0){
            if (id_dashboard > 0){
                for(int item_id: m_listDashboards_projects[id_dashboard]){
                    QJsonObject item;
                    item["id_project"] = item_id;
                    if (item_id == m_idProject){
                        item["dashboard_project_enabled"] = ui->chkEnabled->isChecked();
                        item["dashboard_project_version"] = ui->cmbFixedVersion->currentIndex();
                    }
                    attached_items.append(item);
                }
            }else{
                // New dashboard - append current
                QJsonObject item;
                item["id_project"] = m_idProject;
                item["dashboard_project_enabled"] = ui->chkEnabled->isChecked();
                item["dashboard_project_version"] = ui->cmbFixedVersion->currentIndex();
                attached_items.append(item);
            }
            data_obj["dashboard_projects"] = attached_items;
        }else{
            if (id_dashboard > 0){
                for(int item_id: m_listDashboards_sites[id_dashboard]){
                    QJsonObject item;
                    item["id_site"] = item_id;
                    if (item_id == m_idSite){
                        item["dashboard_site_enabled"] = ui->chkEnabled->isChecked();
                        item["dashboard_site_version"] = ui->cmbFixedVersion->currentIndex();
                    }
                    attached_items.append(item);
                }
            }else{
                // New dashboard - append current
                QJsonObject item;
                item["id_site"] = m_idSite;
                item["dashboard_project_enabled"] = ui->chkEnabled->isChecked();
                item["dashboard_project_version"] = ui->cmbFixedVersion->currentIndex();
                attached_items.append(item);
            }
            data_obj["dashboard_sites"] = attached_items;
        }
    }

    base_obj.insert("dashboard", data_obj);
    document.setObject(base_obj);
    m_dashComManager->doPost(DASHBOARDS_USER_PATH, document.toJson());
}

void DashboardsConfigWidget::on_btnNewVersion_clicked()
{
    ui->cmbVersion->addItem(QString::number(ui->cmbVersion->count()+1), "");
    ui->cmbVersion->setCurrentIndex(ui->cmbVersion->count()-1);
}

void DashboardsConfigWidget::on_btnNewDashboard_clicked()
{
    ui->lstDashboards->setCurrentRow(-1);
    ui->btnEdit->setChecked(true);
    clearDetails();
    ui->cmbVersion->addItem("1", "{}");
    ui->frameDashboards->hide();
    ui->frameSpecific->show();
    on_txtName_textChanged("");
    ui->chkEnabled->setChecked(true);
    ui->frameEditor->show();

}

void DashboardsConfigWidget::on_txtName_textChanged(const QString &arg1)
{
    if (ui->btnEdit->isChecked()){
        if (ui->txtName->text().isEmpty()){
            ui->txtName->setStyleSheet("background-color: #ffaaaa; color:black;");
        }else{
            ui->txtName->setStyleSheet(" ");
        }
        validateDetails();
    }
}

void DashboardsConfigWidget::on_txtDefinition_textChanged()
{
    if (ui->btnEdit->isChecked()){
        if (ui->txtDefinition->toPlainText().isEmpty()){
            ui->txtDefinition->setStyleSheet("background-color: #ffaaaa; color:black;");
        }else{
            ui->txtDefinition->setStyleSheet(" ");
        }
        validateDetails();
    }
}

void DashboardsConfigWidget::on_btnDeleteDashboard_clicked()
{
    if (!ui->lstDashboards->currentItem())
        return;

    GlobalMessageBox msgbox;

    GlobalMessageBox msg;
    QMessageBox::StandardButton conf = msg.showYesNo(tr("Suppression"), tr("Êtes-vous sûr de vouloir supprimer le tableau de bord sélectionné?"));

    if (conf == QMessageBox::Yes){
        int id_dashboard = ui->lstDashboards->currentItem()->data(Qt::UserRole).toInt();
        m_dashComManager->doDelete(DASHBOARDS_USER_PATH, id_dashboard);
    }
}

void DashboardsConfigWidget::on_lstDashboards_currentRowChanged(int currentRow)
{
    ui->btnDeleteDashboard->setEnabled(currentRow>=0);
}

