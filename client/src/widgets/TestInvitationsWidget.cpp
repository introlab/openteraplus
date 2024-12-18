#include <QToolButton>
#include <QClipboard>
#include <QDesktopServices>

#include "TestInvitationsWidget.h"
#include "TableNumberWidgetItem.h"
#include "TableDateWidgetItem.h"
#include "GlobalMessageBox.h"

#include "ui_TestInvitationsWidget.h"

TestInvitationsWidget::TestInvitationsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TestInvitationsWidget)
{
    ui->setupUi(this);

    initUI();

    connectSignals();
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
    connect(m_comManager, &ComManager::deleteResultsOK, this, &TestInvitationsWidget::deleteDataReply);

}

void TestInvitationsWidget::setCurrentTestTypes(const QList<TeraData> &test_types)
{
    m_testTypes = test_types;
}

void TestInvitationsWidget::setInvitableDevices(QHash<int, TeraData> *devices)
{
    m_invitableDevices = devices;
}

void TestInvitationsWidget::setInvitableParticipants(QHash<int, TeraData> *participants)
{
    m_invitableParticipants = participants;
}

void TestInvitationsWidget::setInvitableUsers(QHash<int, TeraData> *users)
{
    m_invitableUsers = users;
}

void TestInvitationsWidget::setEnableEmail(const bool &enable_email)
{
    m_enableEmails = enable_email;
}

void TestInvitationsWidget::loadForProject(const int &id_project)
{
    setViewMode(ViewMode::VIEWMODE_PROJECT);
    ui->tableInvitations->clearContents();
    ui->tableInvitations->setRowCount(0);
    m_listInvitations_items.clear();
    m_invitations.clear();

    if (m_comManager){
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(id_project));
        args.addQueryItem(WEB_QUERY_WITH_URLS, "1");
        args.addQueryItem(WEB_QUERY_FULL, "1");
        m_comManager->doGet(WEB_TESTINVITATION_PATH, args);
    }
}

