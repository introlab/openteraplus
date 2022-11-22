#ifndef LOGVIEWWIDGET_H
#define LOGVIEWWIDGET_H

#include <QWidget>
#include "managers/ComManager.h"

#include "LogEvent.pb.h"

namespace Ui {
class LogViewWidget;
}

class LogViewWidget : public QWidget
{
    Q_OBJECT

public:
    enum ViewMode{
        VIEW_LOGS_NONE = 0,
        VIEW_LOGS_ALL,
        VIEW_LOGINS_ALL,
        VIEW_LOGINS_USER,
        VIEW_LOGINS_PARTICIPANT,
        VIEW_LOGINS_DEVICE
    };

    explicit LogViewWidget(QWidget *parent = nullptr);
    explicit LogViewWidget(ComManager* comMan = nullptr, QWidget *parent = nullptr);
    ~LogViewWidget();

    void setComManager(ComManager* comMan);
    void setViewMode(const ViewMode &mode, const QString& uuid = QString(), const bool& autoload=false);

    void refreshData();

private:
    Ui::LogViewWidget*  ui;
    ComManager*         m_comManager;
    ViewMode            m_currentMode;
    QString             m_currentUuid;

    void connectSignals();

    QStringList getLogLevelNames();
    QString getLogLevelName(const LogEvent::LogLevel &level);

    void queryLoginsForUser();
    void queryLoginsForParticipant();
    void queryLoginsForDevice();
    void queryAllLogins();
    void queryAllLogs();

private slots:
    void processLogsLogins(QList<TeraData> logins, QUrlQuery reply_data);
    void processLogsLogs(QList<TeraData> logins, QUrlQuery reply_data);
    void on_btnFilter_clicked();
};

#endif // LOGVIEWWIDGET_H
