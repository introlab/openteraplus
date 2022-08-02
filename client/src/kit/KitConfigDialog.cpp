#include "KitConfigDialog.h"
#include "ui_KitConfigDialog.h"

KitConfigDialog::KitConfigDialog(ComManager *comMan, KitConfigManager *kitConfig, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KitConfigDialog)
{
    ui->setupUi(this);
    m_comManager = comMan;
    m_kitConfig = kitConfig;
    m_loading = true;
    m_valueJustChanged = false;

    initUi();

    connectSignals();

    // Query data
    querySites();
    queryDeviceConfigForm();
}

KitConfigDialog::~KitConfigDialog()
{
    delete ui;
}

void KitConfigDialog::on_btnClose_clicked()
{
    accept();
}

void KitConfigDialog::comManagerWaitingForReply(bool waiting)
{
    setEnabled(!waiting);
}

void KitConfigDialog::comManagerNetworkError(QNetworkReply::NetworkError error, QString error_msg, QNetworkAccessManager::Operation op, int status_code)
{
    Q_UNUSED(op)

    if (error_msg.endsWith('\n'))
        error_msg = error_msg.left(error_msg.length()-1);

    if (status_code > 0)
        setStatusMessage(tr("Erreur HTTP ") + QString::number(status_code) + ": " + error_msg, true);
    else
        setStatusMessage(tr("Erreur ") + QString::number(error) + ": " + error_msg, true);
}

void KitConfigDialog::processSitesReply(QList<TeraData> sites)
{
    setStatusMessage("");

    // Clear combo box
    ui->cmbSites->clear();

    // Fill combo box
    foreach(TeraData site, sites){
        ui->cmbSites->addItem(site.getName(), site.getId());
    }

    ui->cmbSites->setEnabled(true);
    if (ui->cmbSites->count() > 1){
        // More than one site - request user input
        ui->cmbSites->setCurrentIndex(-1);
    }else{
        // Only one site - select first one
        m_loading = false;
        ui->cmbSites->setCurrentIndex(0);
        on_cmbSites_currentIndexChanged(0);
    }

    m_loading = false;


}

void KitConfigDialog::processProjectsReply(QList<TeraData> projects)
{
    setStatusMessage("");

    // Clear combo box
    ui->cmbProjects->clear();

    // Fill combo box
    foreach(TeraData project, projects){
        ui->cmbProjects->addItem(project.getName(), project.getId());
    }

    ui->cmbProjects->setEnabled(true);
    if (ui->cmbProjects->count() > 1){
        // More than one project - request user input
        ui->cmbProjects->setCurrentIndex(-1);
    }else{
        // Only one project - select first one
        m_loading = false;
        ui->cmbProjects->setCurrentIndex(0);
        on_cmbProjects_currentIndexChanged(0);
    }
    m_loading = false;

}

void KitConfigDialog::processGroupsReply(QList<TeraData> groups)
{
    setStatusMessage("");

    // Clear combo box
    ui->cmbGroups->clear();
    ui->cmbGroups->addItem("(" + tr("Tous") + ")", -1); // Empty group

    // Fill combo box
    foreach(TeraData group, groups){
        ui->cmbGroups->addItem(group.getName(), group.getId());
    }

    ui->cmbGroups->setEnabled(true);
    m_loading = false;

    ui->cmbGroups->setCurrentIndex(0);
    on_cmbGroups_currentIndexChanged(0);

}

void KitConfigDialog::processParticipantsReply(QList<TeraData> participants, QUrlQuery reply_query)
{
    setStatusMessage("");

    if (!reply_query.hasQueryItem(WEB_QUERY_ID_PARTICIPANT)){
        // We didn't query a specific participant

        // Clear all previous participants
        ui->lstParticipants->clear();

        // Fill participants list
        foreach(TeraData participant, participants){
            QListWidgetItem* item = new QListWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_PARTICIPANT)),
                                                        participant.getName(),ui->lstParticipants);
            item->setData(Qt::UserRole, participant.getId());
        }

        ui->lstParticipants->setEnabled(true);
    }else{
        // We queried a specific participant, to associate it to that kit
        if (participants.count() == 1){
            if (participants.first().hasFieldName("participant_token")){
                QString participant_token = participants.first().getFieldValue("participant_token").toString();
                m_kitConfig->setParticipantToken(participant_token);
                QString service_url = TeraData::getServiceParticipantUrl(m_services[ui->cmbServices->currentData().toInt()],
                        m_comManager->getServerUrl(), participant_token);
                m_kitConfig->setParticipantServiceUrl(service_url);
                QString service_key = m_services[ui->cmbServices->currentData().toInt()].getFieldValue("service_key").toString();
                m_kitConfig->setServiceKey(service_key);
                m_kitConfig->saveConfig();
                ui->btnSetParticipant->setEnabled(false);
                GlobalMessageBox msg(this);
                msg.showInfo(tr("Participant associé"), tr("Le participant") + " " + participants.first().getName() + " " + tr("a été associé à ce kit") + ".");
            }
        }

    }
    m_loading = false;

}

