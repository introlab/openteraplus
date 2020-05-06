#include "GlobalEventLogger.h"

#include <QDebug>
#include <QDateTime>

// Create static global instance
Q_GLOBAL_STATIC(GlobalEventLogger, globalEventLogger)


GlobalEventLogger::GlobalEventLogger(QObject *parent) : QObject(parent)
{
    m_log_file = nullptr;
    m_logging = false;
}

GlobalEventLogger *GlobalEventLogger::instance()
{
    return globalEventLogger;
}

void GlobalEventLogger::startLogging()
{
    m_logging = true;
}

void GlobalEventLogger::startLogging(const QString &log_path)
{
    if (m_logging){
        qWarning() << "Already logging - stop logging before starting again.";
        return;
    }

    // Create full path for file
    QDir dir;
    dir.mkpath(log_path);

    // Open file for writing
    m_logging = true;
    m_log_file = new QFile(log_path + "/" + QDate::currentDate().toString("yyyy_MM_dd.txt"));
    if (m_log_file->open(QIODevice::Text | QIODevice::Append)){
        m_log_stream.setDevice(m_log_file);
    }else{
        GlobalEvent error_event(EVENT_ERROR, tr("Impossible de créer le fichier de log!"));
        emit newEventLogged(error_event);
    }

}

void GlobalEventLogger::logEvent(GlobalEvent &log_event)
{
    if (m_logging){
        if (m_log_file){
            m_log_stream << QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss.zzz") << '\t' <<
                            GlobalEvent::getEventTypeDesc(log_event.getEventType()) << '\t' <<
                            log_event.getEventText() << "\r\n";
            m_log_stream.flush();

            emit newEventLogged(log_event);
        }
    }
}

void GlobalEventLogger::stopLogging()
{
    if (m_log_file){
        m_log_file->close();
    }
    m_logging = false;
}
