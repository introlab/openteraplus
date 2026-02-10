#include "MOVEConfigWidget.h"
#include "ui_MOVEConfigWidget.h"

#include "MOVEWebAPI.h"
#include "WebAPI.h"

MOVEConfigWidget::MOVEConfigWidget(ComManager *comManager, int projectId, QString participantUuid, QString participantName, QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::MOVEConfigWidget),
    m_comManager(comManager),
    m_idProject(projectId),
    m_uuidParticipant(participantUuid),
    m_participantName(participantName)
{
    m_ui->setupUi(this);

    // NOTE At this state, comManager is not ready, will need to get service information before and then change state to ready.
    m_MOVEComManager = new MOVEComManager(comManager);


    m_ui->tabMain->setCurrentIndex(0);
    m_ui->lblWarning->hide();
    m_ui->wdgMessages->hide();

    // Generate a new empty profile
    m_participantProfile = QJsonObject();
    // Add context and uuid
    m_participantProfile.insert("participant_context", "");
    m_participantProfile.insert("participant_uuid", m_uuidParticipant);
    m_participantProfile.insert("participant_name", participantName);
    m_participantProfile.insert("id_participant_profile", 0);

    m_ui->frameContextUpdater->setEnabled(false);

    connectSignals();
}

MOVEConfigWidget::~MOVEConfigWidget()
{
    delete m_ui;
    delete m_MOVEComManager;
}

void MOVEConfigWidget::connectSignals()
{
    connect(m_MOVEComManager, &MOVEComManager::participantProfileReceived, this, &MOVEConfigWidget::participantProfileReceived);
    connect(m_MOVEComManager, &MOVEComManager::participantProfileNotFound, this, &MOVEConfigWidget::participantProfileNotFound);
    connect(m_MOVEComManager, &MOVEComManager::readyChanged, this, &MOVEConfigWidget::serviceReadyChanged);
    connect(m_ui->wdgMessages, &ResultMessageWidget::nextMessageShown, this, &MOVEConfigWidget::nextMessageWasShown);
}

void MOVEConfigWidget::participantProfileReceived(QJsonObject user_profile, QUrlQuery reply_query)
{
    if (user_profile.contains("participant_context"))
    {
        m_ui->frameContextUpdater->setEnabled(true);
        updateProfile(user_profile);
        m_ui->wdgMessages->addMessage(Message(Message::MESSAGE_OK, tr("Le profil a été chargé avec succès.")));
    }
}

void MOVEConfigWidget::serviceReadyChanged(bool ready)
{
    if(ready)
    {
        // Try to get the profile
        m_MOVEComManager->queryParticipantProfile(m_uuidParticipant);

        // Connect the update button
        connect(m_ui->btnSave, &QPushButton::clicked, this, &MOVEConfigWidget::onSaveParticipantProfileButtonClicked);

        // Connect change from context text edit
        connect(m_ui->textEdit, &QTextEdit::textChanged, this, [this]() {
            m_participantProfile["participant_context"] = m_ui->textEdit->toPlainText();
        });

        // Connect change from participant name line edit
        connect(m_ui->lineEditParticipantName, &QLineEdit::textChanged, this, [this](const QString &newName) {
            m_participantProfile["participant_name"] = newName;
        });

        // Connect change from participant location line edit
        connect(m_ui->lineEditParticipantLocation, &QLineEdit::textChanged, this, [this](const QString &newLocation) {
            m_participantProfile["participant_location"] = newLocation;
        });

    }
}

void MOVEConfigWidget::updateProfile(const QJsonObject &profile_data)
{
    // Store current profile
    m_participantProfile = profile_data;

    // Get Participant Name
    m_participantName = m_participantProfile.value("participant_name").toString();
    m_ui->lineEditParticipantName->setText(m_participantName);

    // Get Participant Location
    m_participantLocation = m_participantProfile.value("participant_location").toString();
    m_ui->lineEditParticipantLocation->setText(m_participantLocation);

    // Get Context
    QString context = m_participantProfile.value("participant_context").toString();

    // Update textedit
    m_ui->textEdit->setText(context);
}

void MOVEConfigWidget::onSaveParticipantProfileButtonClicked()
{
    // Send the updated profile
    m_MOVEComManager->updateParticipantProfile(m_participantProfile);
}

void MOVEConfigWidget::participantProfileNotFound(const QString &path, int status_code, const QString &error_str)
{
    // Create a participant profile if not found
    m_ui->wdgMessages->addMessage(Message(Message::MESSAGE_WORKING, tr("Création d'un nouveau profil.")));
    // Send the initial profile
    m_MOVEComManager->updateParticipantProfile(m_participantProfile);
}

void MOVEConfigWidget::nextMessageWasShown(Message current_message)
{
    if (current_message.getMessageType()==Message::MESSAGE_NONE)
    {
        m_ui->wdgMessages->hide();
    }
    else
    {
        m_ui->wdgMessages->show();
    }
}
