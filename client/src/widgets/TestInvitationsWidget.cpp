#include "TestInvitationsWidget.h"
#include "ui_TestInvitationsWidget.h"

TestInvitationsWidget::TestInvitationsWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TestInvitationsWidget)
{
    ui->setupUi(this);
}

TestInvitationsWidget::~TestInvitationsWidget()
{
    delete ui;
}
