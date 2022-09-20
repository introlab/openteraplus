#include <QStandardPaths>

#include "TestsWidget.h"
#include "ui_TestsWidget.h"

TestsWidget::TestsWidget(ComManager *comMan, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TestsWidget)
{
    ui->setupUi(this);

    m_idProject = -1;
    m_idSession = -1;
    m_viewMode = VIEWMODE_UNKNOWN;
    setComManager(comMan);

    connectSignals();

    // Hide add button by default
    ui->wdgMessages->hide();

    // Hide summary by default
    ui->tableTestSummary->hide();

    // Catch keyboard events
    ui->tableTests->installEventFilter(this);

    ui->btnNew->hide();

    // Initialize default directory for file dialog
    QStringList documents_path = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);

    if (!documents_path.isEmpty())
        m_fileDialog.setDirectory(documents_path.first());


}

TestsWidget::~TestsWidget()
{
    delete ui;
    qDeleteAll(m_tests);

}

void TestsWidget::setComManager(ComManager *comMan)
{
    m_comManager = comMan;

    if (m_comManager){
        connect(m_comManager, &ComManager::testsReceived, this, &TestsWidget::processTestsReply);
        connect(m_comManager, &ComManager::servicesReceived, this, &TestsWidget::processServicesReply);
        connect(m_comManager, &ComManager::deleteResultsOK, this, &TestsWidget::comDeleteDataReply);
    }
}

void TestsWidget::setAssociatedProject(const int &id_project)
{
    m_idProject = id_project;
}

bool TestsWidget::isEmpty() const
{
    return m_tests.isEmpty();
}

void TestsWidget::clearTests()
{
    ui->tableTests->clearContents();
    ui->tableTests->setRowCount(0);
    qDeleteAll(m_tests);
    m_tests.clear();
    m_tableTests.clear();
}

void TestsWidget::enableNewTests(const bool &enable)
{
    // Default - no setted id project or enable = false - not implemented yet!
    ui->btnNew->setVisible(false);
}

void TestsWidget::displayTestsForSession(const int &id_session)
{
    if (!m_comManager)
        return;

    if (m_viewMode == VIEWMODE_SESSION)
        return; // Already in that mode!

    clearTests();

    setViewMode(VIEWMODE_SESSION);

    setLoading(true, tr("Chargement des données en cours..."), true);

    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(id_session));
    m_dataQuery = query;

    query.addQueryItem(WEB_QUERY_WITH_URLS, "1");
    query.addQueryItem(WEB_QUERY_FULL, "1"); // Include full information, including service name
    m_comManager->doGet(WEB_TESTINFO_PATH, query);

    m_idSession = id_session;
}

void TestsWidget::displayTestsForParticipant(const int &id_participant)
{
    if (!m_comManager)
        return;

    if (m_viewMode == VIEWMODE_PARTICIPANT)
        return; // Already in that mode!

    clearTests();

    setViewMode(VIEWMODE_PARTICIPANT);

    setLoading(true, tr("Chargement des données en cours..."), true);

    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_PARTICIPANT, QString::number(id_participant));
    m_dataQuery = query;

    query.addQueryItem(WEB_QUERY_WITH_URLS, "1");
    query.addQueryItem(WEB_QUERY_FULL, "1"); // Include full information, including service name
    m_comManager->doGet(WEB_TESTINFO_PATH, query);

}

void TestsWidget::connectSignals()
{
    // Com related signals are set in setComManager
    connect(ui->wdgMessages, &ResultMessageWidget::nextMessageShown, this, &TestsWidget::nextMessageWasShown);
}

bool TestsWidget::eventFilter(QObject *o, QEvent *e)
{
    if( o == ui->tableTests && e->type() == QEvent::KeyRelease )
    {
        QKeyEvent* key_event = dynamic_cast<QKeyEvent*>(e);
        if (key_event){
            if (key_event->key() == Qt::Key_Delete){
                on_btnDelete_clicked();
            }
        }
    }

    return QWidget::eventFilter(o,e);
}

