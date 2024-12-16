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

    QList<int> ids = ui->widgetInvitees->getParticipantsIdsInSession();
    for(int id: ids){
        QJsonObject invitation;
        invitation["id_test_invitation"] = 0;
        invitation["id_participant"] = id;
        invitation["id_test_type"] = ui->cmbTestType->currentData().toInt();
        invitation["test_invitation_count"] = ui->numUsage->value();
        invitation["test_invitation_expiration_date"] = QJsonValue::fromVariant(ui->dateExpiration->date());
        invitations.append(invitation);
    }

    ids = ui->widgetInvitees->getDevicesIdsInSession();
    for(int id: ids){
        QJsonObject invitation;
        invitation["id_test_invitation"] = 0;
        invitation["id_device"] = id;
        invitation["id_test_type"] = ui->cmbTestType->currentData().toInt();
        invitation["test_invitation_count"] = ui->numUsage->value();
        invitation["test_invitation_expiration_date"] = QJsonValue::fromVariant(ui->dateExpiration->date());
        invitations.append(invitation);
    }

    ids = ui->widgetInvitees->getUsersIdsInSession();
    for(int id: ids){
        QJsonObject invitation;
        invitation["id_test_invitation"] = 0;
        invitation["id_user"] = id;
        invitation["id_test_type"] = ui->cmbTestType->currentData().toInt();
        invitation["test_invitation_count"] = ui->numUsage->value();
        invitation["test_invitation_expiration_date"] = QJsonValue::fromVariant(ui->dateExpiration->date());
        invitations.append(invitation);
    }
    base_obj["tests_invitations"] = invitations;
    doc.setObject(base_obj);

    m_comManager->doPost(WEB_TESTINVITATION_PATH, doc.toJson());

    accept();
}