void KitConfigDialog::processServicesReply(QList<TeraData> services)
{
    setStatusMessage("");

    // Clear combo box
    ui->cmbServices->clear();
    m_services.clear();

    int default_index = -1;
    // Fill combo box
    foreach(TeraData service, services){
        ui->cmbServices->addItem(service.getName(), service.getId());
        m_services.insert(service.getId(), service);

        // Set videorehab service as default
        if (service.getFieldValue("service_key") == "VideoRehabService"){
            default_index = ui->cmbServices->count() - 1;
        }
    }

    if (default_index>=0)
        ui->cmbServices->setCurrentIndex(default_index);

    ui->cmbServices->setEnabled(true);
    m_loading = false;
}

void KitConfigDialog::processFormsReply(QString form_type, QString data)
{
    if (form_type.contains(WEB_FORMS_QUERY_SERVICE_CONFIG)){
       ui->wdgDeviceConfig->buildUiFromStructure(data);
       ui->wdgDeviceConfig->fillFormFromData(m_kitConfig->getKitConfig());
       ui->wdgDeviceConfig->setEnabled(true);
    }
}

void KitConfigDialog::configFormDirty(bool dirty)
{
    Q_UNUSED(dirty)
    updateSaveButtonState();
}

void KitConfigDialog::configFormValueChanged(QWidget *wdg, QVariant value)
{

    if (m_valueJustChanged){
        m_valueJustChanged = false;
        return;
    }

    // OpenTeraCam camera source
    if (wdg == ui->wdgDeviceConfig->getWidgetForField("teracam_src")){
        QLineEdit* wdg_editor = dynamic_cast<QLineEdit*>(ui->wdgDeviceConfig->getWidgetForField("teracam_src"));
        VideoRehabVirtualCamSetupDialog dlg(ui->wdgDeviceConfig->getFieldValue("teracam_src").toString());
        dlg.setCursorPosition(wdg_editor->cursorPosition());
        m_valueJustChanged = true;
        if (dlg.exec() == QDialog::Accepted){
            ui->wdgDeviceConfig->setFieldValue("teracam_src", dlg.getCurrentSource());
        }else{
            wdg_editor->undo();
        }
    }

    // PTZ changes
    if (wdg == ui->wdgDeviceConfig->getWidgetForField(("camera_ptz"))){
        if (value.toBool())
            showPTZDialog();
    }
}

void KitConfigDialog::initUi()
{
    ui->lblStatus->hide();
    ui->frameTechSup->hide();
    ui->frameOtherSoftware->hide();

    ui->cmbSites->setItemDelegate(new QStyledItemDelegate(ui->cmbSites));
    ui->cmbProjects->setItemDelegate(new QStyledItemDelegate(ui->cmbProjects));
    ui->cmbGroups->setItemDelegate(new QStyledItemDelegate(ui->cmbGroups));
    ui->cmbServices->setItemDelegate(new QStyledItemDelegate(ui->cmbServices));

    ui->btnUnsetParticipant->setEnabled(!m_kitConfig->getParticipantToken().isEmpty());
    ui->chkTechSup->setChecked(!m_kitConfig->getTechSupportClient().isEmpty());
    ui->chkTechSup->setProperty("initial_value", ui->chkTechSup->isChecked());
    ui->txtTechSup->setText(m_kitConfig->getTechSupportClient());
    ui->txtTechSup->setProperty("initial_value", ui->txtTechSup->text());

    QString other_software = m_kitConfig->getOtherSoftwarePath();
    ui->chkOtherSoftware->setChecked(!other_software.isEmpty());
    ui->chkOtherSoftware->setProperty("initial_value", ui->chkOtherSoftware->isChecked());
    ui->txtOtherSoftware->setText(other_software);
    ui->txtOtherSoftware->setProperty("initial_value", other_software);

    ui->tabSections->setCurrentIndex(0);
}

