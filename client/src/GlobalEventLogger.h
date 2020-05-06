#ifndef GLOBALEVENTLOGGER_H
#define GLOBALEVENTLOGGER_H

#include <QObject>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include "GlobalEvent.h"

class GlobalEventLogger : public QObject
{
    Q_OBJECT
public:
    explicit GlobalEventLogger(QObject *parent = nullptr);

    static GlobalEventLogger* instance();

    void startLogging();
    void startLogging(const QString &log_path);
    void logEvent(GlobalEvent &log_event);
    void stopLogging();

signals:
    void newEventLogged(GlobalEvent log_event);

private:
    QFile*      m_log_file;
    QTextStream m_log_stream;
    bool        m_logging;


};

#endif // GLOBALEVENTLOGGER_H