void TestInvitationsWidget::loadForParticipant(TeraData* participant)
{
    m_currentData = participant;
    setViewMode(ViewMode::VIEWMODE_PARTICIPANT);
    ui->tableInvitations->clearContents();
    ui->tableInvitations->setRowCount(0);
    m_listInvitations_items.clear();
    m_invitations.clear();

    if (m_comManager){
        QUrlQuery args;
        args.addQueryItem(WEB_QUERY_ID_PARTICIPANT, QString::number(participant->getId()));
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

    ui->tableInvitations->resizeColumnsToContents();
}

void TestInvitationsWidget::deleteDataReply(QString path, int id)
{
    if (id==0)
        return;

    if (path == WEB_TESTINVITATION_PATH){
        // An invitation got deleted - check if it affects the current display
        if (m_listInvitations_items.contains(id)){
            ui->tableInvitations->removeRow(m_listInvitations_items[id]->row());
            m_listInvitations_items.remove(id);
            m_invitations.remove(id);
        }
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
        int current_row = ui->tableInvitations->rowCount()-1;
        user_item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_USER)),"");
        ui->tableInvitations->setItem(current_row, COLUMN_USER, user_item);
        participant_item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_PARTICIPANT)),"");
        ui->tableInvitations->setItem(current_row, COLUMN_PARTICIPANT, participant_item);
        device_item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_DEVICE)),"");
        ui->tableInvitations->setItem(current_row, COLUMN_DEVICE, device_item);
        test_type_item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_TESTTYPE)),"");
        ui->tableInvitations->setItem(current_row, COLUMN_TESTTYPE, test_type_item);
        limit_item = new TableNumberWidgetItem();
        limit_item->setTextAlignment(Qt::AlignCenter);
        ui->tableInvitations->setItem(current_row, COLUMN_LIMIT, limit_item);
        count_item = new TableNumberWidgetItem();
        count_item->setTextAlignment(Qt::AlignCenter);
        ui->tableInvitations->setItem(current_row, COLUMN_COUNT, count_item);
        key_item = new QTableWidgetItem();
        ui->tableInvitations->setItem(current_row, COLUMN_KEY, key_item);
        creation_item = new TableDateWidgetItem();
        ui->tableInvitations->setItem(current_row, COLUMN_CREATION, creation_item);
        expiration_item = new TableDateWidgetItem();
        expiration_item->setShowTime(false);
        expiration_item->setTextAlignment(Qt::AlignCenter);
        ui->tableInvitations->setItem(current_row, COLUMN_EXPIRATION, expiration_item);

        // Create action buttons
        QFrame* action_frame = new QFrame();
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(1);
        layout->setAlignment(Qt::AlignLeft);
        action_frame->setLayout(layout);

        QToolButton* btnCopy = new QToolButton(action_frame);
        btnCopy->setIcon(m_copyIcon);
        btnCopy->setProperty("id_test_invitation", id_test_invitation);
        btnCopy->setCursor(Qt::PointingHandCursor);
        btnCopy->setMaximumWidth(32);
        btnCopy->setToolTip(tr("Copier le lien"));
        connect(btnCopy, &QToolButton::clicked, this, &TestInvitationsWidget::on_copyInvitation);
        layout->addWidget(btnCopy);

        QToolButton* btnView = new QToolButton(action_frame);
        btnView->setIcon(m_viewIcon);
        btnView->setProperty("id_test_invitation", id_test_invitation);
        btnView->setCursor(Qt::PointingHandCursor);
        btnView->setMaximumWidth(32);
        btnView->setToolTip(tr("Ouvrir le lien"));
        connect(btnView, &QToolButton::clicked, this, &TestInvitationsWidget::on_viewInvitation);
        layout->addWidget(btnView);

        QToolButton* btnEdit = new QToolButton(action_frame);
        btnEdit->setIcon(m_editIcon);
        btnEdit->setProperty("id_test_invitation", id_test_invitation);
        btnEdit->setCursor(Qt::PointingHandCursor);
        btnEdit->setMaximumWidth(32);
        btnEdit->setToolTip(tr("Éditer"));
        connect(btnEdit, &QToolButton::clicked, this, &TestInvitationsWidget::on_editInvitation);
        layout->addWidget(btnEdit);

        QToolButton* btnDelete = new QToolButton(action_frame);
        btnDelete->setIcon(m_deleteIcon);
        btnDelete->setProperty("id_test_invitation", id_test_invitation);
        btnDelete->setCursor(Qt::PointingHandCursor);
        btnDelete->setMaximumWidth(32);
        btnDelete->setToolTip(tr("Supprimer"));
        connect(btnDelete, &QToolButton::clicked, this, &TestInvitationsWidget::on_deleteInvitation);
        layout->addWidget(btnDelete);

        ui->tableInvitations->setCellWidget(current_row, COLUMN_ACTIONS, action_frame);

        m_listInvitations_items[id_test_invitation] = user_item;
    }
    QColor invite_color = Qt::white;
    int max_count = invitation->getFieldValue("test_invitation_max_count").toInt();
    int current_count = invitation->getFieldValue("test_invitation_count").toInt();
    if (max_count > 0 && current_count >= max_count){
        invite_color = Qt::green;
    }else{
        if (invitation->getFieldValue("test_invitation_expiration_date").toDate() < QDate::currentDate()){
            invite_color = Qt::red;
        }
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
        device_item->setText(obj["device_name"].toString());
    }
    if (invitation->hasFieldName("test_invitation_test_type")){
        QJsonObject obj = invitation->getFieldValue("test_invitation_test_type").toJsonObject();
        test_type_item->setText(obj["test_type_name"].toString());
    }

    user_item->setForeground(invite_color);
    participant_item->setForeground(invite_color);
    device_item->setForeground(invite_color);
    test_type_item->setForeground(invite_color);
    limit_item->setForeground(invite_color);
    count_item->setForeground(invite_color);
    key_item->setForeground(invite_color);
    creation_item->setForeground(invite_color);
    expiration_item->setForeground(invite_color);

    limit_item->setText(invitation->getFieldValue("test_invitation_max_count").toString());
    count_item->setText(invitation->getFieldValue("test_invitation_count").toString());
    key_item->setText(invitation->getFieldValue("test_invitation_key").toString());
    creation_item->setDate(invitation->getFieldValue("test_invitation_creation_date"));
    expiration_item->setDate(invitation->getFieldValue("test_invitation_expiration_date"));

    m_invitations[id_test_invitation] = *invitation;
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

    m_invitationDialog = new TestInvitationDialog(m_comManager, this);
    m_invitationDialog->setTestTypes(m_testTypes);
    m_invitationDialog->setInvitableDevices(m_invitableDevices);
    m_invitationDialog->setInvitableParticipants(m_invitableParticipants);
    m_invitationDialog->setInvitableUsers(m_invitableUsers);
    if (m_currentView == VIEWMODE_PARTICIPANT){
        if (m_currentData){
            QHash<int, TeraData> invitable_participant;
            invitable_participant[m_currentData->getId()] = *m_currentData;
            m_invitationDialog->setInvitableParticipants(&invitable_participant);
            m_invitationDialog->addParticipantsToInvitation(QStringList() << m_currentData->getUuid());
            m_invitationDialog->setEnableInviteesList(false);
        }
    }
    m_invitationDialog->setEnableEmail(m_enableEmails);

    connect(m_invitationDialog, &TestInvitationDialog::finished, this, &TestInvitationsWidget::onTestInvitationDialogFinished);
    m_invitationDialog->show();
}

