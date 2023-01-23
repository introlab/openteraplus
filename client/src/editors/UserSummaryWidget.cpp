#include "UserSummaryWidget.h"
#include "ui_UserSummaryWidget.h"

#include "dialogs/PasswordStrengthDialog.h"

#include <QLocale>

UserSummaryWidget::UserSummaryWidget(ComManager *comMan, const TeraData *data, const int &id_project, QWidget *parent) :
    DataEditorWidget(comMan, data, parent),
    ui(new Ui::UserSummaryWidget)
{

    m_diag_editor = nullptr;
    m_sessionLobby = nullptr;
    m_passwordJustGenerated = false;
    m_idProject = id_project;

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground); //Required to set a background image
    setLimited(true);

    // Use base class to manage editing
    setEditorControls(ui->wdgUser, ui->btnEdit, ui->frameButtons, ui->btnSave, ui->btnUndo);

    // Initialize user interface
    initUI();

    // Connect signals and slots
    connectSignals();

    // Query form definition
    QUrlQuery args(WEB_FORMS_QUERY_USER);
    if (!dataIsNew())
        args.addQueryItem(WEB_QUERY_ID, QString::number(m_data->getId()));

    queryDataRequest(WEB_FORMS_PATH, args);

    // Query sites related to that user (to check for edit access)
    args.clear();
    args.addQueryItem(WEB_QUERY_ID_USER, QString::number(m_data->getId()));
    queryDataRequest(WEB_SITEACCESS_PATH, args);

    // Query stats
    queryDataRequest(WEB_STATS_PATH, args); // args already contains id_user

    ui->wdgUser->setComManager(m_comManager);
}

UserSummaryWidget::~UserSummaryWidget()
{
    delete ui;

    if (m_sessionLobby)
        m_sessionLobby->deleteLater();
}

void UserSummaryWidget::connectSignals()
{
    connect(m_comManager, &ComManager::formReceived, this, &UserSummaryWidget::processFormsReply);
    connect(m_comManager, &ComManager::sessionTypesReceived, this, &UserSummaryWidget::processSessionTypesReply);
    connect(m_comManager, &ComManager::siteAccessReceived, this, &UserSummaryWidget::processSiteAccessReply);
    connect(m_comManager, &ComManager::statsReceived, this, &UserSummaryWidget::processStatsReply);

    connect(m_comManager->getWebSocketManager(), &WebSocketManager::userEventReceived, this, &UserSummaryWidget::ws_userEvent);

    connect(ui->wdgUser,  &TeraForm::widgetValueHasChanged, this, &UserSummaryWidget::userFormValueChanged);
    connect(ui->wdgUser,  &TeraForm::widgetValueHasFocus, this, &UserSummaryWidget::userFormValueHasFocus);

}

void UserSummaryWidget::updateControlsState()
{
    ui->frameEdit->setVisible(!m_limited);
    ui->wdgSessions->enableDeletion(!m_limited);
}

void UserSummaryWidget::updateFieldsValue()
{
    if (m_data){
        ui->lblTitle->setText(m_data->getName());

        // Status
        ui->icoOnline->setVisible(m_data->isEnabled());

        ui->icoTitle->setPixmap(QPixmap(m_data->getIconStateFilename()));
        if (m_data->isOnline() || m_data->isBusy()){
            ui->icoOnline->setPixmap(QPixmap("://status/status_online.png"));
        }else{
            ui->icoOnline->setPixmap(QPixmap("://status/status_offline.png"));
        }

        ui->frameNewSession->setVisible(m_data->isEnabled() && !m_data->isNew() &&
                                        m_data->getUuid() != m_comManager->getCurrentUser().getUuid());

        if (m_data->hasBusyStateField())
            ui->btnNewSession->setEnabled(!m_data->isBusy());

        if (ui->wdgUser->formHasStructure())
            ui->wdgUser->fillFormFromData(m_data->toJson());
    }
}

void UserSummaryWidget::initUI()
{
    ui->wdgSessions->setComManager(m_comManager);
    ui->wdgSessions->setViewMode(SessionsListWidget::VIEW_USER_SESSIONS, m_data->getUuid(), m_data->getId());

    ui->cmbSessionType->setItemDelegate(new QStyledItemDelegate(ui->cmbSessionType));
    ui->wdgUser->hideFields({"user_username", "user_notes", "user_superadmin"});

    ui->tabNav->setCurrentIndex(0);
}

