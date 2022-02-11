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
    explicit ResultMessageWidget(QWidget *parent = nullptr, int displayTime = 8000);
    ~ResultMessageWidget();

    void addMessage(const Message &msg);
    bool hasMessagesWaiting(const Message::MessageType &msg_type = Message::MESSAGE_NONE);

    void setDisplayTime(const int &display_time);

private:
    Ui::ResultMessageWidget *ui;

    QList<Message>              m_messages;
    Message                     m_currentMessage;
    QTimer                      m_msgTimer;

    int                         m_displayTime;

    // UI items
    QMovie*                     m_loadingIcon;

public slots:

    void showNextMessage();

signals:
    void nextMessageShown(Message current_message);

private slots:
    void on_btnCloseMessage_clicked();
};

#endif // RESULTMESSAGEWIDGET_H
