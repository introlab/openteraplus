#include "TestInvitationDialog.h"
#include "services/EmailService/EmailServiceWebAPI.h"
#include "ui_TestInvitationDialog.h"

#include <QStyledItemDelegate>

#include "WebAPI.h"

TestInvitationDialog::TestInvitationDialog(ComManager *comMan, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TestInvitationDialog)
    , m_comManager(comMan)
{
    ui->setupUi(this);
    setModal(true);

    initUI();
    connect(m_comManager, &ComManager::testInvitationsReceived, this, &TestInvitationDialog::processTestInvitationsReply);

    m_emailComManager = new EmailComManager(m_comManager);
    connect(m_emailComManager, &EmailComManager::postResultsOK, this, &TestInvitationDialog::emailSentSuccess);
    connect(m_emailComManager, &EmailComManager::networkError, this, &TestInvitationDialog::emailSentError);
}

TestInvitationDialog::~TestInvitationDialog()
{
    delete ui;
    m_emailComManager->deleteLater();
}

void TestInvitationDialog::setTestTypes(const QList<TeraData> &test_types)
{
    ui->cmbTestType->clear();

    for (TeraData tt: test_types){
        ui->cmbTestType->addItem(tt.getName(), tt.getId());
    }
}

void TestInvitationDialog::setCurrentSession(const TeraData *session)
{
    ui->lblSessionName->setText(session->getName());
    m_currentSessionId = session->getId();
    ui->lblSession->show();
    ui->lblSessionName->show();
}

void TestInvitationDialog::setCurrentData(TeraData *data)
{
    m_data = data;
    ui->lblCount->setVisible(m_data);
    ui->numCount->setVisible(m_data);
    ui->cmbTestType->setEnabled(!m_data);
    // ui->btnNext->setVisible(!m_data);
    // ui->btnPrevious->setVisible(!m_data);
    ui->stackedPages->removeWidget(ui->pageTargets);
    ui->chkInviteEmail->setText(m_data ? tr("Renvoyer l'invitation par courriel") : tr("Envoyer les invitations automatiquement par courriel"));
    ui->btnOK->setText(m_data ? tr("OK") : tr("Inviter"));
    /*ui->btnOK->setVisible(m_data);*/

    if (m_data){
        int test_index = ui->cmbTestType->findData(m_data->getFieldValue("id_test_type").toInt());
        ui->cmbTestType->setCurrentIndex(test_index);
        ui->numUsage->setValue(m_data->getFieldValue("test_invitation_max_count").toInt());
        ui->numCount->setValue(m_data->getFieldValue("test_invitation_count").toInt());
    }

}

void TestInvitationDialog::setEnableEmail(const bool &enable_email)
{
    m_enableEmails = enable_email;

    if (!m_enableEmails){
        ui->chkInviteEmail->setCheckState(Qt::Unchecked);
        on_chkInviteEmail_checkStateChanged(Qt::Unchecked);
    }else{
        // Default value is on
        ui->chkInviteEmail->setCheckState(Qt::Checked);
        on_chkInviteEmail_checkStateChanged(Qt::Checked);
    }
    ui->chkInviteEmail->setVisible(m_enableEmails);
}

void TestInvitationDialog::setEnableInviteesList(const bool &enable)
{
    if (m_data)
        return; // Don't change anything if we already have some invitation data

    if (!enable)
        ui->stackedPages->removeWidget(ui->pageTargets);
    else
        ui->stackedPages->addWidget(ui->pageTargets);
}

void TestInvitationDialog::setCurrentProjectId(const int &id_project)
{
    m_currentProjectId = id_project;
}

void TestInvitationDialog::setInvitableDevices(QHash<int, TeraData> *devices)
{
    if (devices)
        ui->widgetInvitees->setAvailableDevices(devices->values(), true);
    else
        ui->widgetInvitees->setAvailableDevices(QList<TeraData>());
}

void TestInvitationDialog::setInvitableParticipants(QHash<int, TeraData> *participants)
{
    if (participants)
        ui->widgetInvitees->setAvailableParticipants(participants->values(), true);
    else
        ui->widgetInvitees->setAvailableParticipants(QList<TeraData>());
}

void TestInvitationDialog::setInvitableUsers(QHash<int, TeraData> *users)
{
    if (users)
        ui->widgetInvitees->setAvailableUsers(users->values(), true);
    else
        ui->widgetInvitees->setAvailableUsers(QList<TeraData>());
}

void TestInvitationDialog::addDevicesToInvitation(const QStringList &device_uuids)
{
    for(const QString& uuid: device_uuids){
        ui->widgetInvitees->addDeviceToSession(uuid);
    }
}

void TestInvitationDialog::addParticipantsToInvitation(const QStringList &participant_uuids)
{
    for(const QString& uuid: participant_uuids){
        ui->widgetInvitees->addParticipantToSession(uuid);
    }
}