void TestInvitationsWidget::on_deleteInvitation()
{
    // Check if the sender is a QToolButton (from the action column)
    QToolButton* action_btn = dynamic_cast<QToolButton*>(sender());
    if (action_btn){
        // Select row according to the invitation id of that button
        int id_test_invitation = action_btn->property("id_test_invitation").toInt();
        QTableWidgetItem* invitation_item = m_listInvitations_items[id_test_invitation];
        if (invitation_item)
            ui->tableInvitations->selectRow(invitation_item->row());
    }

    if (ui->tableInvitations->selectedItems().empty())
        return;

    GlobalMessageBox diag;
    QMessageBox::StandardButton answer = QMessageBox::No;
    if (ui->tableInvitations->selectionModel()->selectedRows().count() == 1){
        answer = diag.showYesNo(tr("Suppression?"), tr("Êtes-vous sûrs de vouloir supprimer cette invitation?"));
    }else{
        answer = diag.showYesNo(tr("Suppression?"), tr("Êtes-vous sûrs de vouloir supprimer toutes les invitations sélectionnées?"));
    }

    if (answer == QMessageBox::Yes){
        // We must delete!
        foreach(QModelIndex selected_row, ui->tableInvitations->selectionModel()->selectedRows()){
            QTableWidgetItem* base_item = ui->tableInvitations->item(selected_row.row(), 0);
            m_comManager->doDelete(WEB_TESTINVITATION_PATH, m_listInvitations_items.key(base_item));
        }
    }
}

void TestInvitationsWidget::on_editInvitation()
{
    int id_test_invitation = -1;
    // Check if the sender is a QToolButton (from the action column)
    QToolButton* action_btn = dynamic_cast<QToolButton*>(sender());
    if (action_btn){
        // Select row according to the invitation id of that button
        id_test_invitation = action_btn->property("id_test_invitation").toInt();
        QTableWidgetItem* invitation_item = m_listInvitations_items[id_test_invitation];
        if (invitation_item)
            ui->tableInvitations->selectRow(invitation_item->row());
    }

    if (ui->tableInvitations->selectionModel()->selectedRows().count() > 1)
        return;

    if (id_test_invitation <= 0){
        QTableWidgetItem* base_item = ui->tableInvitations->item(ui->tableInvitations->selectionModel()->selectedRows().first().row(), 0);
        id_test_invitation = m_listInvitations_items.key(base_item);
    }

    if (m_invitationDialog)
        m_invitationDialog->deleteLater();

    m_invitationDialog = new TestInvitationDialog(m_comManager, this);
    m_invitationDialog->setTestTypes(m_testTypes);
    m_invitationDialog->setCurrentData(&m_invitations[id_test_invitation]);
    m_invitationDialog->setEnableEmail(m_enableEmails);

    connect(m_invitationDialog, &TestInvitationDialog::finished, this, &TestInvitationsWidget::onTestInvitationDialogFinished);
    m_invitationDialog->show();

}

