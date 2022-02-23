#ifndef RESULTMESSAGEWIDGET_H
#define RESULTMESSAGEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QMovie>
#include <QPropertyAnimation>

#include "data/Message.h"

namespace Ui {
class ResultMessageWidget;
}

class ResultMessageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ResultMessageWidget(QWidget *parent = nullptr, int displayTime = 5000);
    ~ResultMessageWidget();

    void addMessage(const Message &msg);
    bool hasMessagesWaiting(const Message::MessageType &msg_type = Message::MESSAGE_NONE);
    void clearMessages();

    void setDisplayTime(const int &display_time);

private:
    Ui::ResultMessageWidget *ui;

    QList<Message>              m_messages;
    Message                     m_currentMessage;
    QTimer                      m_msgTimer;
    QTimer                      m_displayTimer;

    int                         m_displayTime;

    // UI items
    QMovie*                     m_loadingIcon;

    void stopTimers();
    void startTimers();

public slots:

    void showNextMessage();

    void updateTimeoutProgress();

signals:
    void nextMessageShown(Message current_message);

private slots:
    void on_btnCloseMessage_clicked();
};

#endif // RESULTMESSAGEWIDGET_H
