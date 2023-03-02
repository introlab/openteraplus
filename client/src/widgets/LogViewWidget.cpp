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
    m_maxCount = 50; // Number of items displayed by default
    m_filtering = false;
    m_listening = false;

    // Init levels combo box
    int level_id = LogEvent::LogLevel_MIN;
    QStringList names = getLogLevelNames();
    for(const QString &name:qAsConst(names)){
        ui->cmbLevel->addItem(name, level_id++);
        /*if (level_id == LogEvent::LOGLEVEL_INFO){
            ui->cmbLevel->setCurrentIndex(ui->cmbLevel->count()-1);
        }*/
    }
    ui->cmbLevel->setProperty("last_value", 0);
    ui->dateEndDate->setProperty("last_value", ui->dateEndDate->date());
    ui->dateStartDate->setProperty("last_value", ui->dateStartDate->date());
    ui->cmbLevel->setCurrentIndex(0);
    ui->lblWarning->hide();
    ui->tableLogs->hide();
}

LogViewWidget::LogViewWidget(ComManager* comMan, QWidget *parent) :
    LogViewWidget::LogViewWidget(parent)
{
    setComManager(comMan);


}

LogViewWidget::~LogViewWidget()
{
    delete ui;
    qDeleteAll(m_iconsCache);
}

void LogViewWidget::setComManager(ComManager *comMan)
{
    m_comManager = comMan;
    //connectSignals();
}

void LogViewWidget::setViewMode(const ViewMode &mode, const QString &uuid, const bool &autoload)
{
    m_currentMode = mode;
    m_currentUuid = uuid;

    // Adjust columns in display

    // Keep first 3 columns
    while(ui->tableLogs->columnCount()>3){
        ui->tableLogs->removeColumn(ui->tableLogs->columnCount()-1);
    }

    switch(m_currentMode){
    case VIEW_LOGINS_ALL:
    case VIEW_LOGINS_DEVICE:
    case VIEW_LOGINS_PARTICIPANT:
    case VIEW_LOGINS_USER:
    {
        if (m_currentMode == VIEW_LOGINS_ALL){
            ui->tableLogs->insertColumn(ui->tableLogs->columnCount());
            ui->tableLogs->setHorizontalHeaderItem(ui->tableLogs->columnCount()-1, new QTableWidgetItem(tr("Qui")));
        }
        if (m_currentMode == VIEW_LOGINS_DEVICE){
            ui->tableLogs->insertColumn(ui->tableLogs->columnCount());
            ui->tableLogs->setHorizontalHeaderItem(ui->tableLogs->columnCount()-1, new QTableWidgetItem(tr("Appareil")));
        }
        if (m_currentMode == VIEW_LOGINS_PARTICIPANT){
            ui->tableLogs->insertColumn(ui->tableLogs->columnCount());
            ui->tableLogs->setHorizontalHeaderItem(ui->tableLogs->columnCount()-1, new QTableWidgetItem(tr("Participant")));
        }
        if (m_currentMode == VIEW_LOGINS_USER){
            ui->tableLogs->insertColumn(ui->tableLogs->columnCount());
            ui->tableLogs->setHorizontalHeaderItem(ui->tableLogs->columnCount()-1, new QTableWidgetItem(tr("Utilisateur")));
        }

        ui->tableLogs->insertColumn(ui->tableLogs->columnCount());
        ui->tableLogs->setHorizontalHeaderItem(ui->tableLogs->columnCount()-1, new QTableWidgetItem(tr("Type")));

        ui->tableLogs->insertColumn(ui->tableLogs->columnCount());
        ui->tableLogs->setHorizontalHeaderItem(ui->tableLogs->columnCount()-1, new QTableWidgetItem(tr("État")));

        ui->tableLogs->insertColumn(ui->tableLogs->columnCount());
        ui->tableLogs->setHorizontalHeaderItem(ui->tableLogs->columnCount()-1, new QTableWidgetItem(tr("Système")));

        ui->tableLogs->insertColumn(ui->tableLogs->columnCount());
        ui->tableLogs->setHorizontalHeaderItem(ui->tableLogs->columnCount()-1, new QTableWidgetItem(tr("Client")));

        ui->tableLogs->insertColumn(ui->tableLogs->columnCount());
        ui->tableLogs->setHorizontalHeaderItem(ui->tableLogs->columnCount()-1, new QTableWidgetItem(tr("Ressource")));

        ui->tableLogs->insertColumn(ui->tableLogs->columnCount());
        ui->tableLogs->setHorizontalHeaderItem(ui->tableLogs->columnCount()-1, new QTableWidgetItem(tr("Message")));
    }
        break;
    case VIEW_LOGS_ALL:
        ui->tableLogs->insertColumn(ui->tableLogs->columnCount());
        ui->tableLogs->setHorizontalHeaderItem(ui->tableLogs->columnCount()-1, new QTableWidgetItem(tr("Message")));
        break;
    default:
        LOG_WARNING("No view mode or unmanaged mode selected!", "LogViewWidget::setViewMode");
        return;
        break;
    }

    //connectSignals();

    if (autoload){
        refreshData(true);
    }

}