bool UserSummaryWidget::validateData()
{
    return ui->wdgUser->validateFormData();
}

void UserSummaryWidget::processFormsReply(QString form_type, QString data)
{
    if (form_type.startsWith(WEB_FORMS_QUERY_USER)){
        if (!ui->wdgUser->formHasStructure()){
            ui->wdgUser->buildUiFromStructure(data);
            if (m_data){
                if (m_data->getId() == m_comManager->getCurrentUser().getId()){
                    // Editing self - disable "enable" button
                    ui->wdgUser->hideField("user_enabled");
                }
            }
        }
        return;
    }
}

void UserSummaryWidget::processSiteAccessReply(QList<TeraData> access_list, QUrlQuery reply_query)
{
    // Check if the current logged user is admin in at least one of the user site
    bool is_admin = false;
    for(const TeraData &access: access_list){
        if(m_comManager->isCurrentUserSiteAdmin(access.getFieldValue("id_site").toInt())){
            is_admin = true;
            break;
        }
    }

    setLimited(!is_admin);
}

void UserSummaryWidget::saveData(bool signal)
{
    QJsonDocument user_data = ui->wdgUser->getFormDataJson(m_data->isNew());

    if (*m_data == m_comManager->getCurrentUser()){
        m_currentUserPasswordChanged = ui->wdgUser->getFieldDirty("user_password");
    }else{
        m_currentUserPasswordChanged = false;
    }

    //qDebug() << user_data.toJson();
    postDataRequest(WEB_USERINFO_PATH, user_data.toJson());


    if (signal){
        TeraData* new_data = ui->wdgUser->getFormDataObject(TERADATA_USER);
        new_data->setName(new_data->getFieldValue("user_firstname").toString() + " " + new_data->getFieldValue("user_lastname").toString());
        *m_data = *new_data;
        delete new_data;
        if (!m_currentUserPasswordChanged)
            // If current password was changed, we will process it in the user replies
            emit dataWasChanged();
    }
}

void UserSummaryWidget::processSessionTypesReply(QList<TeraData> session_types)
{
    ui->cmbSessionType->clear();

    for (const TeraData &st:session_types){
        if (!m_ids_session_types.contains(st.getId())){
            m_ids_session_types[st.getId()] = new TeraData(st);
            // New session ComboBox
            ui->cmbSessionType->addItem(st.getName(), st.getId());
        }else{
            *m_ids_session_types[st.getId()] = st;
        }
    }

    // Query sessions
    ui->wdgSessions->setSessionTypes(session_types);

}

void UserSummaryWidget::processUsersReply(QList<TeraData> users)
{
    for (int i=0; i<users.count(); i++){
        if (users.at(i) == *m_data){
            // Update password if it was changed
            if (m_currentUserPasswordChanged){
                m_comManager->setCredentials(m_data->getFieldValue("user_username").toString(),
                                             ui->wdgUser->getFieldValue("user_password").toString());
                m_currentUserPasswordChanged = false;
                emit dataWasChanged();
            }
            // We found "ourself" in the list - update data.
            //*m_data = users.at(i);
            m_data->updateFrom(users.at(i));
            updateFieldsValue();
            break;
        }
    }
}

void UserSummaryWidget::processStatsReply(TeraData stats, QUrlQuery reply_query)
{

    // Check if stats are for us
    if (!reply_query.hasQueryItem("id_user"))
        return;

    if (reply_query.queryItemValue("id_user").toInt() != m_data->getId())
        return;

    // Fill stats information
    int total_sessions = stats.getFieldValue("sessions_total_count").toInt();
    ui->wdgSessions->setSessionsCount(total_sessions);

    // Query sessions types
    QUrlQuery args;
   /* if (m_idProject > 0)
        args.addQueryItem(WEB_QUERY_ID_PROJECT, QString::number(m_idProject));*/
    queryDataRequest(WEB_SESSIONTYPE_PATH, args);
}

