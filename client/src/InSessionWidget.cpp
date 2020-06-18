#include "InSessionWidget.h"
#include "ui_InSessionWidget.h"

InSessionWidget::InSessionWidget(ComManager *comMan, const TeraData* session_type, const int id_session, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InSessionWidget),
    m_sessionType(*session_type)
{
    ui->setupUi(this);

    m_comManager = comMan;
    m_serviceWidget = nullptr;



    connectSignals();

    //Query session information
    if (id_session >0){
        setSessionId(id_session);
    }

    initUI();

}

InSessionWidget::~InSessionWidget()
{
    delete ui;
    if (m_session)
        delete m_session;
    if (m_serviceWidget)
        m_serviceWidget->deleteLater();
}

void InSessionWidget::disconnectSignals()
{
    disconnect(m_comManager, nullptr, this, nullptr);
}

void InSessionWidget::setSessionId(int session_id)
{
    // Create temporary object
    m_session = new TeraData(TERADATA_SESSION);
    m_session->setId(session_id);

    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(session_id));
    m_comManager->doQuery(WEB_SESSIONINFO_PATH, args);
}

void InSessionWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (!m_session){
        StartSessionDialog diag(tr("Démarrage de séance en cours..."), m_comManager);
        diag.exec();
    }


}

void InSessionWidget::on_btnEndSession_clicked()
{
    GlobalMessageBox msg_box;
    // TODO: Manage if we are the session creator or not...
    if (msg_box.showYesNo(tr("Terminer la séance?"), tr("Mettre fin à la séance?")) == QMessageBox::Yes){
        int id_service = 0;
        if (getSessionTypeCategory() == TeraSessionCategory::SESSION_TYPE_SERVICE){
            id_service = m_sessionType.getFieldValue("id_service").toInt();
        }
        m_comManager->stopSession(*m_session, id_service);
        StartSessionDialog diag(tr("Arrêt de la séance en cours..."), m_comManager);
        diag.exec();
    }
}

void InSessionWidget::on_btnInSessionInfos_toggled(bool checked)
{
    ui->tabInfos->setVisible(checked);

}

void InSessionWidget::processSessionsReply(QList<TeraData> sessions)
{
    for(TeraData session:sessions){
        if (session.getId() == m_session->getId()){
            // This is an update to the session information we have
            delete m_session;
            m_session = new TeraData(session);
            updateUI();
        }
    }
}

void InSessionWidget::processJoinSessionEvent(JoinSessionEvent event)
{
    QString session_uuid = QString::fromStdString(event.session_uuid());

    // Check if that event is really for us
    /*if (!m_session){
        LOG_ERROR("Received JoinSessionEvent, but no current session!", "InSessionWidget::processJoinSessionEvent");
        m_comManager->getWebSocketManager()->sendJoinSessionReply(session_uuid, JoinSessionReply::REPLY_DENIED);
        return;
    }

    if (m_session->getFieldValue("session_uuid").toString() != session_uuid){
        LOG_WARNING("Received JoinSessionEvent, but it's not for current session - replying busy", "InSessionWidget::processJoinSessionEvent");
        m_comManager->getWebSocketManager()->sendJoinSessionReply(session_uuid, JoinSessionReply::REPLY_BUSY);
        return;
    }*/

    // Update users list
    // TODO

    // Update participants list
    // TODO

    // Forward to widget
    if (m_serviceWidget){
        bool result = m_serviceWidget->handleJoinSessionEvent(event);
        if (result){
            m_comManager->getWebSocketManager()->sendJoinSessionReply(session_uuid, JoinSessionReply::REPLY_ACCEPTED);
        }else{
            m_comManager->getWebSocketManager()->sendJoinSessionReply(session_uuid, JoinSessionReply::REPLY_DENIED);
        }
    }

}

void InSessionWidget::connectSignals()
{
    connect(m_comManager, &ComManager::sessionsReceived, this, &InSessionWidget::processSessionsReply);
    connect(m_comManager->getWebSocketManager(), &WebSocketManager::joinSessionEventReceived, this, &InSessionWidget::processJoinSessionEvent);
}

void InSessionWidget::initUI()
{

    ui->btnInSessionInfos->setChecked(true);
    //ui->tabInfos->hide();

    // Clean up, if needed
    if (m_serviceWidget){
        m_serviceWidget->deleteLater();
        m_serviceWidget = nullptr;
    }

    // Get session type category
    TeraSessionCategory::SessionTypeCategories category = getSessionTypeCategory();

    switch(category){
    case TeraSessionCategory::SESSION_TYPE_SERVICE:{
        // Get service key to load the proper ui
        QString service_key = m_sessionType.getFieldValue("session_type_service_key").toString();
        bool handled = false;
        if (service_key == "VideoRehabService"){
            // Main widget = QWebEngine
            m_serviceWidget = new VideoRehabWidget(m_comManager);
            setMainWidget(m_serviceWidget);
            handled = true;
        }

        if (!handled){
            GlobalMessageBox msg_box;
            msg_box.showWarning(tr("Service non-supporté"), tr("Le service \"") + service_key + tr("\" n'est pas gérée par cette version du logiciel.\n\nVeuillez vérifier si une mise à jour existe ou contribuez au développement du logiciel!"));
        }

        break;
    }
    default:
        GlobalMessageBox msg_box;
        msg_box.showWarning(tr("Catégorie de séance non-supportée"), tr("La catégorie de séance \"") + TeraSessionCategory::getSessionTypeCategoryName(category) + tr("\" n'est pas gérée par cette version du logiciel.\n\nVeuillez vérifier si une mise à jour existe ou contribuez au développement du logiciel!"));
    }
}

void InSessionWidget::updateUI()
{

}

void InSessionWidget::setMainWidget(QWidget *wdg)
{
    // Check for layout
    if (!ui->widgetMain->layout()){
        // No existing layout - create one
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        ui->widgetMain->setLayout(layout);
    }

    ui->widgetMain->layout()->addWidget(wdg);

}

TeraSessionCategory::SessionTypeCategories InSessionWidget::getSessionTypeCategory()
{
    return static_cast<TeraSessionCategory::SessionTypeCategories>(m_sessionType.getFieldValue("session_type_category").toInt());
}

