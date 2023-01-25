#ifndef BASESERVICEWIDGET_H
#define BASESERVICEWIDGET_H

#include <QWidget>
#include "managers/ComManager.h"

class BaseServiceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BaseServiceWidget(ComManager* comMan, QWidget *parent = nullptr);
    ~BaseServiceWidget();

    void setSession(const TeraData* session);

    virtual bool handleJoinSessionEvent(const JoinSessionEvent &event);

    virtual void setDataSavePath();

    static QStringList getHandledServiceKeys();
    static QList<TeraSessionCategory::SessionTypeCategories> getHandledSessionCategories();

protected:
    ComManager*     m_comManager;

    TeraData*       m_session;

signals:

    void widgetIsReady(bool ready_state);
};

#endif // BASESERVICEWIDGET_H
