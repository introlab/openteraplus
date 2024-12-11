#ifndef TESTINVITATIONSWIDGET_H
#define TESTINVITATIONSWIDGET_H

#include <QWidget>

namespace Ui {
class TestInvitationsWidget;
}

class TestInvitationsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TestInvitationsWidget(QWidget *parent = nullptr);
    ~TestInvitationsWidget();

private:
    Ui::TestInvitationsWidget *ui;
};

#endif // TESTINVITATIONSWIDGET_H