void TestsWidget::updateTest(const TeraData &test, const int &id_participant, const bool &emit_count_update_signal)
{
    QTableWidgetItem* session_item;
    QTableWidgetItem* type_item;
    QTableWidgetItem* name_item;
    QTableWidgetItem* status_item;
    TableDateWidgetItem* date_item;
    QTableWidgetItem* owner_item;

    QString test_uuid = test.getUuid();

    // Check to display test correctly
    if (m_tableTests.contains(test_uuid)){
        name_item = m_tableTests[test_uuid];
        int current_row = name_item->row();
        status_item = ui->tableTests->item(current_row, TEST_STATUS_COLUMN);
        date_item = dynamic_cast<TableDateWidgetItem*>(ui->tableTests->item(current_row, TEST_DATE_COLUMN));
        owner_item = ui->tableTests->item(current_row, TEST_OWNER_COLUMN);
        session_item = ui->tableTests->item(current_row, TEST_SESSION_COLUMN);
        type_item = ui->tableTests->item(current_row, TEST_TYPE_COLUMN);
    }else{
        // Add test info to internal list
        if (!m_tests.contains(test_uuid))
            m_tests.insert(test_uuid, new TeraData(test));
        else{
            // Test already present - merge infos
            m_tests[test_uuid]->updateFrom(test);
        }

        ui->tableTests->setRowCount(ui->tableTests->rowCount()+1);
        int current_row = ui->tableTests->rowCount()-1;
        name_item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_TEST)),"");
        ui->tableTests->setItem(current_row, TEST_NAME_COLUMN, name_item);
        status_item = new QTableWidgetItem("");
        status_item->setTextAlignment(Qt::AlignCenter);
        ui->tableTests->setItem(current_row, TEST_STATUS_COLUMN, status_item);
        date_item = new TableDateWidgetItem("");
        ui->tableTests->setItem(current_row, TEST_DATE_COLUMN, date_item);
        owner_item = new QTableWidgetItem("");
        ui->tableTests->setItem(current_row, TEST_OWNER_COLUMN, owner_item);
        session_item = new QTableWidgetItem("");
        ui->tableTests->setItem(current_row, TEST_SESSION_COLUMN, session_item);
        type_item = new QTableWidgetItem("");
        ui->tableTests->setItem(current_row, TEST_TYPE_COLUMN, type_item);

        // Create action buttons
       /* QFrame* action_frame = new QFrame();
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        layout->setAlignment(Qt::AlignLeft);
        action_frame->setLayout(layout);*/

        // Delete
        /*QToolButton* btnDelete = new QToolButton();
        btnDelete->setIcon(QIcon(":/icons/delete_old.png"));
        btnDelete->setIconSize(QSize(24,24));
        btnDelete->setProperty("test_uuid", test.getUuid());
        btnDelete->setCursor(Qt::PointingHandCursor);
        btnDelete->setMaximumWidth(32);
        btnDelete->setToolTip(tr("Supprimer"));
        connect(btnDelete, &QToolButton::clicked, this, &TestsWidget::on_btnDelete_clicked);
        layout->addWidget(btnDelete);

        ui->treeAssets->setItemWidget(item, ui->treeAssets->columnCount()-1, action_frame);*/

        // Update internal lists
        m_tableTests.insert(test.getUuid(), name_item);

        // Emit update signal
        if (emit_count_update_signal)
            emit testCountChanged(m_tests.count());
    }

    // Update informations
    name_item->setText(test.getName());
    status_item->setText(TeraTest::getTestStatusName(test.getFieldValue("test_status").toInt()));
    status_item->setForeground(QColor(TeraTest::getTestStatusColor(test.getFieldValue("test_status").toInt())));
    date_item->setDate(test.getFieldValue("test_datetime"));

    if (test.hasFieldName("test_session_name")){
        session_item->setText(test.getFieldValue("test_session_name").toString());
    }

    if (test.hasFieldName("test_test_type_name")){
        type_item->setText(test.getFieldValue("test_test_type_name").toString());
    }

    // Owner (creator) of that test
    if (test.hasFieldName("test_participant"))
        owner_item->setText(tr("Participant:") + " " + test.getFieldValue("test_participant").toString());
    else if(test.hasFieldName("test_device"))
        owner_item->setText(tr("Appareil: ") + test.getFieldValue("test_device").toString());
    else if(test.hasFieldName("test_user"))
        owner_item->setText(tr("Utilisateur: ") + test.getFieldValue("test_user").toString());
    else if(test.hasFieldName("test_service"))
        owner_item->setText(tr("Service: ") + test.getFieldValue("test_service").toString());
    else {
        owner_item->setText(tr("Inconnu"));
    }

    //btnView->setEnabled(has_asset_infos_url);
}

