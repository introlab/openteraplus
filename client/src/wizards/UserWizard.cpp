#include "UserWizard.h"

UserWizard::UserWizard(ComManager* comMan) : BaseWizard(comMan)
{
    // Set wizard titles
    setWindowTitle(tr("Nouvel utilisateur"));

    // Initialize pages
    initPages();

}

void UserWizard::initPages()
{
    addPage(createTeraFormPage(TERADATA_USER)); // User info page

}
