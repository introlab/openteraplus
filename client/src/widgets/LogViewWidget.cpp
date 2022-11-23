#include "LogViewWidget.h"
#include "ui_LogViewWidget.h"

#include <QStyledItemDelegate>

LogViewWidget::LogViewWidget(QWidget *parent):
    QWidget(parent),
    ui(new Ui::LogViewWidget)
{
    ui->setupUi(this);
    ui->cmbLevel->setItemDelegate(new QStyledItemDelegate(ui->cmbLevel));
    m_currentMode = ViewMode::VIEW_LOGS_NONE;
    m_currentUuid = QString();
    m_maxCount = 50; // 50 items at a time by default
    m_filtering = false;

    // Init levels combo box
    int level_id = LogEvent::LogLevel_MIN;
    QStringList names = getLogLevelNames();
    for(const QString &name:qAsConst(names)){
        ui->cmbLevel->addItem(name, level_id++);
        if (level_id == LogEvent::LOGLEVEL_INFO){
            ui->cmbLevel->setCurrentIndex(ui->cmbLevel->count()-1);
        }
    }
}

LogViewWidget::LogViewWidget(ComManager* comMan, QWidget *parent) :
    LogViewWidget::LogViewWidget(parent)
{
    setComManager(comMan);


}

LogViewWidget::~LogViewWidget()
{
    delete ui;
}

void LogViewWidget::setComManager(ComManager *comMan)
{
    m_comManager = comMan;
    connectSignals();
}

void LogViewWidget::setViewMode(const ViewMode &mode, const QString &uuid, const bool &autoload)
{
    m_currentMode = mode;
    m_currentUuid = uuid;

    if (autoload){
        refreshData(true);
    }

}

void LogViewWidget::refreshData(const bool &stats_only)
{
    QUrlQuery args;
    if (stats_only)
        args.addQueryItem(WEB_QUERY_STATS, "1");
    else{
        // Set limiters and offset
        int offset = (ui->spinPage->value()-1) * m_maxCount;
        args.addQueryItem(WEB_QUERY_OFFSET, QString::number(offset));
        args.addQueryItem(WEB_QUERY_LIMIT, QString::number(m_maxCount));
    }

    if (m_filtering){
        // Set start and end date
        args.addQueryItem(WEB_QUERY_START_DATE, ui->dateStartDate->date().toString(Qt::ISODate));
        args.addQueryItem(WEB_QUERY_END_DATE, ui->dateEndDate->date().toString(Qt::ISODate));
        // Log level
        args.addQueryItem(WEB_QUERY_LOG_LEVEL, QString::number(ui->cmbLevel->currentIndex()));
    }

    // Set paths and UUIDs if required
    QString path;
    switch(m_currentMode){
    case VIEW_LOGINS_ALL:
        path = WEB_LOGS_LOGINS_PATH;
        break;
    case VIEW_LOGINS_DEVICE:
        path = WEB_LOGS_LOGINS_PATH;
        args.addQueryItem(WEB_QUERY_UUID_DEVICE, m_currentUuid);
        break;
    case VIEW_LOGINS_PARTICIPANT:
        path = WEB_LOGS_LOGINS_PATH;
        args.addQueryItem(WEB_QUERY_UUID_PARTICIPANT, m_currentUuid);
        break;
    case VIEW_LOGINS_USER:
        path = WEB_LOGS_LOGINS_PATH;
        args.addQueryItem(WEB_QUERY_UUID_USER, m_currentUuid);
        break;
    case VIEW_LOGS_ALL:
        path = WEB_LOGS_LOGS_PATH;
        break;
    case VIEW_LOGS_NONE:
        LOG_WARNING("No view mode selected!", "LogViewWidget");
        return;
        break;
    }
    setEnabled(false);
    m_comManager->doGet(path, args);

}

void LogViewWidget::connectSignals()
{
    if (m_comManager){
        connect(m_comManager, &ComManager::logsLoginReceived, this, &LogViewWidget::processLogsLogins);
        connect(m_comManager, &ComManager::logsLogsReceived, this, &LogViewWidget::processLogsLogs);
    }
}

QStringList LogViewWidget::getLogLevelNames()
{
    QStringList names;
    for(int i=LogEvent::LogLevel_MIN; i<LogEvent::LogLevel_MAX; i++){
        names.append(getLogLevelName(static_cast<LogEvent::LogLevel>(i)));
    }
    return names;
}

