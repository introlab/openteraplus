#include "CleanUpDialog.h"
#include "ui_CleanUpDialog.h"

CleanUpDialog::CleanUpDialog(ComManager *comManager, QVariantList &data, TeraDataTypes data_type, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CleanUpDialog),
    m_comManager(comManager),
    m_data(data),
    m_dataType(data_type)
{
    ui->setupUi(this);

    // Create column headers
    createTableColumns();

    // Connect signals
    connectSignals();

    // Set default features and refresh data
    setFeatures(false, true);
}

CleanUpDialog::~CleanUpDialog()
{
    delete ui;
}

void CleanUpDialog::setFeatures(const bool &enable_delete, const bool &enable_disable)
{
    m_allowDelete = enable_delete;
    m_allowDisable = enable_disable;

    ui->btnDeleteAll->setVisible(enable_delete);
    ui->btnDisableAll->setVisible(enable_disable);

    refreshData();
}

void CleanUpDialog::deleteResultOk(QString path, int id)
{
    if (path == TeraData::getPathForDataType(m_dataType)){
        // Remove row
        QTableWidgetItem* base_item = m_itemsMap.key(id);
        if (base_item){
            m_itemsMap.take(base_item);
            ui->tableItems->removeRow(base_item->row());
        }

        // Remove from list
        m_idsToManage.removeAll(id);

        // Send next item
        deleteNextItem();
    }
}

void CleanUpDialog::postResultOk(QString path)
{
    if (path == TeraData::getPathForDataType(m_dataType)){
        // Remove row
        int id = m_idsToManage.takeFirst();
        QTableWidgetItem* base_item = m_itemsMap.key(id);
        if (base_item){
            m_itemsMap.take(base_item);
            ui->tableItems->removeRow(base_item->row());

        }

        // Send next item
        disableNextItem();
    }
}

void CleanUpDialog::comOperationError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code)
{
    // Oops... Error... Clear pending list.
    m_idsToManage.clear();
}

void CleanUpDialog::createTableColumns()
{
    ui->tableItems->clear();
    m_columnsMap.clear();

    if (!m_data.isEmpty()){
        QStringList labels;
        QVariantHash first_data = m_data.first().toHash();

        if (first_data.contains("participant_name")){
            m_columnsMap["participant_name"] = labels.count();
            labels.append(tr("Nom"));
        }

        if (first_data.contains("user_fullname")){
            m_columnsMap["user_fullname"] = labels.count();
            labels.append(tr("Nom"));
        }


        if (first_data.contains("project_name")){
            m_columnsMap["project_name"] = labels.count();
            labels.append(tr("Projet"));
        }

        if (first_data.contains("last_updated")){
            m_columnsMap["last_updated"] = labels.count();
            labels.append(tr("Dernière mise à jour"));
        }

        if (first_data.contains("last_session")){
            m_columnsMap["last_session"] = labels.count();
            labels.append(tr("Dernière séance"));
        }

        if (first_data.contains("user_lastonline")){
            m_columnsMap["user_lastonline"] = labels.count();
            labels.append(tr("Date en ligne"));
        }

        if (first_data.contains("months")){
            m_columnsMap["months"] = labels.count();
            labels.append(tr("Mois"));
        }

        labels.append(tr("Actions"));
        ui->tableItems->setColumnCount(labels.count());
        ui->tableItems->setHorizontalHeaderLabels(labels);
    }
}

void CleanUpDialog::refreshData()
{
    ui->tableItems->clearContents();
    m_itemsMap.clear();

    for(const QVariant &data:std::as_const(m_data)){
        QVariantMap data_map = data.toMap();
        int current_row = ui->tableItems->rowCount();
        ui->tableItems->setRowCount(ui->tableItems->rowCount()+1);
        for (const QVariant &data_value: std::as_const(data_map)){
            QString data_key = data_map.key(data_value);
            if (m_columnsMap.contains(data_key)){
                QTableWidgetItem* item;
                if (data_key == "user_lastonline" || data_key == "last_updated" || data_key == "last_session"){
                        item = new TableDateWidgetItem();
                        dynamic_cast<TableDateWidgetItem*>(item)->setDate(data_value);
                }else{
                    if (data_key == "months"){
                        item = new TableNumberWidgetItem(data_value.toString());

                    }else{
                        item = new QTableWidgetItem(data_value.toString());
                    }
                }
                item->setTextAlignment(Qt::AlignCenter);
                ui->tableItems->setItem(current_row, m_columnsMap[data_key], item);
            }
        }

        m_itemsMap[ui->tableItems->item(current_row,0)] = data_map["id_" + TeraData::getDataTypeName(m_dataType)].toInt();

        // Add action buttons
        QFrame* action_frame = new QFrame();
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        layout->setAlignment(Qt::AlignLeft);
        action_frame->setLayout(layout);

        // Delete button
        if (m_allowDisable){
            QToolButton* btnDisable = new QToolButton();
            btnDisable->setIcon(QIcon("://controls/check_off.png"));
            btnDisable->setProperty("id", m_itemsMap[ui->tableItems->item(current_row,0)]);
            btnDisable->setCursor(Qt::PointingHandCursor);
            btnDisable->setMaximumWidth(32);
            btnDisable->setToolTip(tr("Désactiver"));
            connect(btnDisable, &QToolButton::clicked, this, &CleanUpDialog::btnDisable_clicked);
            layout->addWidget(btnDisable);
        }

        // Delete button
        if (m_allowDelete){
            QToolButton* btnDelete = new QToolButton();
            btnDelete->setIcon(QIcon(":/icons/delete_old.png"));
            btnDelete->setProperty("id", m_itemsMap[ui->tableItems->item(current_row,0)]);
            btnDelete->setCursor(Qt::PointingHandCursor);
            btnDelete->setMaximumWidth(32);
            btnDelete->setToolTip(tr("Supprimer"));
            connect(btnDelete, &QToolButton::clicked, this, &CleanUpDialog::btnDelete_clicked);
            layout->addWidget(btnDelete);
        }

        ui->tableItems->setCellWidget(current_row, ui->tableItems->columnCount()-1, action_frame);

    }
}

