#include "OnlineManagerWidget.h"
#include "ui_OnlineManagerWidget.h"

// Must be reset each time a site is changed.
// 1. Query online * using APIs
// 2. React to events using protobuf messages
//

OnlineManagerWidget::OnlineManagerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OnlineManagerWidget)
{
    ui->setupUi(this);
}

OnlineManagerWidget::~OnlineManagerWidget()
{
    delete ui;
}

void OnlineManagerWidget::setComManager(ComManager *comMan)
{
    m_comManager = comMan;

    connectSignals();

    initUi();
}

void OnlineManagerWidget::initUi()
{
    ui->lstOnlineDevices->hide();
    ui->lstOnlineUsers->hide();
    ui->btnShowOnlineParticipants->setChecked(true); // Expand participants by default
}

void OnlineManagerWidget::connectSignals()
{

}

void OnlineManagerWidget::updateOnlineUser(const QString &uuid, const bool &online, const QString &name)
{
    /*QListWidgetItem* user_item = nullptr;

    if (m_onlineUsers.contains(uuid)){
        user_item = m_onlineUsers[uuid];
    }else{
        // If offline, we don't need to anything since it's there !
        if (online){
            user_item = new QListWidgetItem(QIcon("://icons/software_user_online.png"), name);
            //ui->lstOnlineUsers->addItem(user_item);
            m_onlineUsers[uuid] = user_item;
        }
    }

    if (user_item){
        if (!online){
            // We must remove that item
            delete user_item;
            m_onlineUsers.remove(uuid);
        }else{
            // Update name if needed
            user_item->setText(name);

            // Resort items
            //ui->lstOnlineUsers->sortItems();
        }
    }*/
}

void OnlineManagerWidget::updateOnlineParticipant(const QString &uuid, const bool &online, const QString &name)
{
    /*QListWidgetItem* participant_item = nullptr;

    if (m_onlineParticipants.contains(uuid)){
        participant_item = m_onlineParticipants[uuid];
    }else{
        // If offline, we don't need to anything since it's there !
        if (online){
            participant_item = new QListWidgetItem(QIcon("://icons/software_user_online.png"), name);
            m_onlineParticipants[uuid] = participant_item;
        }
    }

    if (participant_item){
        if (!online){
            // We must remove that item
            delete participant_item;
            m_onlineParticipants.remove(uuid);
        }else{
            // Update name if needed
            participant_item->setText(name);

            // Resort items
            //ui->lstOnlineUsers->sortItems();
        }
    }*/
}

void OnlineManagerWidget::updateOnlineDevice(const QString &uuid, const bool &online, const QString &name)
{

}

void OnlineManagerWidget::on_btnShowOnlineParticipants_clicked()
{
    ui->lstOnlineParticipants->setVisible(ui->btnShowOnlineParticipants->isChecked());
}

void OnlineManagerWidget::on_btnShowOnlineUsers_clicked()
{
    ui->lstOnlineUsers->setVisible(ui->btnShowOnlineUsers->isChecked());
}

void OnlineManagerWidget::on_btnShowOnlineDevices_clicked()
{
    ui->lstOnlineDevices->setVisible(ui->btnShowOnlineDevices->isChecked());
}