void KitConfigDialog::connectSignals()
{
    connect(m_comManager, &ComManager::waitingForReply,             this, &KitConfigDialog::comManagerWaitingForReply);
    connect(m_comManager, &ComManager::networkError,                this, &KitConfigDialog::comManagerNetworkError);

    connect(m_comManager, &ComManager::sitesReceived,               this, &KitConfigDialog::processSitesReply);
    connect(m_comManager, &ComManager::projectsReceived,            this, &KitConfigDialog::processProjectsReply);
    connect(m_comManager, &ComManager::groupsReceived,              this, &KitConfigDialog::processGroupsReply);
    connect(m_comManager, &ComManager::participantsReceived,        this, &KitConfigDialog::processParticipantsReply);
    connect(m_comManager, &ComManager::servicesReceived,            this, &KitConfigDialog::processServicesReply);
    connect(m_comManager, &ComManager::formReceived,                this, &KitConfigDialog::processFormsReply);

    connect(ui->wdgDeviceConfig, &TeraForm::formIsNowDirty,         this, &KitConfigDialog::configFormDirty);
    connect(ui->wdgDeviceConfig, &TeraForm::widgetValueHasChanged,  this, &KitConfigDialog::configFormValueChanged);

}

void KitConfigDialog::updateSaveButtonState()
{
    ui->btnSaveConfig->setEnabled(ui->wdgDeviceConfig->isDirty() ||
                                  ui->chkTechSup->isChecked() != ui->chkTechSup->property("initial_value").toBool() ||
                                  ui->txtTechSup->text() != ui->txtTechSup->property("initial_value").toString() ||
                                  ui->chkOtherSoftware->isChecked() != ui->chkOtherSoftware->property("initial_value").toBool() ||
                                  ui->txtOtherSoftware->text() != ui->txtOtherSoftware->property("initial_value").toString());
}

void KitConfigDialog::setStatusMessage(QString msg, bool error)
{
    ui->lblStatus->setText(msg);

    if (!error){
        ui->lblStatus->setStyleSheet("background-color: rgba(64,227,227,50%);");
    }else {
        ui->lblStatus->setStyleSheet("background-color: rgba(255, 97, 97,50%);");
    }

    ui->lblStatus->setVisible(!msg.isEmpty());
}

void KitConfigDialog::showPTZDialog()
{
    ui->wdgDeviceConfig->setFieldsEnabled(QStringList() << "camera_ptz_type" << "camera_ptz_ip" << "camera_ptz_port" << "camera_ptz_username" << "camera_ptz_password", false);

    VideoRehabPTZDialog dlg;
    dlg.setCurrentValues(ui->wdgDeviceConfig->getFieldValue("camera_ptz_type").toInt(),
                         ui->wdgDeviceConfig->getFieldValue("camera_ptz_ip").toString(),
                         ui->wdgDeviceConfig->getFieldValue("camera_ptz_port").toInt(),
                         ui->wdgDeviceConfig->getFieldValue("camera_ptz_username").toString(),
                         ui->wdgDeviceConfig->getFieldValue("camera_ptz_password").toString()
                         );
    //dlg.setCursorPosition(wdg_editor->cursorPosition());
    if (dlg.exec() == QDialog::Accepted){
        ui->wdgDeviceConfig->setFieldValue("camera_ptz_type", dlg.getCurrentSrcIndex());
        ui->wdgDeviceConfig->setFieldValue("camera_ptz_ip", dlg.getCurrentUrl());
        ui->wdgDeviceConfig->setFieldValue("camera_ptz_port", dlg.getCurrentPort());
        ui->wdgDeviceConfig->setFieldValue("camera_ptz_username", dlg.getCurrentUsername());
        ui->wdgDeviceConfig->setFieldValue("camera_ptz_password", dlg.getCurrentPassword());
    }else{
        ui->wdgDeviceConfig->setFieldValue("camera_ptz", false);
    }
}

