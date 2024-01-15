#include "BaseDialog.h"
#include <QSizePolicy>
#include <QVBoxLayout>

BaseDialog::BaseDialog(QWidget *parent)
    : QDialog(parent)
{
    m_ui.setupUi(this);

    m_centralWidgetLayout = new QVBoxLayout(m_ui.centralWidget);
    //m_centralWidgetLayout->setMargin(0);
    m_centralWidgetLayout->setContentsMargins(0,0,0,0);

    // setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

    connect(m_ui.btnClose, &QPushButton::clicked, this, &QDialog::accept);

}

void BaseDialog::setCentralWidget(QWidget *widget)
{
    widget->setParent(m_ui.centralWidget);
    m_centralWidgetLayout->addWidget(widget);
    //widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

}

void BaseDialog::showEvent(QShowEvent * event)
{
    QDialog::showEvent(event);

   /* if (parentWidget()){
        move(parentWidget()->width()/2 - width()/2, parentWidget()->height()/2 - height()/2);
    }*/
}