void TestInvitationsWidget::on_copyInvitation()
{

    int id_test_invitation = -1;
    QClipboard* clipboard = QApplication::clipboard();
    // Check if the sender is a QToolButton (from the action column)
    QToolButton* action_btn = dynamic_cast<QToolButton*>(sender());
    if (action_btn){
        // Select row according to the invitation id of that button
        id_test_invitation = action_btn->property("id_test_invitation").toInt();
        QTableWidgetItem* invitation_item = m_listInvitations_items[id_test_invitation];
        if (invitation_item)
            ui->tableInvitations->selectRow(invitation_item->row());
    }

    if (ui->tableInvitations->selectionModel()->selectedRows().count() > 1)
        return;

    if (id_test_invitation <= 0){
        QTableWidgetItem* base_item = ui->tableInvitations->item(ui->tableInvitations->selectionModel()->selectedRows().first().row(), 0);
        id_test_invitation = m_listInvitations_items.key(base_item);
    }

    clipboard->setText(m_invitations[id_test_invitation].getFieldValue("test_invitation_url").toString(), QClipboard::Clipboard);
}

void TestInvitationsWidget::on_viewInvitation()
{
    int id_test_invitation = -1;
    // Check if the sender is a QToolButton (from the action column)
    QToolButton* action_btn = dynamic_cast<QToolButton*>(sender());
    if (action_btn){
        // Select row according to the invitation id of that button
        id_test_invitation = action_btn->property("id_test_invitation").toInt();
        QTableWidgetItem* invitation_item = m_listInvitations_items[id_test_invitation];
        if (invitation_item)
            ui->tableInvitations->selectRow(invitation_item->row());
    }

    if (ui->tableInvitations->selectionModel()->selectedRows().count() > 1)
        return;

    if (id_test_invitation <= 0){
        QTableWidgetItem* base_item = ui->tableInvitations->item(ui->tableInvitations->selectionModel()->selectedRows().first().row(), 0);
        id_test_invitation = m_listInvitations_items.key(base_item);
    }
    QDesktopServices::openUrl(m_invitations[id_test_invitation].getFieldValue("test_invitation_url").toString());
}

void TestInvitationsWidget::initUI()
{
    // Load table icons
    m_deleteIcon = QIcon(":/icons/delete_old.png");
    m_viewIcon = QIcon(":/icons/search.png");
    m_editIcon = QIcon(":/icons/edit.png");
    m_copyIcon = QIcon(":/icons/copy.png");
}

void TestInvitationsWidget::connectSignals()
{
    connect(ui->btnDeleteInvitation, &QPushButton::clicked, this, &TestInvitationsWidget::on_deleteInvitation);
    connect(ui->btnEdit, &QPushButton::clicked, this, &TestInvitationsWidget::on_editInvitation);
    connect(ui->btnCopyLink, &QPushButton::clicked, this, &TestInvitationsWidget::on_copyInvitation);
    connect(ui->btnOpen, &QPushButton::clicked, this, &TestInvitationsWidget::on_viewInvitation);
}

void TestInvitationsWidget::on_tableInvitations_itemSelectionChanged()
{
    bool has_selection = !ui->tableInvitations->selectedItems().empty();
    int selected_count = ui->tableInvitations->selectionModel()->selectedRows().count();

    ui->btnCopyLink->setEnabled(selected_count == 1);
    ui->btnDeleteInvitation->setEnabled(has_selection);
    ui->btnEdit->setEnabled(selected_count == 1);
    ui->btnOpen->setEnabled(selected_count == 1);
}

