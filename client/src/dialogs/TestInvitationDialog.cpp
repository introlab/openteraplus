#include "TestInvitationDialog.h"
#include "ui_TestInvitationDialog.h"
#include "WebAPI.h"

TestInvitationDialog::TestInvitationDialog(ComManager *comMan, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TestInvitationDialog)
    , m_comManager(comMan)
{
    ui->setupUi(this);
    setModal(true);

    initUI();
}

TestInvitationDialog::~TestInvitationDialog()
{
    delete ui;
}

void TestInvitationDialog::setTestTypes(const QList<TeraData> &test_types)
{
    ui->cmbTestType->clear();

    for (TeraData tt: test_types){
        ui->cmbTestType->addItem(tt.getName(), tt.getId());
    }
}

void TestInvitationDialog::setCurrentData(TeraData *data)
{
    m_data = data;
    ui->lblCount->setVisible(m_data);
    ui->numCount->setVisible(m_data);
    ui->cmbTestType->setEnabled(!m_data);
    ui->btnNext->setVisible(!m_data);
    ui->btnPrevious->setVisible(!m_data);
    ui->btnOK->setText(m_data ? tr("OK") : tr("Inviter"));
    ui->btnOK->setVisible(m_data);

    if (m_data){
        int test_index = ui->cmbTestType->findData(m_data->getFieldValue("id_test_type").toInt());
        ui->cmbTestType->setCurrentIndex(test_index);
        ui->numUsage->setValue(m_data->getFieldValue("test_invitation_max_count").toInt());
        ui->numCount->setValue(m_data->getFieldValue("test_invitation_count").toInt());
    }

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

void TestInvitationDialog::initUI()
{
    ui->stackedPages->setCurrentIndex(0);
    ui->lblCount->hide();
    ui->numCount->hide();
    ui->btnPrevious->setEnabled(false);
    ui->btnOK->setVisible(false);

    ui->dateExpiration->setDate(QDate::currentDate().addDays(30));

    ui->widgetInvitees->setCountLimited(false);
    ui->widgetInvitees->showAvailableInvitees(true);
    ui->widgetInvitees->showOnlineFilter(false);
    ui->widgetInvitees->selectFilterParticipant();

}

void TestInvitationDialog::on_btnCancel_clicked()
{
    reject();
}

void TestInvitationDialog::on_stackedPages_currentChanged(int current_index)
{
    int last_page_offset = 0;
    if (!ui->chkInviteEmail->isChecked())
        last_page_offset = 1;

    ui->btnPrevious->setEnabled(current_index>0);
    ui->btnNext->setEnabled(current_index < ui->stackedPages->count() - 1 - last_page_offset);
    ui->btnOK->setVisible(current_index == ui->stackedPages->count() - 1 - last_page_offset);
}


void TestInvitationDialog::on_btnPrevious_clicked()
{
    if (ui->stackedPages->currentIndex() > 0)
        ui->stackedPages->setCurrentIndex(ui->stackedPages->currentIndex()-1);
}


void TestInvitationDialog::on_btnNext_clicked()
{
    int last_page_offset = 0;
    if (!ui->chkInviteEmail->isChecked())
        last_page_offset = 1;
    if (ui->stackedPages->currentIndex() < ui->stackedPages->count() -1 - last_page_offset)
        ui->stackedPages->setCurrentIndex(ui->stackedPages->currentIndex()+1);
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

    accept();
}