void KitConfigDialog::querySites()
{
    m_loading = true;
    setStatusMessage(tr("Chargement des sites en cours..."));

    m_comManager->doGet(WEB_SITEINFO_PATH, QUrlQuery(WEB_QUERY_LIST));
}

void KitConfigDialog::queryProjects(int id_site)
{
    m_loading = true;
    setStatusMessage(tr("Chargement des projets en cours..."));

    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_SITE, QString::number(id_site));
    query.addQueryItem(WEB_QUERY_LIST, "true");
    m_comManager->doGet(WEB_PROJECTINFO_PATH, query);
}

void KitConfigDialog::queryGroups(int id_project)
{
    m_loading = true;
    setStatusMessage(tr("Chargement des groupes en cours..."));

    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(id_project));
    query.addQueryItem(WEB_QUERY_LIST, "true");
    m_comManager->doGet(WEB_GROUPINFO_PATH, query);
}

void KitConfigDialog::queryServices(int id_project)
{
    m_loading = true;
    setStatusMessage(tr("Chargement des services en cours..."));

    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(id_project));
    query.addQueryItem(WEB_QUERY_LIST, "true");
    m_comManager->doGet(WEB_SERVICEINFO_PATH, query);
}

void KitConfigDialog::queryParticipantsForProject(int id_project)
{
    m_loading = true;
    setStatusMessage(tr("Chargement des participants en cours..."));

    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(id_project));
    query.addQueryItem(WEB_QUERY_LIST, "true");
    query.addQueryItem(WEB_QUERY_ENABLED, "true");
    m_comManager->doGet(WEB_PARTICIPANTINFO_PATH, query);
}

void KitConfigDialog::queryParticipantsForGroup(int id_group)
{
    m_loading = true;
    setStatusMessage(tr("Chargement des participants en cours..."));

    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_GROUP, QString::number(id_group));
    query.addQueryItem(WEB_QUERY_LIST, "true");
    query.addQueryItem(WEB_QUERY_ENABLED, "true");
    m_comManager->doGet(WEB_PARTICIPANTINFO_PATH, query);

}

void KitConfigDialog::queryParticipant(int id_participant)
{
    m_loading = true;
    setStatusMessage(tr("Chargement des informations du participant en cours..."));

    QUrlQuery query;
    query.addQueryItem(WEB_QUERY_ID_PARTICIPANT, QString::number(id_participant));
    m_comManager->doGet(WEB_PARTICIPANTINFO_PATH, query);
}

void KitConfigDialog::queryDeviceConfigForm()
{
    QUrlQuery query(WEB_FORMS_QUERY_SERVICE_CONFIG);
    query.addQueryItem(WEB_QUERY_KEY, "VideoRehabService");
    m_comManager->doGet(WEB_FORMS_PATH, query);
}

void KitConfigDialog::on_cmbSites_currentIndexChanged(int index)
{
    if (index<0 || m_loading)
        return;

    int id_site = ui->cmbSites->currentData().toInt();

    if (id_site>0){
        ui->cmbProjects->setEnabled(false);
        ui->cmbProjects->clear();
        ui->cmbGroups->setEnabled(false);
        ui->cmbGroups->clear();
        ui->lstParticipants->clear();
        ui->cmbServices->clear();
        ui->cmbServices->setEnabled(false);
        queryProjects(id_site);
    }
}

void KitConfigDialog::on_cmbProjects_currentIndexChanged(int index)
{
    if (index<0 || m_loading)
        return;

    int id_project = ui->cmbProjects->currentData().toInt();

    if (id_project>0){
        ui->cmbGroups->setEnabled(false);
        ui->cmbGroups->clear();
        ui->cmbServices->clear();
        ui->cmbServices->setEnabled(false);
        ui->lstParticipants->clear();
        queryGroups(id_project);
        queryServices(id_project);
    }
}

void KitConfigDialog::on_cmbGroups_currentIndexChanged(int index)
{
    if (index<0 || m_loading)
        return;

    int id_group = ui->cmbGroups->currentData().toInt();

    if (id_group>0){
        queryParticipantsForGroup(id_group);
    }else{
        int id_project = ui->cmbProjects->currentData().toInt();
        if (id_project > 0){
            queryParticipantsForProject(id_project);
        }
    }
}

