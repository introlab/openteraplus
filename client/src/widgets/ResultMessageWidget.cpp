#include "ResultMessageWidget.h"
#include "ui_ResultMessageWidget.h"

ResultMessageWidget::ResultMessageWidget(QWidget *parent, int displayTime) :
    QWidget(parent),
    ui(new Ui::ResultMessageWidget)
{
    ui->setupUi(this);

    // Initialize timer and message queue
    m_currentMessage.setMessage(Message::MESSAGE_NONE,"");
    setDisplayTime(displayTime);
    m_msgTimer.setSingleShot(true);

    // Connect signals
    connect(&m_msgTimer, &QTimer::timeout, this, &ResultMessageWidget::showNextMessage);

    // Setup loading icon animation
    m_loadingIcon = new QMovie("://status/loading.gif");
}

ResultMessageWidget::~ResultMessageWidget()
{
    delete ui;
    delete m_loadingIcon;
}

void ResultMessageWidget::addMessage(const Message &msg)
{
    m_messages.append(msg);
}

bool ResultMessageWidget::hasMessagesWaiting(const Message::MessageType &msg_type)
{
    if (msg_type==Message::MESSAGE_NONE)
        // We return if there's message at all
        return m_messages.isEmpty();

    // Otherwise, we look for message of that type in the queue
    for (Message msg:qAsConst(m_messages)){
        if (msg.getMessageType() == msg_type)
            return true;
    }
    return false;
}

void ResultMessageWidget::setDisplayTime(const int &display_time)
{
    m_displayTime = display_time;
    m_msgTimer.setInterval(m_displayTime);
}

void ResultMessageWidget::showNextMessage()
{
    m_msgTimer.stop();
    if (m_messages.isEmpty()){
        m_currentMessage.setMessage(Message::MESSAGE_NONE,"");
        emit nextMessageShown(m_currentMessage);
        return;
    }

    m_currentMessage = m_messages.takeFirst();

    QString background_color = "rgba(128,128,128,50%)";
    // QString icon_path = "";

    switch(m_currentMessage.getMessageType()){
    case Message::MESSAGE_OK:
        background_color = "rgba(0,200,0,50%)";
        ui->icoMessage->setPixmap(QPixmap("://icons/ok.png"));
        break;
    case Message::MESSAGE_ERROR:
        background_color = "rgba(200,0,0,50%)";
        ui->icoMessage->setPixmap(QPixmap("://icons/error.png"));
        break;
    case Message::MESSAGE_WARNING:
        background_color = "rgba(255,85,0,50%)";
        ui->icoMessage->setPixmap(QPixmap("://icons/warning.png"));
        break;
    case Message::MESSAGE_WORKING:
        background_color = "rgba(128,128,128,50%)";
        ui->icoMessage->setMovie(m_loadingIcon);
        m_loadingIcon->start();
        break;
    default:
        break;
    }
    ui->frameMessage->setStyleSheet("QWidget#frameMessage{background-color: " + background_color + ";}");
    ui->lblMessage->setText(m_currentMessage.getMessageText());
    if (m_currentMessage.getMessageType() != Message::MESSAGE_ERROR && m_currentMessage.getMessageType()!=Message::MESSAGE_NONE)
        m_msgTimer.start();

    QPropertyAnimation *animate = new QPropertyAnimation(this,"windowOpacity",this->parent());
    animate->setDuration(1000);
    animate->setStartValue(0.0);
    animate->setKeyValueAt(0.1, 0.8);
    animate->setKeyValueAt(0.9, 0.8);
    animate->setEndValue(0.0);
    animate->start(QAbstractAnimation::DeleteWhenStopped);

    emit nextMessageShown(m_currentMessage);
}

void ResultMessageWidget::on_btnCloseMessage_clicked()
{
    showNextMessage();
}

