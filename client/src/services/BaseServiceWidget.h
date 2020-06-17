#ifndef BASESERVICEWIDGET_H
#define BASESERVICEWIDGET_H

#include <QWidget>
#include "ComManager.h"

class BaseServiceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BaseServiceWidget(ComManager* comMan, QWidget *parent = nullptr);

    virtual bool handleJoinSessionEvent(const JoinSessionEvent &event);

protected:
    ComManager* m_comManager;

signals:

};

#endif // BASESERVICEWIDGET_H