void TestsWidget::setViewMode(const ViewMode &new_mode)
{
    m_viewMode = new_mode;
    if (new_mode == VIEWMODE_SESSION)
        ui->tableTests->hideColumn(TEST_SESSION_COLUMN); // Hide session column
    else
        ui->tableTests->showColumn(TEST_SESSION_COLUMN);

}

void TestsWidget::setLoading(const bool &loading, const QString &msg, const bool &hide_ui)
{
    ui->tableTests->setEnabled(!loading);
    ui->frameButtons->setEnabled(!loading);

    if (hide_ui && loading){
        ui->tableTests->setVisible(false);
        ui->frameButtons->setVisible(false);
    }else{
        ui->tableTests->setVisible(true);
        ui->frameButtons->setVisible(true);
    }

    if (loading){
        ui->wdgMessages->addMessage(Message(Message::MESSAGE_WORKING, msg));
    }else{
        ui->wdgMessages->clearMessages();
    }
}

void TestsWidget::showTestSummary(const QString &test_uuid)
{
    // Reset values
    ui->tableTestSummary->clearContents();
    ui->tableTestSummary->setRowCount(0);

    // Load summary
    TeraData* test = m_tests[test_uuid];
    if (test){
        QJsonDocument test_summary_doc = QJsonDocument::fromJson(test->getFieldValue("test_summary").toString().toUtf8());
        if (!test_summary_doc.isNull()){
            QVariantMap test_summary = test_summary_doc.object().toVariantMap();

            QVariantMap::const_iterator test_result = test_summary.constBegin();
            while (test_result != test_summary.constEnd()) {
                ui->tableTestSummary->setRowCount(ui->tableTestSummary->rowCount()+1);
                int current_row = ui->tableTestSummary->rowCount()-1;
                ui->tableTestSummary->setItem(current_row, 0, new QTableWidgetItem(test_result.key()));
                ui->tableTestSummary->setItem(current_row, 1, new QTableWidgetItem(test_result.value().toString()));
                ++test_result;
            }

        }

    }

}

void TestsWidget::updateTestsCountLabel()
{
    QString suffix;
    if (m_tests.count()>1){
        suffix = tr("éléments");
    }else{
        suffix = tr("élément");
    }
    ui->lblTestsCount->setText(QString::number(m_tests.count()) + " " + suffix);

    ui->btnDownloadAll->setEnabled(m_tests.count()>0);
}

void TestsWidget::processTestsReply(QList<TeraData> tests, QUrlQuery reply_query)
{
    if (reply_query.hasQueryItem(WEB_QUERY_WITH_ONLY_TOKEN)){
        // Only update access token!
        foreach(TeraData test, tests){
            QString test_uuid = test.getUuid();
            QString access_token = test.getFieldValue("access_token").toString();
            if (m_tests.contains(test_uuid)){
                m_tests[test_uuid]->setFieldValue("access_token", access_token);
            }
            // Start refresh timer
            // m_refreshTokenTimer.start();
        }
    }else{
        bool empty_tests = m_tests.isEmpty();
        foreach(TeraData test, tests){
            // Participant name
            int id_participant = -1;
            if (reply_query.hasQueryItem(WEB_QUERY_ID_PARTICIPANT)){
                id_participant = reply_query.queryItemValue(WEB_QUERY_ID_PARTICIPANT).toInt();
            }
            updateTest(test, id_participant, !empty_tests); // No count change signal on new query
        }

        updateTestsCountLabel();

        // Query extra information
        //queryAssetsInfos();

        /*if (!m_refreshTokenTimer.isActive())
            m_refreshTokenTimer.start();*/
    }
    ui->tableTests->resizeColumnsToContents();

    setLoading(false);

}

