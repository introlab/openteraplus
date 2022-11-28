#ifndef LOGVIEWWIDGET_H
#define LOGVIEWWIDGET_H

#include <QWidget>
#include "managers/ComManager.h"

#include "LogEvent.pb.h"
#include "LoginEvent.pb.h"

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
    void setUuidName(const QString& uuid, const QString& name);

    void refreshData(const bool &stats_only = true);

private:
    Ui::LogViewWidget*      ui;
    ComManager*             m_comManager;
    ViewMode                m_currentMode;
    QString                 m_currentUuid;
    int                     m_maxCount;
    bool                    m_filtering; // Applying selected filters
    bool                    m_listening; // Listening for logs
    QHash<QString, QString> m_uuidsNames;

    void connectSignals();
    void disconnectSignals();

    QStringList getLogLevelNames();
    QString getLogLevelName(const LogEvent::LogLevel &level);
    QString getLogLevelIcon(const LogEvent::LogLevel &level);

    QString getLoginTypeName(const LoginEvent::LoginType &login_type);
    QString getLoginTypeIcon(const LoginEvent::LoginType &login_type);

    QString getLoginStatusName(const LoginEvent::LoginStatus &status);
    QString getLoginStatusIcon(const LoginEvent::LoginStatus &status);

    QString getOSIcon(const QString &os);
    QString getBrowserIcon(const QString &browser);

    void updateNavButtons();
    void updateFilterButton();

    void processStats(const TeraData& stats);

private slots:
    void processLogsLogins(QList<TeraData> logins, QUrlQuery reply_data);
    void processLogsLogs(QList<TeraData> logins, QUrlQuery reply_data);
    void on_btnFilter_clicked();
    void on_btnNextPage_clicked();
    void on_btnPrevPage_clicked();
    void on_spinPage_editingFinished();
    void on_btnRefresh_clicked();
    void on_cmbLevel_currentIndexChanged(int index);
    void on_dateStartDate_dateChanged(const QDate &date);
    void on_dateEndDate_dateChanged(const QDate &date);
};

#endif // LOGVIEWWIDGET_H
