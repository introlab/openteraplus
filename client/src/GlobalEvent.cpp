#include "GlobalEvent.h"
#include <QVariant>

GlobalEvent::GlobalEvent(QObject *parent):
    QObject(parent)
{

}

GlobalEvent::GlobalEvent(const EventType &event_type, const QString &msg, QObject* parent):
    QObject(parent)
{
    setEvent(event_type, msg);
}

GlobalEvent::GlobalEvent(const GlobalEvent &copy, QObject *parent):
    QObject(parent)
{
    *this = copy;
}

void GlobalEvent::setEvent(const EventType &event_type, const QString &msg)
{
    m_msg = msg;
    m_type = event_type;
}

QString GlobalEvent::getEventTypeDesc(const EventType &event_type)
{
    return QVariant::fromValue(event_type).toString();
}

GlobalEvent &GlobalEvent::operator =(const GlobalEvent &other)
{
    this->m_msg = other.m_msg;
    this->m_type = other.m_type;

    return *this;
}

EventType GlobalEvent::getEventType()
{
    return m_type;
}

QString GlobalEvent::getEventText()
{
    return m_msg;
}
