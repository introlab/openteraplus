#include "BaseWizard.h"
#include <QEventLoop>


BaseWizard::BaseWizard(ComManager *comMan)
{
    m_comManager = comMan;

    // Connect signals
    connect(m_comManager, &ComManager::formReceived, this, &BaseWizard::formReceived);

    setupWizard();
}

QWizardPage *BaseWizard::createTeraFormPage(TeraDataTypes form_type)
{
    QWizardPage* page = new QWizardPage();
    page->setTitle(TeraData::getDataTypeNameText(form_type));
    page->setPixmap(QWizard::LogoPixmap, QPixmap(TeraData::getIconFilenameForDataType(form_type)));

    TeraForm *form = new TeraForm();

    // Load form informations from server
    m_comManager->doQuery(WEB_FORMS_PATH, QUrlQuery(WEB_FORMS_QUERY_USER));

    // Synchronisouly wait for server reply
    QEventLoop wait_loop;
    connect(m_comManager, &ComManager::queryResultsOK, &wait_loop, &QEventLoop::quit);
    connect(m_comManager, &ComManager::networkError, &wait_loop, &QEventLoop::quit);
    wait_loop.exec();

    //if (m_lastReceivedFormType == WEB_FORMS_QUERY_USER){
    form->buildUiFromStructure(m_lastReceivedFormData);
    //}

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(form);
    page->setLayout(layout);

    return page;
}

void BaseWizard::setupWizard()
{
    setWizardStyle(WizardStyle::ModernStyle);
    setButtonText(QWizard::NextButton, tr("Suivant"));
    setButtonText(QWizard::BackButton, tr("Précédent"));
    setButtonText(QWizard::CommitButton, tr("Enregistrer"));
    setButtonText(QWizard::FinishButton, tr("Terminer"));
    setButtonText(QWizard::CancelButton, tr("Annuler"));

}


void BaseWizard::formReceived(QString form_type, QString data)
{
    m_lastReceivedFormType = form_type;
    m_lastReceivedFormData = data;
}
