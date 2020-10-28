#ifndef BASESERVICESETUPWIDGET_H
#define BASESERVICESETUPWIDGET_H

#include <QObject>
#include <QWidget>
#include "ComManager.h"

class BaseServiceSetupWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BaseServiceSetupWidget(ComManager* comManager, QWidget *parent = nullptr);

    virtual QJsonDocument getSetupConfig();

signals:

protected:
    ComManager*             m_comManager;;

};

#endif // BASESERVICESETUPWIDGET_H
