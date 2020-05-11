#ifndef GLOBALEVENT_H
#define GLOBALEVENT_H

#include <QString>
#include <QObject>


typedef enum {
    EVENT_WARNING,
    EVENT_ERROR,
    EVENT_NETWORK,
    EVENT_LOGIN,
    EVENT_LOGOUT,
    EVENT_DATA_EDIT,
    EVENT_DATA_DELETE,
    EVENT_DATA_NEW,
    EVENT_USER_ONLINE,
    EVENT_USER_OFFLINE,
    EVENT_PART_ONLINE,
    EVENT_PART_OFFLINE}
EventType;
Q_DECLARE_METATYPE(EventType)


class GlobalEvent: public QObject
{
    Q_OBJECT
public:

    explicit GlobalEvent(QObject* parent = nullptr);
    explicit GlobalEvent(const EventType& event_type, const QString& msg, QObject* parent = nullptr);
    GlobalEvent(const GlobalEvent& copy, QObject *parent=nullptr);

    void setEvent(const EventType& event_type, const QString& msg);

    static QString getEventTypeDesc(const EventType& event_type);

    GlobalEvent &operator = (const GlobalEvent& other);

    EventType getEventType();
    QString getEventText();

private:
    EventType   m_type;
    QString     m_msg;
};

#endif // GLOBALEVENT_H