void TestsWidget::processServicesReply(QList<TeraData> services, QUrlQuery reply_query)
{
    for(const TeraData &service:services){
       if (service.getFieldValue("service_key").toString() == "FileTransferService"){
           // Check if project is in service_fields
           if (service.hasFieldName("service_projects")){
               QVariantList projects = service.getFieldValue("service_projects").toList();
               for (const QVariant &project:qAsConst(projects)){
                   QVariantMap project_info = project.toMap();
                   if (project_info["id_project"].toInt() == m_idProject){
                       // Ok, we are allowed to use file transfer service
                       /*if (m_fileTransferServiceInfos)
                           delete m_fileTransferServiceInfos;
                       m_fileTransferServiceInfos = new TeraData(service);
                       ui->btnNew->setVisible(true);*/
                       break;
                   }
               }
           }
       }
    }
}

void TestsWidget::comDeleteDataReply(QString path, int id)
{
    if (path == WEB_TESTINFO_PATH){
        // Remove test from list
        for(TeraData* test_info:qAsConst(m_tests)){
            if (test_info->getId() == id){
                QString test_uuid = test_info->getUuid();
                ui->tableTests->removeRow(m_tableTests[test_uuid]->row());
                m_tableTests.remove(test_uuid);
                delete m_tests.take(test_uuid);

                // Emit update signal
                emit testCountChanged(m_tests.count());
                updateTestsCountLabel();
                break;
            }
        }
    }
}

void TestsWidget::nextMessageWasShown(Message current_message)
{
    if (current_message.getMessageType()==Message::MESSAGE_NONE){
        ui->wdgMessages->hide();
    }else{
        ui->wdgMessages->show();
    }
}

/*void AssetsWidget::processAssetsInfos(QList<QJsonObject> infos, QUrlQuery reply_query, QString reply_path)
{
    for (const QJsonObject &asset_info:qAsConst(infos)){
        QString asset_uuid = asset_info["asset_uuid"].toString();

        if (!m_treeAssets.contains(asset_uuid)){
            // Create a new asset, if possible (should happen if we posted a new asset)
            TeraData asset(TERADATA_ASSET, asset_info);
            updateAsset(asset); // TODO: manage participant association, if required by the view
        }

        if (m_treeAssets.contains(asset_uuid)){
            QTreeWidgetItem* asset_item = m_treeAssets[asset_uuid];

            if (asset_info.contains("asset_file_size")){

                asset_item->setText(AssetColumn::ASSET_SIZE, Utils::formatFileSize(asset_info["asset_file_size"].toInt()));
                ui->treeAssets->showColumn(AssetColumn::ASSET_SIZE);
            }

            if (asset_info.contains("file_size")){
                asset_item->setText(AssetColumn::ASSET_SIZE, Utils::formatFileSize(asset_info["file_size"].toInt()));
                ui->treeAssets->showColumn(AssetColumn::ASSET_SIZE);
            }

            if (asset_info.contains("video_duration")){
                asset_item->setText(AssetColumn::ASSET_DURATION, Utils::formatDuration(asset_info["video_duration"].toString()));
                ui->treeAssets->showColumn(AssetColumn::ASSET_DURATION);
            }
        }

        if (m_assets.contains(asset_uuid)){
            // Append received fields to current asset
            TeraData* asset = m_assets[asset_uuid];
            asset->updateFrom(asset_info);
        }
    }
    resizeAssetsColumnsToContent();

    setLoading(false);
}

void AssetsWidget::refreshAccessToken()
{
    if (!m_comManager)
        return;

    QUrlQuery query = m_dataQuery; // Original data query

    query.addQueryItem(WEB_QUERY_WITH_ONLY_TOKEN, "1");
    m_comManager->doGet(WEB_ASSETINFO_PATH, query);
}

void AssetsWidget::on_treeAssets_itemSelectionChanged()
{
    bool at_least_one_asset_selected = false;

    // Make sure we don't have only "labels" items (session, participant, ...) selected
    const QList<QTreeWidgetItem*> selected_items = ui->treeAssets->selectedItems();
    for(QTreeWidgetItem* item:selected_items){
        if (!m_treeAssets.key(item).isEmpty() ){
            at_least_one_asset_selected = true;
            break;
        }
    }
    ui->btnDelete->setEnabled(at_least_one_asset_selected);
    ui->btnDownload->setEnabled(at_least_one_asset_selected);
}


void AssetsWidget::on_btnNew_clicked()
{
    // Upload a new asset to the FileTransfer service
    if (!m_fileTransferServiceInfos || !m_comAssetManager || !m_comManager){
        return; // Should not happen at this point.
    }

    if (m_uploadDialog){
        m_uploadDialog->deleteLater();
    }

    // Open upload dialog
    m_uploadDialog = new FileUploaderDialog(tr("Tous (*.*)"), dynamic_cast<QWidget*>(this));
    if (m_uploadDialog->result() == QDialog::Rejected){
        // No file selected - no need to upload anything!
        m_uploadDialog->deleteLater();
        m_uploadDialog = nullptr;
        return;
    }
    m_uploadDialog->setMinimumWidth(2*this->width()/3);

    connect(m_uploadDialog, &FileUploaderDialog::finished, this, &AssetsWidget::fileUploaderFinished);

    m_uploadDialog->setModal(true);
    m_uploadDialog->show();


}*/