void LogViewWidget::setUuidName(const QString &uuid, const QString &name)
{
    m_uuidsNames[uuid] = name;
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
        args.addQueryItem(WEB_QUERY_START_DATE, ui->dateStartDate->property("last_value").toDate().toString(Qt::ISODate));
        args.addQueryItem(WEB_QUERY_END_DATE, ui->dateEndDate->property("last_value").toDateTime().addSecs(23*3600 + 59*60 + 59).addMSecs(999).toString(Qt::ISODate)); // At 23:59:59
        // Log level
        args.addQueryItem(WEB_QUERY_LOG_LEVEL, QString::number(ui->cmbLevel->itemData(ui->cmbLevel->property("last_value").toInt()).toInt()));
    }

    // Set paths and UUIDs if required
    QString path;
    switch(m_currentMode){
    case VIEW_LOGINS_ALL:
        path = WEB_LOGS_LOGINS_PATH;
        args.addQueryItem(WEB_QUERY_WITH_NAMES, "1");
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
    connectSignals();
    m_comManager->doGet(path, args);

    // TODO: Keep args to check if the response we get is really for us!

}

void LogViewWidget::connectSignals()
{
    if (m_comManager && m_currentMode != VIEW_LOGS_NONE && !m_listening){
        connect(m_comManager, &ComManager::logsLoginReceived, this, &LogViewWidget::processLogsLogins);
        connect(m_comManager, &ComManager::logsLogsReceived, this, &LogViewWidget::processLogsLogs);
    }
}



