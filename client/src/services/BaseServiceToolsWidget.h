#ifndef BASESERVICETOOLSWIDGET_H
#define BASESERVICETOOLSWIDGET_H

#include <QWidget>
#include "BaseServiceWidget.h"
#include "ComManager.h"

class BaseServiceToolsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BaseServiceToolsWidget(ComManager* comMan, BaseServiceWidget* baseWidget, QWidget *parent = nullptr);


    virtual bool sessionCanBeEnded();

public slots:
    virtual void setReadyState(bool ready_state);


protected:
    ComManager*         m_comManager;
    BaseServiceWidget*  m_baseWidget;
};

#endif // BASESERVICETOOLSWIDGET_H
