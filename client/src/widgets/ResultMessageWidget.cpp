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
    m_displayTimer.setSingleShot(false);

    // Init progress bar
    ui->progTimer->hide();

    // Connect signals
    connect(&m_msgTimer, &QTimer::timeout, this, &ResultMessageWidget::showNextMessage);
    connect(&m_displayTimer, &QTimer::timeout, this, &ResultMessageWidget::updateTimeoutProgress);

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
    showNextMessage();
}

bool ResultMessageWidget::hasMessagesWaiting(const Message::MessageType &msg_type)
{
    if (msg_type==Message::MESSAGE_NONE)
        // We return if there's message at all
        return m_messages.isEmpty();

    // Otherwise, we look for message of that type in the queue
    for (Message msg:std::as_const(m_messages)){
        if (msg.getMessageType() == msg_type)
            return true;
    }
    return false;
}

void ResultMessageWidget::clearMessages()
{
    m_messages.clear();
    showNextMessage();
}

void ResultMessageWidget::setDisplayTime(const int &display_time)
{
    m_displayTime = display_time;
    m_msgTimer.setInterval(m_displayTime);
    ui->progTimer->setMaximum(m_displayTime);
    m_displayTimer.setInterval(500);
}

void ResultMessageWidget::stopTimers()
{
    m_msgTimer.stop();
    m_displayTimer.stop();
    ui->progTimer->hide();
}

void ResultMessageWidget::startTimers()
{
    m_msgTimer.start();
    m_displayTimer.start();
    ui->progTimer->setValue(m_displayTime);
    ui->progTimer->show();
}

void ResultMessageWidget::showNextMessage()
{
    stopTimers();
    if (m_messages.isEmpty()){
        m_currentMessage.setMessage(Message::MESSAGE_NONE,"");
        emit nextMessageShown(m_currentMessage);
        return;
    }

    m_currentMessage = m_messages.takeFirst();

    QString background_color = "rgba(128,128,128,50%)";
    // QString icon_path = "";
    ui->btnCloseMessage->show();

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
        background_color = "rgba(50, 137, 168, 50%)";
        ui->icoMessage->setMovie(m_loadingIcon);
        m_loadingIcon->start();
        ui->btnCloseMessage->hide();
        break;
    default:
        break;
    }
    ui->frameMessage->setStyleSheet("QWidget#frameMessage{background-color: " + background_color + ";}");
    ui->lblMessage->setText(m_currentMessage.getMessageText());
    if (m_currentMessage.getMessageType() != Message::MESSAGE_ERROR
            && m_currentMessage.getMessageType()!=Message::MESSAGE_NONE
            && m_currentMessage.getMessageType()!=Message::MESSAGE_WORKING)
        startTimers();

    QPropertyAnimation *animate = new QPropertyAnimation(this,"windowOpacity",this->parent());
    animate->setDuration(1000);
    animate->setStartValue(0.0);
    animate->setKeyValueAt(0.1, 0.8);
    animate->setKeyValueAt(0.9, 0.8);
    animate->setEndValue(0.0);
    animate->start(QAbstractAnimation::DeleteWhenStopped);

    emit nextMessageShown(m_currentMessage);
}

void ResultMessageWidget::updateTimeoutProgress()
{
    int new_value = ui->progTimer->value() - m_displayTimer.interval();
    if (new_value>0)
        ui->progTimer->setValue(new_value);
    else
        ui->progTimer->setValue(0);
    ui->progTimer->update();
}

void ResultMessageWidget::on_btnCloseMessage_clicked()
{
    showNextMessage();
}