void TestInvitationDialog::addUsersToInvitation(const QStringList &user_uuids)
{
    for(const QString& uuid: user_uuids){
        ui->widgetInvitees->addUserToSession(uuid);
    }
}

void TestInvitationDialog::processTestInvitationsReply(QList<TeraData> invitations)
{
    if (ui->chkInviteEmail->isChecked()){
        // Create email queue
        m_pendingInvitations = invitations;

        // Start sending!
        ui->progressEmails->setMaximum(m_pendingInvitations.count());
        ui->progressEmails->setValue(0);
        ui->btnOK->hide();
        ui->frameMessage->hide();
        ui->frameEmails->show();
        processNextEmail();
    }
}

void TestInvitationDialog::emailSentSuccess()
{
    ui->txtEmailLogs->insertHtml("<font color='green'>" + tr("OK") + "</font><br>");
    ui->progressEmails->setValue(ui->progressEmails->value()+1);
    processNextEmail();
}

void TestInvitationDialog::emailSentError(QNetworkReply::NetworkError error, QString error_str, QNetworkAccessManager::Operation op, int status_code, QString path, QUrlQuery url_query)
{
    ui->txtEmailLogs->insertHtml("<font color='red'>" + tr("Erreur") + ": " + error_str + "</font><br>");
    ui->progressEmails->setValue(ui->progressEmails->value()+1);
    processNextEmail();
}

void TestInvitationDialog::initUI()
{
    ui->cmbTestType->setItemDelegate(new QStyledItemDelegate(ui->cmbTestType));

    ui->stackedPages->setCurrentIndex(0);
    ui->lblCount->hide();
    ui->numCount->hide();
    ui->lblSession->hide();
    ui->lblSessionName->hide();
    ui->btnPrevious->setEnabled(false);
    ui->btnOK->setVisible(false);
    ui->btnDone->setVisible(false);
    ui->txtMessage->setEmailTemplate(tr("Bonjour") + " $participant,<br><br>" + tr("Vous êtes invités à compléter un questionnaire.") +
                                     "<br>" + tr("Veuillez") + " $join_link " + tr("pour compléter le questionnaire.") + "<br><br>" +
                                     tr("Merci de votre participation") + "<br><br>$fullname");
    ui->txtMessage->setVariable("$fullname", m_comManager->getCurrentUser().getName());
    setEnableEmail(m_enableEmails);
    ui->frameEmails->hide();

    ui->dateExpiration->setDate(QDate::currentDate().addDays(30));

    ui->widgetInvitees->setCountLimited(false);
    ui->widgetInvitees->showAvailableInvitees(true);
    ui->widgetInvitees->showOnlineFilter(false);
    ui->widgetInvitees->selectFilterParticipant();

    connect(ui->widgetInvitees, &SessionInviteWidget::inviteesCountChanged, this, &TestInvitationDialog::inviteesCountChanged);

}

void TestInvitationDialog::processNextEmail()
{
    if (m_pendingInvitations.isEmpty()){
        // All done sending!
        ui->btnCancel->hide();
        ui->btnNext->hide();
        ui->btnPrevious->hide();
        ui->btnDone->setVisible(true);
        setEnabled(true);
        return;
    }

    QJsonDocument doc;
    QJsonObject data_obj;
    QString participant_name;
    TeraData* invitation = &m_pendingInvitations.first();
    if (invitation->hasFieldName("test_invitation_participant")){
        QJsonObject obj = invitation->getFieldValue("test_invitation_participant").toJsonObject();
        data_obj["participant_uuid"] = obj["participant_uuid"].toString();
        participant_name = obj["participant_name"].toString();
        ui->txtMessage->setVariable("$participant", participant_name);
    }
    ui->txtMessage->setVariable("$join_link",  "<a href=\"" + invitation->getFieldValue("test_invitation_url").toString() + "\">" + tr("cliquez ici") + "</a>");

    data_obj["body"] = ui->txtMessage->getPreview()->toHtml();
    data_obj["subject"] = ui->txtSubject->text();
    doc.setObject(data_obj);
    ui->txtEmailLogs->insertHtml(tr("Envoi de l'invitation à") + " " + participant_name + "... ");
    m_emailComManager->doPost(EMAIL_SEND_PATH, doc.toJson());

    m_pendingInvitations.removeFirst();
}

void TestInvitationDialog::on_btnCancel_clicked()
{
    reject();
}

void TestInvitationDialog::on_stackedPages_currentChanged(int current_index)
{
    ui->btnPrevious->setEnabled(current_index>0);
    ui->btnNext->setEnabled(current_index < ui->stackedPages->count() - 1);
    ui->btnOK->setVisible(current_index == ui->stackedPages->count() - 1);
    if (ui->btnNext->isEnabled() && ui->stackedPages->currentWidget() == ui->pageTargets){
        ui->btnNext->setEnabled(ui->widgetInvitees->getInviteesCount() > 0);
    }
    if (ui->btnOK->isVisible() && ui->stackedPages->currentWidget() == ui->pageTargets){
        ui->btnOK->setEnabled(ui->widgetInvitees->getInviteesCount() > 0);
    }
}


