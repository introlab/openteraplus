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

    ui->dateEndDate->setDate(QDate::currentDate());
    ui->dateStartDate->setDate(QDate::currentDate().addMonths(-6));

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
        refreshData();
    }

}

void LogViewWidget::refreshData()
{
    switch(m_currentMode){
    case VIEW_LOGINS_ALL:
        queryAllLogins();
        break;
    case VIEW_LOGINS_DEVICE:
        queryLoginsForDevice();
        break;
    case VIEW_LOGINS_PARTICIPANT:
        queryLoginsForParticipant();
        break;
    case VIEW_LOGINS_USER:
        queryLoginsForUser();
        break;
    case VIEW_LOGS_ALL:
        queryAllLogs();
        break;
    case VIEW_LOGS_NONE:
        LOG_WARNING("No view mode selected!", "LogViewWidget");
        break;
    }
}

void LogViewWidget::queryLoginsForUser()
{
    if (m_currentUuid.isEmpty()){
        LOG_WARNING("No UUID was set!", "LogViewWidget::queryLoginsForUser");
        return;
    }

    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_UUID_USER, m_currentUuid);

    if (m_comManager){
        m_comManager->doGet(WEB_LOGS_LOGINS_PATH, args);
    }else{
        LOG_WARNING("No ComManager was set!", "LogViewWidget");
    }
}

void LogViewWidget::queryLoginsForParticipant()
{
    if (m_currentUuid.isEmpty()){
        LOG_WARNING("No UUID was set!", "LogViewWidget::queryLoginsForParticipant");
        return;
    }
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_UUID_PARTICIPANT, m_currentUuid);

    if (m_comManager){
        m_comManager->doGet(WEB_LOGS_LOGINS_PATH, args);
    }else{
        LOG_WARNING("No ComManager was set!", "LogViewWidget");
    }
}

void LogViewWidget::queryLoginsForDevice()
{
    if (m_currentUuid.isEmpty()){
        LOG_WARNING("No UUID was set!", "LogViewWidget::queryLoginsForDevice");
        return;
    }
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_UUID_DEVICE, m_currentUuid);

    if (m_comManager){
        m_comManager->doGet(WEB_LOGS_LOGINS_PATH, args);
    }else{
        LOG_WARNING("No ComManager was set!", "LogViewWidget");
    }
}

void LogViewWidget::queryAllLogins()
{
    if (m_comManager){
        m_comManager->doGet(WEB_LOGS_LOGINS_PATH);
    }else{
        LOG_WARNING("No ComManager was set!", "LogViewWidget");
    }
}

void LogViewWidget::queryAllLogs()
{
    if (m_comManager){
        m_comManager->doGet(WEB_LOGS_LOGS_PATH);
    }else{
        LOG_WARNING("No ComManager was set!", "LogViewWidget");
    }
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

void LogViewWidget::processLogsLogins(QList<TeraData> logins, QUrlQuery reply_data)
{

}

void LogViewWidget::processLogsLogs(QList<TeraData> logins, QUrlQuery reply_data)
{
    ui->tableLogs->clearContents();
    ui->tableLogs->setRowCount(logins.count());

    int row = 0;
    for(const TeraData &login:logins){
        QTableWidgetItem* item = new QTableWidgetItem();
        QDateTime log_date = QDateTime::fromMSecsSinceEpoch(login.getFieldValue("timestamp").toULongLong());
        item->setText(log_date.toString("dd-MM-yyyy"));
        ui->tableLogs->setItem(row, 0, item);

        item = new QTableWidgetItem();
        item->setText(log_date.toString("hh:mm:ss.zzz"));
        ui->tableLogs->setItem(row, 1, item);

        item = new QTableWidgetItem();
        item->setText(getLogLevelName(static_cast<LogEvent::LogLevel>(login.getFieldValue("level").toInt())));
        ui->tableLogs->setItem(row, 2, item);

        item = new QTableWidgetItem();
        item->setText(login.getFieldValue("message").toString());
        ui->tableLogs->setItem(row, 3, item);
        row++;
    }
}

void LogViewWidget::on_btnFilter_clicked()
{
   refreshData();
}

