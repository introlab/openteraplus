#ifndef ONLINEMANAGERWIDGET_H
#define ONLINEMANAGERWIDGET_H

#include <QWidget>
#include <QListWidgetItem>

#include "ComManager.h"

namespace Ui {
class OnlineManagerWidget;
}

class OnlineManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OnlineManagerWidget(QWidget *parent = nullptr);
    ~OnlineManagerWidget();

    void setComManager(ComManager* comMan);

private:
    Ui::OnlineManagerWidget     *ui;
    ComManager                  *m_comManager;

    // Data management
    QHash<QString, QListWidgetItem*> m_onlineUsers;        // User UUID mapping of online users
    QHash<QString, QListWidgetItem*> m_onlineParticipants; // Participant UUID mapping of online participants
    QHash<QString, QListWidgetItem*> m_onlineDevices;      // Participant UUID mapping of online participants

    void initUi();
    void connectSignals();

    void updateOnlineUser(const QString &uuid, const bool &online,  const QString &name);
    void updateOnlineParticipant(const QString &uuid, const bool &online,  const QString &name);
    void updateOnlineDevice(const QString &uuid, const bool &online,  const QString &name);

private slots:
    void on_btnShowOnlineParticipants_clicked();
    void on_btnShowOnlineUsers_clicked();
    void on_btnShowOnlineDevices_clicked();
};

#endif // ONLINEMANAGERWIDGET_H
