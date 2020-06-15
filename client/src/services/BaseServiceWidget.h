#ifndef BASESERVICEWIDGET_H
#define BASESERVICEWIDGET_H

#include <QWidget>
#include "ComManager.h"

class BaseServiceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BaseServiceWidget(ComManager* comMan, QWidget *parent = nullptr);

protected:
    ComManager* m_comManager;

signals:

};

#endif // BASESERVICEWIDGET_H
