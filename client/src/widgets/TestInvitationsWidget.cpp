#include "TestInvitationsWidget.h"
#include "TableNumberWidgetItem.h"
#include "TableDateWidgetItem.h"
#include "ui_TestInvitationsWidget.h"

TestInvitationsWidget::TestInvitationsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TestInvitationsWidget)
{
    ui->setupUi(this);
}

TestInvitationsWidget::TestInvitationsWidget(ComManager *comMan, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TestInvitationsWidget)
{
    setComManager(comMan);
}

TestInvitationsWidget::~TestInvitationsWidget()
{
    delete ui;
}

void TestInvitationsWidget::setComManager(ComManager *comMan)
{
    m_comManager = comMan;

    connect(m_comManager, &ComManager::testInvitationsReceived, this, &TestInvitationsWidget::processTestInvitationsReply);

}

void TestInvitationsWidget::setCurrentTestTypes(const QList<TeraData> &test_types)
{
    m_testTypes = test_types;
}

void TestInvitationsWidget::loadForProject(const int &id_project)
{
    setViewMode(ViewMode::VIEWMODE_PROJECT);
    ui->tableInvitations->clearContents();
    m_listInvitations_items.clear();

    if (m_comManager){
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(id_project));
        args.addQueryItem(WEB_QUERY_WITH_URLS, "1");
        args.addQueryItem(WEB_QUERY_FULL, "1");
        m_comManager->doGet(WEB_TESTINVITATION_PATH, args);
    }
}

void TestInvitationsWidget::processTestInvitationsReply(QList<TeraData> invitations)
{
    for(int i=0; i<invitations.count(); i++){
        updateInvitation(&invitations[i]);
    }
}

void TestInvitationsWidget::onTestInvitationDialogFinished(int result)
{
    m_invitationDialog->deleteLater();
    m_invitationDialog = nullptr;
}

void TestInvitationsWidget::updateInvitation(const TeraData *invitation)
{
    int id_test_invitation = invitation->getId();

    QTableWidgetItem* user_item;
    QTableWidgetItem* participant_item;
    QTableWidgetItem* device_item;
    QTableWidgetItem* test_type_item;
    TableNumberWidgetItem* limit_item;
    TableNumberWidgetItem* count_item;
    QTableWidgetItem* key_item;
    TableDateWidgetItem* creation_item;
    TableDateWidgetItem* expiration_item;

    if (m_listInvitations_items.contains(id_test_invitation)){
        // Already there, get items
        user_item = m_listInvitations_items[id_test_invitation];
        int row = user_item->row();
        participant_item = ui->tableInvitations->item(row, COLUMN_PARTICIPANT);
        device_item = ui->tableInvitations->item(row, COLUMN_DEVICE);
        test_type_item = ui->tableInvitations->item(row, COLUMN_TESTTYPE);
        limit_item = dynamic_cast<TableNumberWidgetItem*>(ui->tableInvitations->item(row, COLUMN_LIMIT));
        count_item = dynamic_cast<TableNumberWidgetItem*>(ui->tableInvitations->item(row, COLUMN_COUNT));
        key_item = ui->tableInvitations->item(row, COLUMN_KEY);
        creation_item = dynamic_cast<TableDateWidgetItem*>(ui->tableInvitations->item(row, COLUMN_CREATION));
        expiration_item = dynamic_cast<TableDateWidgetItem*>(ui->tableInvitations->item(row, COLUMN_EXPIRATION));
    }else{
        ui->tableInvitations->setRowCount(ui->tableInvitations->rowCount()+1);
        int current_row = ui->tableInvitations->rowCount();
        user_item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_USER)),"");
        ui->tableInvitations->setItem(current_row, COLUMN_USER, user_item);
        participant_item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_PARTICIPANT)),"");
        ui->tableInvitations->setItem(current_row, COLUMN_PARTICIPANT, participant_item);
        device_item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_DEVICE)),"");
        ui->tableInvitations->setItem(current_row, COLUMN_DEVICE, device_item);
        test_type_item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_TESTTYPE)),"");
        ui->tableInvitations->setItem(current_row, COLUMN_TESTTYPE, test_type_item);
        limit_item = new TableNumberWidgetItem();
        ui->tableInvitations->setItem(current_row, COLUMN_LIMIT, limit_item);
        count_item = new TableNumberWidgetItem();
        ui->tableInvitations->setItem(current_row, COLUMN_COUNT, count_item);
        key_item = new QTableWidgetItem();
        ui->tableInvitations->setItem(current_row, COLUMN_KEY, key_item);
        creation_item = new TableDateWidgetItem();
        ui->tableInvitations->setItem(current_row, COLUMN_CREATION, creation_item);
        expiration_item = new TableDateWidgetItem();
        ui->tableInvitations->setItem(current_row, COLUMN_EXPIRATION, expiration_item);
    }

    if (invitation->hasFieldName("test_invitation_user")){
        QJsonObject obj = invitation->getFieldValue("test_invitation_user").toJsonObject();
        user_item->setText(obj["user_firstname"].toString() + " " + obj["user_lastname"].toString());
    }
    if (invitation->hasFieldName("test_invitation_participant")){
        QJsonObject obj = invitation->getFieldValue("test_invitation_participant").toJsonObject();
        participant_item->setText(obj["participant_name"].toString());
    }
    if (invitation->hasFieldName("test_invitation_device")){
        QJsonObject obj = invitation->getFieldValue("test_invitation_device").toJsonObject();
        device_item->setText(invitation->getFieldValue("device_name").toString());
    }
    if (invitation->hasFieldName("test_invitation_test_type")){
        QJsonObject obj = invitation->getFieldValue("test_invitation_test_type").toJsonObject();
        device_item->setText(invitation->getFieldValue("test_type_name").toString());
    }
    limit_item->setText(invitation->getFieldValue("test_invitation_max_count").toString());
    count_item->setText(invitation->getFieldValue("test_invitation_usage").toString());
    key_item->setText(invitation->getFieldValue("test_invitation_key").toString());
    creation_item->setDate(invitation->getFieldValue("test_invitation_creation_date"));
    expiration_item->setDate(invitation->getFieldValue("test_invitation_expiration_date"));

}