void UserSummaryWidget::userFormValueChanged(QWidget *widget, QVariant value)
{
    if (widget == ui->wdgUser->getWidgetForField("user_password")){
        QString current_pass = value.toString();
        if (!current_pass.isEmpty() && !m_passwordJustGenerated){
            // Show password dialog
            PasswordStrengthDialog dlg(current_pass);

            if (dlg.exec() == QDialog::Accepted){
                m_passwordJustGenerated = true;
                ui->wdgUser->setFieldValue("user_password", dlg.getCurrentPassword());
            }else{
                ui->wdgUser->setFieldValue("user_password", "");
            }

        }else{
            if (m_passwordJustGenerated)
                m_passwordJustGenerated = false;
        }
    }
}

void UserSummaryWidget::userFormValueHasFocus(QWidget *widget)
{
    if (widget == ui->wdgUser->getWidgetForField("user_password")){
        if (!m_passwordJustGenerated){
            // Show password dialog
            QString current_pass = ui->wdgUser->getFieldValue("user_password").toString();
            PasswordStrengthDialog dlg(current_pass);
            //QLineEdit* wdg_editor = dynamic_cast<QLineEdit*>(ui->wdgUser->getWidgetForField("user_password"));
            //dlg.setCursorPosition(wdg_editor->cursorPosition());

            if (dlg.exec() == QDialog::Accepted){
                m_passwordJustGenerated = true;
                ui->wdgUser->setFieldValue("user_password", dlg.getCurrentPassword());
            }else{
                ui->wdgUser->setFieldValue("user_password", "");
                //wdg_editor->undo();
            }

        }else{
            if (m_passwordJustGenerated)
                m_passwordJustGenerated = false;
        }
    }
}

void UserSummaryWidget::ws_userEvent(UserEvent event)
{
    if (!m_data)
        return;

    if (QString::fromStdString(event.user_uuid()) != m_data->getUuid())
        return; // Not for us!

    if (event.type() == UserEvent_EventType_USER_CONNECTED){
        m_data->setOnline(true);
    }
    if (event.type() == UserEvent_EventType_USER_DISCONNECTED){
        m_data->setOnline(false);
    }
    if (event.type() == UserEvent_EventType_USER_JOINED_SESSION){
        m_data->setBusy(true);
    }
    if (event.type() == UserEvent_EventType_USER_LEFT_SESSION){
        m_data->setBusy(false);
    }
    updateFieldsValue();
}

void UserSummaryWidget::sessionLobbyStartSessionRequested()
{
    int id_session_type = ui->cmbSessionType->currentData().toInt();
    // Start session
    m_comManager->startSession(*m_ids_session_types[id_session_type],
                               m_sessionLobby->getIdSession(),
                               m_sessionLobby->getSessionParticipantsUuids(),
                               m_sessionLobby->getSessionUsersUuids(),
                               m_sessionLobby->getSessionDevicesUuids(),
                               m_sessionLobby->getSessionConfig());

    m_sessionLobby->deleteLater();
    m_sessionLobby = nullptr;
}

void UserSummaryWidget::sessionLobbyStartSessionCancelled()
{
    if (m_sessionLobby){
        m_sessionLobby->deleteLater();
        m_sessionLobby = nullptr;
    }
}

void UserSummaryWidget::on_btnNewSession_clicked()
{
    if (ui->cmbSessionType->currentIndex() < 0)
        return;

    if (!m_data)
        return;

    int id_session_type = ui->cmbSessionType->currentData().toInt();

    if (m_sessionLobby)
        m_sessionLobby->deleteLater();

    m_sessionLobby = new SessionLobbyDialog(m_comManager, *m_ids_session_types[id_session_type], -1, 0, this);

    // Add current users to session
    m_sessionLobby->addUsersToSession(QList<TeraData>() << m_comManager->getCurrentUser() << *m_data, QList<int>() << m_comManager->getCurrentUser().getId() << m_data->getId());

    connect(m_sessionLobby, &QDialog::accepted, this, &UserSummaryWidget::sessionLobbyStartSessionRequested);
    connect(m_sessionLobby, &QDialog::rejected, this, &UserSummaryWidget::sessionLobbyStartSessionCancelled);

    // Show Session Lobby
    m_sessionLobby->exec();
}
