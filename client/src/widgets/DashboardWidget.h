#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>

#include "managers/ComManager.h"

namespace Ui {
class DashboardWidget;
}

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(ComManager* comMan, QWidget *parent = nullptr);
    ~DashboardWidget();

private slots:
    void on_btnUpcomingSessions_clicked();
    void on_btnRecentParticipants_clicked();
    void on_btnAttention_clicked();
    void on_icoUpcomingSessions_clicked();
    void on_icoRecentParticipants_clicked();
    void on_icoAttention_clicked();

private:
    Ui::DashboardWidget *ui;

    ComManager* m_comManager;

    void connectSignals();
};

#endif // DASHBOARDWIDGET_H