void CleanUpDialog::disableNextItem()
{
    if (m_idsToManage.isEmpty()){
        if (ui->tableItems->rowCount() == 0){
            // All done! We return
            accept();
        }
        return; // Nothing left to do for now
    }

    QJsonDocument document;
    QJsonObject data_obj;
    QJsonObject base_obj;

    data_obj.insert("id_" + TeraData::getDataTypeName(m_dataType), m_idsToManage.first());
    //data_obj.insert("id_project", QJsonValue::fromVariant(part_info->getFieldValue("id_project")));
    data_obj.insert(TeraData::getDataTypeName(m_dataType) + "_enabled", false);

    base_obj.insert(TeraData::getDataTypeName(m_dataType), data_obj);
    document.setObject(base_obj);

    m_comManager->doPost(TeraData::getPathForDataType(m_dataType), document.toJson());
}

void CleanUpDialog::deleteNextItem()
{
    if (m_idsToManage.isEmpty()){
        if (ui->tableItems->rowCount() == 0){
            // All done! We return
            accept();
        }
        return; // Nothing left to do for now!
    }

    m_comManager->doDelete(TeraData::getPathForDataType(m_dataType),  m_idsToManage.first());
}

void CleanUpDialog::on_btnOK_clicked()
{
    accept();
}

void CleanUpDialog::btnDisable_clicked()
{
    QToolButton* action_btn = dynamic_cast<QToolButton*>(sender());
    if (action_btn){
        // Select row according to the session id of that button
        int id_to_manage = action_btn->property("id").toInt();
        GlobalMessageBox diag;
        QMessageBox::StandardButton answer = QMessageBox::No;
        QTableWidgetItem* base_item = m_itemsMap.key(id_to_manage);
        answer = diag.showYesNo(tr("Désactivation?"), tr("Êtes-vous sûrs de vouloir désactiver """) + base_item->text() + """?");
        if (answer == QMessageBox::Yes){
           m_idsToManage.append(id_to_manage);
           disableNextItem();
        }
    }

}

void CleanUpDialog::btnDelete_clicked()
{
    QToolButton* action_btn = dynamic_cast<QToolButton*>(sender());
    if (action_btn){
        // Select row according to the session id of that button
        int id_to_manage = action_btn->property("id").toInt();

        GlobalMessageBox diag;
        QMessageBox::StandardButton answer = QMessageBox::No;
        QTableWidgetItem* base_item = m_itemsMap.key(id_to_manage);
        answer = diag.showYesNo(tr("Suppression?"), tr("Êtes-vous sûrs de vouloir supprimer """) + base_item->text() + """?");
        if (answer == QMessageBox::Yes){
            // We must delete!
           m_idsToManage.append(id_to_manage);
           deleteNextItem();
        }
    }
}

void CleanUpDialog::connectSignals()
{
    connect(m_comManager, &ComManager::deleteResultsOK, this, &CleanUpDialog::deleteResultOk);
    connect(m_comManager, &ComManager::postResultsOK, this, &CleanUpDialog::postResultOk);
    connect(m_comManager, &ComManager::networkError, this, &CleanUpDialog::comOperationError);
}


void CleanUpDialog::on_btnDisableAll_clicked()
{
    GlobalMessageBox diag;
    QMessageBox::StandardButton answer = QMessageBox::No;

    answer = diag.showYesNo(tr("Désactivation?"), tr("Êtes-vous sûrs de vouloir tout désactiver?"));
    if (answer == QMessageBox::Yes){
        for(int id:std::as_const(m_itemsMap)){
            m_idsToManage.append(id);
        }
        disableNextItem();
    }
}


void CleanUpDialog::on_btnDeleteAll_clicked()
{
    GlobalMessageBox diag;
    QMessageBox::StandardButton answer = QMessageBox::No;

    answer = diag.showYesNo(tr("Suppression?"), tr("Êtes-vous sûrs de vouloir tout supprimer?"));
    if (answer == QMessageBox::Yes){
        for(int id:std::as_const(m_itemsMap)){
            m_idsToManage.append(id);
        }
        deleteNextItem();
    }
}

