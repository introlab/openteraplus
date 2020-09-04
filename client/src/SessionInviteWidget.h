#ifndef SESSIONINVITEWIDGET_H
#define SESSIONINVITEWIDGET_H

#include <QWidget>

namespace Ui {
class SessionInviteWidget;
}

class SessionInviteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SessionInviteWidget(QWidget *parent = nullptr);
    ~SessionInviteWidget();

private slots:
    void on_btnManageInvitees_clicked();

private:
    Ui::SessionInviteWidget *ui;
};

#endif // SESSIONINVITEWIDGET_H