void TestInvitationsWidget::setViewMode(const ViewMode &mode)
{
    m_currentView = mode;

    switch(m_currentView){

    case VIEWMODE_UNKNOWN:
        break;
    case VIEWMODE_DEVICE:
        ui->tableInvitations->hideColumn(InviteColumns::COLUMN_PARTICIPANT);
        ui->tableInvitations->hideColumn(InviteColumns::COLUMN_USER);
        ui->tableInvitations->showColumn(InviteColumns::COLUMN_DEVICE);
        break;
    case VIEWMODE_PARTICIPANT:
        ui->tableInvitations->showColumn(InviteColumns::COLUMN_PARTICIPANT);
        ui->tableInvitations->hideColumn(InviteColumns::COLUMN_USER);
        ui->tableInvitations->hideColumn(InviteColumns::COLUMN_DEVICE);
        break;
    case VIEWMODE_PROJECT:
        // For now... Someday, also adds devices and users to project
        ui->tableInvitations->showColumn(InviteColumns::COLUMN_PARTICIPANT);
        ui->tableInvitations->hideColumn(InviteColumns::COLUMN_USER);
        ui->tableInvitations->hideColumn(InviteColumns::COLUMN_DEVICE);
        break;
    case VIEWMODE_USER:
        ui->tableInvitations->hideColumn(InviteColumns::COLUMN_PARTICIPANT);
        ui->tableInvitations->showColumn(InviteColumns::COLUMN_USER);
        ui->tableInvitations->hideColumn(InviteColumns::COLUMN_DEVICE);
        break;
    }
}

void TestInvitationsWidget::on_btnInvite_clicked()
{
    if (m_invitationDialog)
        m_invitationDialog->deleteLater();

    m_invitationDialog = new TestInvitationDialog(this);
    m_invitationDialog->setTestTypes(m_testTypes);

    connect(m_invitationDialog, &TestInvitationDialog::finished, this, &TestInvitationsWidget::onTestInvitationDialogFinished);
    m_invitationDialog->show();
}