void LogViewWidget::disconnectSignals()
{
    if (m_comManager)
        disconnect(m_comManager, nullptr, this, nullptr);
    m_listening = false;
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
            return tr("Tous");
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

QString LogViewWidget::getLogLevelIcon(const LogEvent::LogLevel &level)
{
    switch(level){
        case opentera::protobuf::LogEvent_LogLevel_LOGLEVEL_UNKNOWN:
            return "";
        break;
        case opentera::protobuf::LogEvent_LogLevel_LOGLEVEL_TRACE:
        case opentera::protobuf::LogEvent_LogLevel_LOGLEVEL_DEBUG:
            return "://icons/search.png";
        break;
        case opentera::protobuf::LogEvent_LogLevel_LOGLEVEL_INFO:
            return "://icons/info.png";
        break;
        case opentera::protobuf::LogEvent_LogLevel_LOGLEVEL_WARNING:
            return "://status/warning.png";
        break;
        case opentera::protobuf::LogEvent_LogLevel_LOGLEVEL_FATAL:
        case opentera::protobuf::LogEvent_LogLevel_LOGLEVEL_CRITICAL:
            return "://icons/error2.png";
        break;
        case opentera::protobuf::LogEvent_LogLevel_LOGLEVEL_ERROR:
            return "://icons/error.png";
        break;
        default:
            return "";
    }
}

QString LogViewWidget::getLoginTypeName(const LoginEvent::LoginType &login_type)
{
    switch(login_type){
        case LoginEvent_LoginType_LOGIN_TYPE_TOKEN:
            return tr("Jeton");
        break;
        case LoginEvent_LoginType_LOGIN_TYPE_PASSWORD:
            return tr("Mot de passe");
        break;
        case LoginEvent_LoginType_LOGIN_TYPE_CERTIFICATE:
            return tr("Certificat");
        break;
        case LoginEvent_LoginType_LOGIN_TYPE_UNKNOWN:
        default:
            return tr("Inconnu");
        break;
    }
}

QString LogViewWidget::getLoginTypeIcon(const LoginEvent::LoginType &login_type)
{
    switch(login_type){
        case LoginEvent_LoginType_LOGIN_TYPE_TOKEN:
            return "://icons/logs/token.png";
        break;
        case LoginEvent_LoginType_LOGIN_TYPE_PASSWORD:
            return "://icons/logs/password.png";
        break;
        case LoginEvent_LoginType_LOGIN_TYPE_CERTIFICATE:
            return "://icons/logs/certificate.png";
        break;
        case LoginEvent_LoginType_LOGIN_TYPE_UNKNOWN:
        default:
            return "://icons/question.png";
        break;
    }
}

QString LogViewWidget::getLoginStatusName(const LoginEvent::LoginStatus &status)
{
    switch(status){
        case LoginEvent_LoginStatus_LOGIN_STATUS_SUCCESS:
            return tr("Succès");
        break;
        case LoginEvent_LoginStatus_LOGIN_STATUS_FAILED_WITH_WRONG_PASSWORD:
            return tr("Mauvais mot de passe");
        break;
        case LoginEvent_LoginStatus_LOGIN_STATUS_FAILED_WITH_WRONG_USERNAME:
            return tr("Mauvais code utilisateur");
        break;
        case LoginEvent_LoginStatus_LOGIN_STATUS_FAILED_WITH_MAX_ATTEMPTS_REACHED:
            return tr("Nombre maximum d'essais atteint");
        break;
        case LoginEvent_LoginStatus_LOGIN_STATUS_FAILED_WITH_ALREADY_LOGGED_IN:
            return tr("Déjà connecté");
        break;
        case LoginEvent_LoginStatus_LOGIN_STATUS_FAILED_WITH_INVALID_TOKEN:
            return tr("Jeton invalide");
        break;
        case LoginEvent_LoginStatus_LOGIN_STATUS_FAILED_WITH_DISABLED_ACCOUNT:
            return tr("Compte désactivé");
        break;
        case LoginEvent_LoginStatus_LOGIN_STATUS_FAILED_WITH_EXPIRED_TOKEN:
            return tr("Jeton expiré");
        break;
        case LoginEvent_LoginStatus_LOGIN_STATUS_UNKNOWN:
        default:
            return tr("Inconnu");
        break;
    }
}

QString LogViewWidget::getLoginStatusIcon(const LoginEvent::LoginStatus &status)
{
    switch(status){
        case LoginEvent_LoginStatus_LOGIN_STATUS_SUCCESS:
            return "://status/status_ok.png";
        break;
        case LoginEvent_LoginStatus_LOGIN_STATUS_FAILED_WITH_WRONG_PASSWORD:
        case LoginEvent_LoginStatus_LOGIN_STATUS_FAILED_WITH_WRONG_USERNAME:
        case LoginEvent_LoginStatus_LOGIN_STATUS_FAILED_WITH_INVALID_TOKEN:
        case LoginEvent_LoginStatus_LOGIN_STATUS_FAILED_WITH_EXPIRED_TOKEN:
            return "://status/status_notok.png";
        break;
        case LoginEvent_LoginStatus_LOGIN_STATUS_FAILED_WITH_MAX_ATTEMPTS_REACHED:
        case LoginEvent_LoginStatus_LOGIN_STATUS_FAILED_WITH_ALREADY_LOGGED_IN:
        case LoginEvent_LoginStatus_LOGIN_STATUS_FAILED_WITH_DISABLED_ACCOUNT:
            return "://status/status_incomplete.png";
        break;
        case LoginEvent_LoginStatus_LOGIN_STATUS_UNKNOWN:
        default:
            return "://icons/question.png";
        break;
    }
}

QString LogViewWidget::getOSIcon(const QString &os)
{
    QString compare_os = os.toLower();
    if (compare_os.contains("windows"))
        return "://icons/logs/os_windows.png";

    if (compare_os.contains("apple") || compare_os.contains("mac"))
        return "://icons/logs/os_mac.png";

    if (compare_os.contains("linux") || compare_os.contains("ubuntu"))
        return "://icons/logs/os_linux.png";

    if (compare_os.contains("ios"))
        return "://icons/logs/os_ios.png";

    if (compare_os.contains("android"))
        return "://icons/logs/os_android.png";

    return "";

}

QString LogViewWidget::getBrowserIcon(const QString &browser)
{
    QString compare_browser = browser.toLower();
    if (compare_browser.contains("chrome") || compare_browser.contains("chromium"))
        return "://icons/logs/browser_chrome.png";

    if (compare_browser.contains("firefox"))
        return "://icons/logs/browser_firefox.png";

    if (compare_browser.contains("safari"))
        return "://icons/logs/browser_safari.png";

    if (compare_browser.contains("openteraplus"))
        return "://icons/OpenTeraPlus.ico";

    return "";
}

const QIcon *LogViewWidget::getIcon(const QString &path)
{
    if (m_iconsCache.contains(path))
        return m_iconsCache[path];

    QIcon* icon = new QIcon(path);
    m_iconsCache.insert(path, icon);
    return icon;
}

void LogViewWidget::updateNavButtons()
{
    ui->btnPrevPage->setEnabled(ui->spinPage->value() > ui->spinPage->minimum());
    ui->btnNextPage->setEnabled(ui->spinPage->value() < ui->spinPage->maximum());
}

void LogViewWidget::updateFilterButton()
{
    ui->btnFilter->setEnabled(ui->cmbLevel->property("last_value").toInt() != ui->cmbLevel->currentIndex()
            || ui->dateStartDate->property("last_value").toDate() != ui->dateStartDate->date()
            || ui->dateEndDate->property("last_value").toDate() != ui->dateEndDate->date());
}

void LogViewWidget::processStats(const TeraData &stats)
{
    int count = stats.getFieldValue("count").toInt();
    int page_count = count / m_maxCount + 1;
    ui->lblEntriesCount->setText(QString::number(count));
    ui->frameNav->setVisible(count > m_maxCount);
    ui->tableLogs->setVisible(count>0);
    ui->lblWarning->setVisible(count==0);
    ui->spinPage->setValue(1);
    ui->spinPage->setMaximum(page_count);
    ui->lblTotalPages->setText("/" + QString::number(page_count));
    ui->btnPrevPage->setDisabled(true);
    ui->btnNextPage->setEnabled(page_count>1);

    QDate min_date = stats.getFieldValue("min_timestamp").toDate();
    QDate max_date = stats.getFieldValue("max_timestamp").toDate();

    if (ui->dateStartDate->date().year() <= 2000){
        ui->dateStartDate->setProperty("last_value", min_date);
        ui->dateStartDate->setDate(min_date);
        ui->dateEndDate->setProperty("last_value", max_date);
        ui->dateEndDate->setDate(max_date);
    }
    refreshData(false);
}

void LogViewWidget::processLogsLogins(QList<TeraData> logins, QUrlQuery reply_data)
{
    if (reply_data.hasQueryItem(WEB_QUERY_STATS)){
        // We received stats, update display
        if (!logins.empty()){
            processStats(logins.first());
            return;
        }else{
            LOG_WARNING("Expected log stats. Received empty reply", "LogViewWidget::processLogsLogins");
        }
    }
    setEnabled(true);
    ui->tableLogs->clearContents();
    ui->tableLogs->setRowCount(logins.count());
    int row = 0;
    for(const TeraData &login:logins){
        QTableWidgetItem* item = new QTableWidgetItem();
        QDateTime log_date = login.getFieldValue("login_timestamp").toDateTime().toLocalTime();
        item->setText(log_date.toString("dd-MM-yyyy"));
        ui->tableLogs->setItem(row, 0, item);

        item = new QTableWidgetItem();
        item->setText(log_date.toString("hh:mm:ss.zzz"));
        ui->tableLogs->setItem(row, 1, item);

        item = new QTableWidgetItem();
        LogEvent::LogLevel level = static_cast<LogEvent::LogLevel>(login.getFieldValue("login_log_level").toInt());
        item->setText(getLogLevelName(level));
        item->setIcon(*getIcon((getLogLevelIcon(level))));
        ui->tableLogs->setItem(row, 2, item);

        // Try to find the name associated to that log
        QString login_name;
        QString login_icon = "";
        if (login.hasFieldName("login_user_name")){
            QString name = login.getFieldValue("login_user_name").toString();
            if (!name.isEmpty()){
                login_name = /*tr("Utilisateur") + ": " +*/ name;
                login_icon = TeraData::getIconFilenameForDataType(TeraDataTypes::TERADATA_USER);
            }
        }
        if (login.hasFieldName("login_participant_name") && login_name.isEmpty()){
            QString name = login.getFieldValue("login_participant_name").toString();
            if (!name.isEmpty()){
                login_name = /*tr("Participant") + ": " +*/ name;
                login_icon = TeraData::getIconFilenameForDataType(TeraDataTypes::TERADATA_PARTICIPANT);
            }
        }
        if (login.hasFieldName("login_device_name") && login_name.isEmpty()){
            QString name = login.getFieldValue("login_device_name").toString();
            if (!name.isEmpty()){
                login_name = /*tr("Appareil") + ": " + */name;
                login_icon = TeraData::getIconFilenameForDataType(TeraDataTypes::TERADATA_DEVICE);
            }
        }
        if (login.hasFieldName("login_service_name") && login_name.isEmpty()){
            QString name = login.getFieldValue("login_service_name").toString();
            if (!name.isEmpty()){
                login_name = /*tr("Service") + ": " +*/ name;
                login_icon = TeraData::getIconFilenameForDataType(TeraDataTypes::TERADATA_SERVICE);
            }
        }

        if (login_name.isEmpty()){
            QString user_login_uuid = login.getFieldValue("login_user_uuid").toString();
            QString part_login_uuid = login.getFieldValue("login_participant_uuid").toString();
            QString device_login_uuid = login.getFieldValue("login_device_uuid").toString();
            QString service_login_uuid = login.getFieldValue("login_service_uuid").toString();

            if (m_uuidsNames.contains(user_login_uuid)){
                login_name = m_uuidsNames[user_login_uuid];
                login_icon = TeraData::getIconFilenameForDataType(TeraDataTypes::TERADATA_USER);
            }
            else if (m_uuidsNames.contains(part_login_uuid)){
                login_name = m_uuidsNames[part_login_uuid];
                login_icon = TeraData::getIconFilenameForDataType(TeraDataTypes::TERADATA_PARTICIPANT);
            }
            else if (m_uuidsNames.contains(device_login_uuid)){
                login_name = m_uuidsNames[device_login_uuid];
                login_icon = TeraData::getIconFilenameForDataType(TeraDataTypes::TERADATA_DEVICE);
            }
            else if (m_uuidsNames.contains(service_login_uuid)){
                login_name = m_uuidsNames[service_login_uuid];
                login_icon = TeraData::getIconFilenameForDataType(TeraDataTypes::TERADATA_SERVICE);
            }
        }

        item = new QTableWidgetItem();
        item->setText(login_name);
        item->setIcon(*getIcon(login_icon));
        ui->tableLogs->setItem(row, 3, item);

        item = new QTableWidgetItem();
        LoginEvent::LoginType login_type = static_cast<LoginEvent::LoginType>(login.getFieldValue("login_type").toInt());
        item->setText(getLoginTypeName(login_type));
        item->setIcon(*getIcon(getLoginTypeIcon(login_type)));
        ui->tableLogs->setItem(row, 4, item);

        item = new QTableWidgetItem();
        LoginEvent::LoginStatus login_status = static_cast<LoginEvent::LoginStatus>(login.getFieldValue("login_status").toInt());
        item->setText(getLoginStatusName(login_status));
        item->setIcon(*getIcon(getLoginStatusIcon(login_status)));
        ui->tableLogs->setItem(row, 5, item);

        item = new QTableWidgetItem();
        item->setText(login.getFieldValue("login_os_name").toString() + " " + login.getFieldValue("login_os_version").toString());
        item->setIcon(*getIcon(getOSIcon(login.getFieldValue("login_os_name").toString())));
        ui->tableLogs->setItem(row, 6, item);

        item = new QTableWidgetItem();
        item->setText(login.getFieldValue("login_client_name").toString() + " " + login.getFieldValue("login_client_version").toString());
        item->setIcon(*getIcon(getBrowserIcon(login.getFieldValue("login_client_name").toString())));
        ui->tableLogs->setItem(row, 7, item);

        item = new QTableWidgetItem();
        item->setText(login.getFieldValue("login_server_endpoint").toString());
        ui->tableLogs->setItem(row, 8, item);

        item = new QTableWidgetItem();
        item->setText(login.getFieldValue("login_message").toString());
        ui->tableLogs->setItem(row, 9, item);
        row++;
    }

    ui->tableLogs->resizeColumnsToContents();
    disconnectSignals();

}

void LogViewWidget::processLogsLogs(QList<TeraData> logins, QUrlQuery reply_data)
{
    if (reply_data.hasQueryItem(WEB_QUERY_STATS)){
        // We received stats, update display
        if (!logins.empty()){
            processStats(logins.first());
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
        LogEvent::LogLevel level = static_cast<LogEvent::LogLevel>(login.getFieldValue("log_level").toInt());
        item->setText(getLogLevelName(level));
        item->setIcon(*getIcon(getLogLevelIcon(level)));
        ui->tableLogs->setItem(row, 2, item);

        item = new QTableWidgetItem();
        item->setText(login.getFieldValue("message").toString());
        ui->tableLogs->setItem(row, 3, item);
        row++;
    }

    ui->tableLogs->resizeColumnsToContents();
    disconnectSignals();
}

void LogViewWidget::on_btnFilter_clicked()
{
    m_filtering = true;
    ui->cmbLevel->setProperty("last_value", ui->cmbLevel->currentIndex());
    ui->dateEndDate->setProperty("last_value", ui->dateEndDate->date());
    ui->dateStartDate->setProperty("last_value", ui->dateStartDate->date());
    updateFilterButton();
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

void LogViewWidget::on_btnRefresh_clicked()
{
    refreshData(false);
}


void LogViewWidget::on_cmbLevel_currentIndexChanged(int index)
{
    updateFilterButton();
}


void LogViewWidget::on_dateStartDate_dateChanged(const QDate &date)
{
    ui->dateEndDate->setMinimumDate(date);
    updateFilterButton();
}


void LogViewWidget::on_dateEndDate_dateChanged(const QDate &date)
{
    updateFilterButton();
}