QString LogViewWidget::getLogLevelName(const LogEvent::LogLevel &level)
{
    switch(level){
        case opentera::protobuf::LogEvent_LogLevel_LOGLEVEL_UNKNOWN:
            return tr("Inconnu");
        break;
        case opentera::protobuf::LogEvent_LogLevel_LOGLEVEL_TRACE:
            return tr("Trace");
        break;
        case opentera::protobuf::LogEvent_LogLevel_LOGLEVEL_DEBUG:
            return tr("Debug");
        break;
        case opentera::protobuf::LogEvent_LogLevel_LOGLEVEL_INFO:
            return tr("Information");
        break;
        case opentera::protobuf::LogEvent_LogLevel_LOGLEVEL_WARNING:
            return tr("Avertissement");
        break;
        case opentera::protobuf::LogEvent_LogLevel_LOGLEVEL_CRITICAL:
            return tr("Critique");
        break;
        case opentera::protobuf::LogEvent_LogLevel_LOGLEVEL_ERROR:
            return tr("Erreur");
        break;
        case opentera::protobuf::LogEvent_LogLevel_LOGLEVEL_FATAL:
            return tr("Fatal");
        break;
        default:
            return tr("Inconnu");
    }
}

void LogViewWidget::updateNavButtons()
{
    ui->btnPrevPage->setEnabled(ui->spinPage->value() > ui->spinPage->minimum());
    ui->btnNextPage->setEnabled(ui->spinPage->value() < ui->spinPage->maximum());
}

void LogViewWidget::processLogsLogins(QList<TeraData> logins, QUrlQuery reply_data)
{
    setEnabled(true);
}

void LogViewWidget::processLogsLogs(QList<TeraData> logins, QUrlQuery reply_data)
{

    if (reply_data.hasQueryItem(WEB_QUERY_STATS)){
        // We received stats, update display
        if (!logins.empty()){
            int count = logins.first().getFieldValue("count").toInt();
            int page_count = count / m_maxCount + 1;
            ui->lblEntriesCount->setText(QString::number(count));
            ui->frameNav->setVisible(count > m_maxCount);
            ui->spinPage->setValue(1);
            ui->spinPage->setMaximum(page_count);
            ui->lblTotalPages->setText("/" + QString::number(page_count));
            ui->btnPrevPage->setDisabled(true);
            ui->btnNextPage->setEnabled(page_count>1);

            if (ui->dateStartDate->date().year() <= 2000)
                ui->dateStartDate->setDate(logins.first().getFieldValue("min_timestamp").toDate());
            if (ui->dateEndDate->date().year() <= 2000)
                ui->dateEndDate->setDate(logins.first().getFieldValue("max_timestamp").toDate());
            refreshData(false);
            return;
        }else{
            LOG_WARNING("Expected log stats. Received empty reply", "LogViewWidget::processLogsLogs");
        }
    }
    setEnabled(true);
    ui->tableLogs->clearContents();
    ui->tableLogs->setRowCount(logins.count());

    int row = 0;
    for(const TeraData &login:logins){
        QTableWidgetItem* item = new QTableWidgetItem();
        QDateTime log_date = login.getFieldValue("timestamp").toDateTime().toLocalTime();
        item->setText(log_date.toString("dd-MM-yyyy"));
        ui->tableLogs->setItem(row, 0, item);

        item = new QTableWidgetItem();
        item->setText(log_date.toString("hh:mm:ss.zzz"));
        ui->tableLogs->setItem(row, 1, item);

        item = new QTableWidgetItem();
        item->setText(getLogLevelName(static_cast<LogEvent::LogLevel>(login.getFieldValue("log_level").toInt())));
        ui->tableLogs->setItem(row, 2, item);

        item = new QTableWidgetItem();
        item->setText(login.getFieldValue("message").toString());
        ui->tableLogs->setItem(row, 3, item);
        row++;
    }
}

void LogViewWidget::on_btnFilter_clicked()
{
    m_filtering = true;
    refreshData();
}


void LogViewWidget::on_btnNextPage_clicked()
{
    if (ui->spinPage->value() + 1 <= ui->spinPage->maximum()){
        ui->spinPage->setValue(ui->spinPage->value() + 1);
        updateNavButtons();
        refreshData(false);
    }
}


void LogViewWidget::on_btnPrevPage_clicked()
{
    if (ui->spinPage->value() - 1 >= ui->spinPage->minimum()){
        ui->spinPage->setValue(ui->spinPage->value() - 1);
        updateNavButtons();
        refreshData(false);
    }
}


void LogViewWidget::on_spinPage_editingFinished()
{
    updateNavButtons();
    refreshData(false);
}