void KitConfigDialog::on_lstParticipants_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)

    if (ui->cmbServices->currentIndex()>=0)
        ui->btnSetParticipant->setEnabled(current);
    else
        ui->btnSetParticipant->setEnabled(false);
}

void KitConfigDialog::on_btnSetParticipant_clicked()
{
    if (!ui->lstParticipants->currentItem())
        return;

    int id_participant = ui->lstParticipants->currentItem()->data(Qt::UserRole).toInt();

    queryParticipant(id_participant);
}

void KitConfigDialog::on_btnUnsetParticipant_clicked()
{
    m_kitConfig->setParticipantToken("");
    m_kitConfig->saveConfig();
    ui->btnUnsetParticipant->setEnabled(false);
    GlobalMessageBox msg(this);
    msg.showInfo(tr("Participant désassocié"), tr("Ce kit n'est maintenant plus associé à aucun participant"));
}

void KitConfigDialog::on_cmbServices_currentIndexChanged(int index)
{
    if (ui->lstParticipants->currentItem() && index>=0)
        ui->btnSetParticipant->setEnabled(true);
    else
        ui->btnSetParticipant->setEnabled(false);
}

void KitConfigDialog::on_btnSaveConfig_clicked()
{
    //QString config = ui->wdgDeviceConfig->getFormData(true);
    QJsonDocument config = ui->wdgDeviceConfig->getFormDataJson(true);
    m_kitConfig->setKitConfig(config["service_config_config"].toObject().toVariantHash());
    QString tech_sup_client;
    if (ui->chkTechSup->isChecked() && !ui->txtTechSup->text().isEmpty()){
        tech_sup_client = ui->txtTechSup->text();
    }
    m_kitConfig->setTechSupportClient(tech_sup_client);

    QString other_software;
    if (ui->chkOtherSoftware->isChecked() && !ui->txtOtherSoftware->text().isEmpty()){
        other_software = ui->txtOtherSoftware->text();
    }
    m_kitConfig->setOtherSoftwarePath(other_software);
    m_kitConfig->saveConfig();

    ui->chkTechSup->setProperty("initial_value", ui->chkTechSup->isChecked());
    ui->txtTechSup->setProperty("initial_value", ui->txtTechSup->text());

    ui->chkOtherSoftware->setProperty("initial_value", ui->chkOtherSoftware->isChecked());
    ui->txtOtherSoftware->setProperty("initial_value", ui->txtOtherSoftware->text());

    ui->btnSaveConfig->setEnabled(false);
    GlobalMessageBox msg;
    msg.showInfo(tr("Sauvegarde complétée"), tr("La configuration du kit a été sauvegardée") + ".");

}

void KitConfigDialog::on_chkTechSup_stateChanged(int arg1)
{
    Q_UNUSED(arg1)

    ui->frameTechSup->setVisible(ui->chkTechSup->isChecked());
    updateSaveButtonState();
}

void KitConfigDialog::on_btnTechSup_clicked()
{
    QStringList base_paths = QStandardPaths::standardLocations(QStandardPaths::RuntimeLocation);
    QString base_path = base_paths.first();
    QString filename = QFileDialog::getOpenFileName(this, tr("Sélectionnez le logiciel à lancer lors du support technique"), base_path);

    if (!filename.isEmpty()){
        ui->txtTechSup->setText(filename);
    }

}

void KitConfigDialog::on_txtTechSup_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)

    updateSaveButtonState();
}

void KitConfigDialog::on_chkOtherSoftware_stateChanged(int arg1)
{
    Q_UNUSED(arg1)

    ui->frameOtherSoftware->setVisible(ui->chkOtherSoftware->isChecked());
    updateSaveButtonState();
}


void KitConfigDialog::on_btnOtherSoftware_clicked()
{
    QStringList base_paths = QStandardPaths::standardLocations(QStandardPaths::RuntimeLocation);
    QString base_path = base_paths.first();
    QString filename = QFileDialog::getOpenFileName(this, tr("Sélectionnez le logiciel à lancer lors du démarrage"), base_path);

    if (!filename.isEmpty()){
        ui->txtOtherSoftware->setText(filename);
    }
}


void KitConfigDialog::on_txtOtherSoftware_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)

    updateSaveButtonState();
}