void TestsWidget::on_btnDelete_clicked()
{
    // Check if the sender is a QToolButton (from the action column)
    QToolButton* action_btn = dynamic_cast<QToolButton*>(sender());
    if (action_btn && action_btn != ui->btnDelete){
        // Select row according to the session id of that button
        QString test_uuid = action_btn->property("test_uuid").toString();
        QTableWidgetItem* test_item = m_tableTests[test_uuid];
        ui->tableTests->clearSelection();
        if (test_item)
            test_item->setSelected(true); // Select row?
    }

    if (ui->tableTests->selectedItems().count()==0)
        return;

    GlobalMessageBox diag;
    QMessageBox::StandardButton answer = QMessageBox::No;
    if (ui->tableTests->selectionModel()->selectedRows().count() == 1){
        QTableWidgetItem* base_item = ui->tableTests->item(ui->tableTests->currentRow(), TEST_NAME_COLUMN);
        answer = diag.showYesNo(tr("Suppression?"), tr("Êtes-vous sûrs de vouloir supprimer """) + base_item->text() + """?");
    }else{
        answer = diag.showYesNo(tr("Suppression?"), tr("Êtes-vous sûrs de vouloir supprimer toutes les évaluations sélectionnées?"));
    }

    if (answer == QMessageBox::Yes){
        // We must delete!
        foreach(QModelIndex selected_row, ui->tableTests->selectionModel()->selectedRows()){
            QTableWidgetItem* base_item = ui->tableTests->item(selected_row.row(), TEST_NAME_COLUMN);
            QString test_uuid = m_tableTests.key(base_item);
            int test_id = m_tests[test_uuid]->getId();
            m_comManager->doDelete(WEB_TESTINFO_PATH, test_id);
        }
    }
}


void TestsWidget::on_tableTests_itemSelectionChanged()
{
    ui->btnDelete->setEnabled(!ui->tableTests->selectedItems().isEmpty());

    // Display / hide summary
    ui->tableTestSummary->setVisible(ui->tableTests->selectionModel()->selectedRows().count() == 1);

    if (ui->tableTestSummary->isVisible()){
        QString test_uuid = m_tableTests.key(ui->tableTests->item(ui->tableTests->currentRow(), TEST_NAME_COLUMN));
        showTestSummary(test_uuid);
    }
}

void TestsWidget::on_btnDownloadAll_clicked()
{
    if (m_tests.isEmpty()) return; // Should not happen

    GlobalMessageBox msg(this);

    // Query for file to save
    QString full_path;
    QString filename = ""; // Generate default filename
    if (m_tests.first()->hasFieldName("test_participant")){
        QString clean_name = Utils::removeNonAlphanumerics(m_tests.first()->getFieldValue("test_participant").toString());
        filename += clean_name;
        if (!filename.endsWith("_"))
            filename += "_";
    }
    if (m_viewMode == VIEWMODE_SESSION){
        if (m_tests.first()->hasFieldName("test_session_name")){
            QString clean_name = Utils::removeNonAlphanumerics(m_tests.first()->getFieldValue("test_session_name").toString());
            filename += clean_name;
            if (!filename.endsWith("_"))
                filename += "_";
        }
    }
    filename += "tests.csv";

    m_fileDialog.selectFile(filename);
    m_fileDialog.setFileMode(QFileDialog::AnyFile);
    m_fileDialog.setWindowTitle(tr("Fichier à enregistrer"));
    m_fileDialog.setAcceptMode(QFileDialog::AcceptSave);

    if (m_fileDialog.exec()){
        QStringList selected_files = m_fileDialog.selectedFiles();
        if (selected_files.isEmpty())
            return;
        full_path = selected_files.first();
    }else{
        return;
    }

    // Get all variables in the json fields
    QStringList columns;
    QStringList variables;
    for (TeraData* test:qAsConst(m_tests)){
        // Parse json result
        if (test->hasFieldName("test_summary")){
            QJsonDocument test_summary_doc = QJsonDocument::fromJson(test->getFieldValue("test_summary").toString().toUtf8());
            if (!test_summary_doc.isNull()){
                QVariantMap test_summary = test_summary_doc.object().toVariantMap();
                QVariantMap::const_iterator test_result = test_summary.constBegin();
                while (test_result != test_summary.constEnd()) {
                    if (!variables.contains(test_result.key()))
                        variables << test_result.key();
                    ++test_result;
                }
            }
        }

    }

    columns << "Participant" << "User" << "Device" << "Service" << "Session" << "TestType" << "Status" << "Name" << "Date" << "Time"; // Basic columns

    // Format and save to csv
    QFile csv_file(full_path);

    if (csv_file.open(QIODevice::WriteOnly| QIODevice::Text)){
        QTextStream csv_writer(&csv_file);

        for (const QString &col: qAsConst(columns))
            csv_writer << col << '\t';

        for (const QString &var: qAsConst(variables))
            csv_writer << var << '\t';
        csv_writer << Qt::endl;

        for (int i=0; i<ui->tableTests->rowCount(); i++){
            // Use the data in the table to keep received order of items
            TeraData* test = m_tests[m_tableTests.key(ui->tableTests->item(i,0))];

            // Creator
            if (test->hasFieldName("test_participant")){
                csv_writer << test->getFieldValue("test_participant").toString();
            }
            csv_writer << '\t';
            if (test->hasFieldName("test_user")){
                csv_writer << test->getFieldValue("test_user").toString();
            }
            csv_writer << '\t';
            if (test->hasFieldName("test_device")){
                csv_writer << test->getFieldValue("test_device").toString();
            }
            csv_writer << '\t';
            if (test->hasFieldName("test_service")){
                csv_writer << test->getFieldValue("test_service").toString();
            }
            csv_writer << '\t';

            // Test infos
            if (test->hasFieldName("test_session_name")){
                csv_writer << test->getFieldValue("test_session_name").toString();
            }
            csv_writer << '\t';
            if (test->hasFieldName("id_test_type")){
                csv_writer << test->getFieldValue("id_test_type").toString();
            }
            csv_writer << '\t';
            if (test->hasFieldName("test_status")){
                csv_writer << test->getFieldValue("test_status").toString();
            }
            csv_writer << '\t';
            if (test->hasFieldName("test_name")){
                csv_writer << test->getFieldValue("test_name").toString();
            }
            csv_writer << '\t';
            if (test->hasFieldName("test_datetime")){
                csv_writer << test->getFieldValue("test_datetime").toDateTime().toString("yyyy-MM-dd");
                csv_writer << '\t';
                csv_writer << test->getFieldValue("test_datetime").toDateTime().toString("hh:mm:ss");
            }
            csv_writer << '\t';
            if (test->hasFieldName("test_summary")){
                QJsonDocument test_summary_doc = QJsonDocument::fromJson(test->getFieldValue("test_summary").toString().toUtf8());
                if (!test_summary_doc.isNull()){
                   QVariantMap test_summary = test_summary_doc.object().toVariantMap();
                   for (const QString &var: qAsConst(variables)){
                       if (test_summary.contains(var)){
                           csv_writer <<test_summary[var].toString();
                       }
                       csv_writer << '\t';
                   }
                }
            }
            csv_writer << Qt::endl;

        }
        csv_file.close();
    }else{
        msg.showError(tr("Erreur d'exportation"), tr("Impossible d'exporter les données dans le fichier.") + "\n\r" + tr("Le fichier est peut-être ouvert dans une autre application?"));
        return;
    }

    msg.showInfo(tr("Exportation terminée"), tr("L'exportation des données a été complétée avec succès."));


}

