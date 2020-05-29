#ifndef USERWIZARD_H
#define USERWIZARD_H

#include <QObject>
#include "BaseWizard.h"

class UserWizard : public BaseWizard
{
    Q_OBJECT
public:
    UserWizard(ComManager* comMan);

private:
    void initPages();

};

#endif // USERWIZARD_H