void TestInvitationDialog::on_btnPrevious_clicked()
{
    if (ui->stackedPages->currentIndex() > 0)
        ui->stackedPages->setCurrentIndex(ui->stackedPages->currentIndex()-1);
}


void TestInvitationDialog::on_btnNext_clicked()
{
    if (ui->stackedPages->currentIndex() < ui->stackedPages->count() - 1)
        ui->stackedPages->setCurrentIndex(ui->stackedPages->currentIndex() + 1);
}


void TestInvitationDialog::on_btnOK_clicked()
{
    // Convert and send invitations
    QJsonDocument doc;
    QJsonObject base_obj;
    QJsonArray invitations;

    if (!m_data){
        // New
        QList<int> ids = ui->widgetInvitees->getParticipantsIdsInSession();
        for(int id: ids){
            QJsonObject invitation;
            invitation["id_test_invitation"] = 0;
            invitation["id_participant"] = id;
            invitation["id_project"] = m_currentProjectId;
            if (m_currentSessionId)
                invitation["id_session"] = m_currentSessionId;
            invitation["id_test_type"] = ui->cmbTestType->currentData().toInt();
            invitation["test_invitation_max_count"] = ui->numUsage->value();
            invitation["test_invitation_expiration_date"] = QJsonValue::fromVariant(ui->dateExpiration->date());
            invitations.append(invitation);
        }

        ids = ui->widgetInvitees->getDevicesIdsInSession();
        for(int id: ids){
            QJsonObject invitation;
            invitation["id_test_invitation"] = 0;
            invitation["id_device"] = id;
            invitation["id_project"] = m_currentProjectId;
            if (m_currentSessionId)
                invitation["id_session"] = m_currentSessionId;
            invitation["id_test_type"] = ui->cmbTestType->currentData().toInt();
            invitation["test_invitation_max_count"] = ui->numUsage->value();
            invitation["test_invitation_expiration_date"] = QJsonValue::fromVariant(ui->dateExpiration->date());
            invitations.append(invitation);
        }

        ids = ui->widgetInvitees->getUsersIdsInSession();
        for(int id: ids){
            QJsonObject invitation;
            invitation["id_test_invitation"] = 0;
            invitation["id_user"] = id;
            invitation["id_project"] = m_currentProjectId;
            if (m_currentSessionId)
                invitation["id_session"] = m_currentSessionId;
            invitation["id_test_type"] = ui->cmbTestType->currentData().toInt();
            invitation["test_invitation_max_count"] = ui->numUsage->value();
            invitation["test_invitation_expiration_date"] = QJsonValue::fromVariant(ui->dateExpiration->date());
            invitations.append(invitation);
        }
    }else{
        // Update
        QJsonObject invitation;
        invitation["id_test_invitation"] = m_data->getId();
        invitation["test_invitation_max_count"] = ui->numUsage->value();
        invitation["test_invitation_count"] = ui->numCount->value();
        invitation["test_invitation_expiration_date"] = QJsonValue::fromVariant(ui->dateExpiration->date());
        invitations.append(invitation);

    }
    base_obj["tests_invitations"] = invitations;
    doc.setObject(base_obj);

    m_comManager->doPost(WEB_TESTINVITATION_PATH, doc.toJson());

    setEnabled(false);
    if (!ui->chkInviteEmail->isChecked()){
        disconnect(m_comManager);
        accept();
    }
}


void TestInvitationDialog::on_btnDone_clicked()
{
    accept();
}


void TestInvitationDialog::on_chkInviteEmail_checkStateChanged(const Qt::CheckState &state)
{
    if (!ui->chkInviteEmail->isChecked()){
        ui->stackedPages->removeWidget(ui->pageMessage);
    }else{
        bool has_widget = false;
        for (int i=0; i<ui->stackedPages->count(); i++){
            if (ui->stackedPages->widget(i) == ui->pageMessage){
                has_widget = true;
                break;
            }
        }
        if (!has_widget)
            ui->stackedPages->addWidget(ui->pageMessage);
    }
    on_stackedPages_currentChanged(ui->stackedPages->currentIndex());
}

void TestInvitationDialog::inviteesCountChanged(int count)
{
    if (ui->stackedPages->currentWidget() == ui->pageTargets && ui->stackedPages->currentIndex() < ui->stackedPages->count() - 1)
        ui->btnNext->setEnabled(count > 0);

    if (ui->stackedPages->currentIndex() == ui->stackedPages->count() - 1)
        ui->btnOK->setEnabled(count > 0);
}

