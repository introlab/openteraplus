#ifndef BASEWIZARD_H
#define BASEWIZARD_H

#include <QObject>
#include <QWizard>
#include "managers/ComManager.h"
#include "TeraData.h"
#include "TeraForm.h"

class BaseWizard : public QWizard
{
    Q_OBJECT
public:
    BaseWizard(ComManager *comMan);

protected:
    ComManager*     m_comManager;

    QWizardPage *createTeraFormPage(TeraDataTypes form_type);

private:
    QString m_lastReceivedFormType;
    QString m_lastReceivedFormData;

    void setupWizard();

private slots:
    void formReceived(QString form_type, QString data);

};

#endif // BASEWIZARD_H
