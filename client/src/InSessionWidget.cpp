#include "InSessionWidget.h"
#include "ui_InSessionWidget.h"

InSessionWidget::InSessionWidget(ComManager *comMan, const TeraData* session_type, const int id_session, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InSessionWidget),
    m_sessionType(*session_type)
{
    ui->setupUi(this);

    m_comManager = comMan;

    // Create temporary object
    m_session = new TeraData(TERADATA_SESSION);
    m_session->setId(id_session);

    connectSignals();

    //Query session information
    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(id_session));
    m_comManager->doQuery(WEB_SESSIONINFO_PATH, args);

    initUI();

}

InSessionWidget::~InSessionWidget()
{
    delete ui;
    if (m_session)
        delete m_session;
}

void InSessionWidget::disconnectSignals()
{
    disconnect(m_comManager, nullptr, this, nullptr);
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

void InSessionWidget::connectSignals()
{
    connect(m_comManager, &ComManager::sessionsReceived, this, &InSessionWidget::processSessionsReply);
}

void InSessionWidget::initUI()
{
    ui->btnInSessionInfos->setChecked(false);
    ui->tabInfos->hide();

    // Get session type category
    TeraSessionCategory::SessionTypeCategories category = getSessionTypeCategory();

    switch(category){
    default:
        GlobalMessageBox msg_box;
        msg_box.showWarning(tr("Catégorie de séance non-supportée"), tr("La catégorie de séance \"") + TeraSessionCategory::getSessionTypeCategoryName(category) + "\" n'est pas gérée par cette version du logiciel.\n\nVeuillez vérifier si une mise à jour existe ou contribuez au développement du logiciel!");
    }
}

void InSessionWidget::updateUI()
{

}

TeraSessionCategory::SessionTypeCategories InSessionWidget::getSessionTypeCategory()
{
    return static_cast<TeraSessionCategory::SessionTypeCategories>(m_sessionType.getFieldValue("session_type_category").toInt());
}
