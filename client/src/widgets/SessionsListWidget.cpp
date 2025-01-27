#include "SessionsListWidget.h"
#include "ui_SessionsListWidget.h"

#include <QScreen>

#include "GlobalMessageBox.h"
#include "editors/SessionWidget.h"
#include "widgets/TableDateWidgetItem.h"

#include "widgets/AssetsWidget.h"
#include "widgets/TestsWidget.h"

SessionsListWidget::SessionsListWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionsListWidget)
{
    ui->setupUi(this);

    // Init local variables
    m_totalSessions = 0;
    m_totalAssets = 0;
    m_totalTests = 0;
    m_currentIdSession = -1;
    m_currentSessions = 0;
    m_sessionsLoading = false;
    m_currentSessionShowAssets = false;
    m_currentSessionShowTests = false;
    m_allowDeletion = false;
    m_allowFileTransfers = false;
    m_comManager = nullptr;
    m_diag_editor = nullptr;
    m_viewMode = VIEW_NONE;

    // InitUI
    initUI();

}

SessionsListWidget::~SessionsListWidget()
{
    qDeleteAll(m_ids_session_types);
    qDeleteAll(m_ids_sessions);

    delete ui;
}

void SessionsListWidget::setComManager(ComManager *comMan)
{
    if (m_comManager != comMan){
        m_comManager = comMan;
        connectSignals();
    }
}

void SessionsListWidget::setViewMode(const ViewMode &mode, const QString &uuid, const int &id, const int &id_project)
{
    m_viewMode = mode;
    m_currentUuid = uuid;
    m_currentIdProject = id_project;
    m_currentId = id;
}

void SessionsListWidget::initUI()
{
    ui->frameFilterSessionTypes->hide();
    ui->btnCheckSessionTypes->hide();
    ui->btnAssetsBrowser->hide();
    ui->btnTestsBrowser->hide();

    setSessionsLoading(false);

    // Set default calendar view based on current date
    updateCalendars(QDate::currentDate());

    // Intercepts some events
    ui->tableSessions->installEventFilter(this);

    // Set default session sorting
    ui->tableSessions->sortItems(1, Qt::DescendingOrder);

    // Load table icons
    m_deleteIcon = QIcon(":/icons/delete_old.png");
    m_viewIcon = QIcon(":/icons/search.png");
    m_downloadIcon = QIcon(":/icons/data.png");
    m_resumeIcon = QIcon(":/icons/play.png");
    m_testIcon = QIcon(":/icons/test.png");
}

void SessionsListWidget::connectSignals()
{
    connect(ui->calMonth1, &HistoryCalendarWidget::clicked, this, &SessionsListWidget::currentCalendarDateChanged);
    connect(ui->calMonth2, &HistoryCalendarWidget::clicked, this, &SessionsListWidget::currentCalendarDateChanged);
    connect(ui->calMonth3, &HistoryCalendarWidget::clicked, this, &SessionsListWidget::currentCalendarDateChanged);
    connect(ui->calMonth1, &HistoryCalendarWidget::activated, this, &SessionsListWidget::currentCalendarDateActivated);
    connect(ui->calMonth2, &HistoryCalendarWidget::activated, this, &SessionsListWidget::currentCalendarDateActivated);
    connect(ui->calMonth3, &HistoryCalendarWidget::activated, this, &SessionsListWidget::currentCalendarDateActivated);

    if (m_comManager){
        connect(m_comManager, &ComManager::sessionsReceived, this, &SessionsListWidget::processSessionsReply);
        connect(m_comManager, &ComManager::deleteResultsOK, this, &SessionsListWidget::deleteDataReply);
    }
}

void SessionsListWidget::enableDeletion(const bool &enable)
{
    m_allowDeletion = enable;
    ui->btnDelSession->setVisible(enable);
}

void SessionsListWidget::enableFileTransfers(const bool &enable)
{
    m_allowFileTransfers = enable;
}

void SessionsListWidget::enableTestInvitations(QList<TeraData> *test_types)
{
    m_testTypes = test_types;
}

void SessionsListWidget::enableEmails(const bool &enable)
{
    m_allowEmails = enable;
}

void SessionsListWidget::setSessionsCount(const int &count)
{
    m_totalSessions = count;
    ui->tableSessions->setRowCount(m_totalSessions);
    ui->progSessionsLoad->setMaximum(m_totalSessions);

    emit sessionsCountUpdated(m_totalSessions);
}

int SessionsListWidget::getSessionsCount()
{
    return m_totalSessions;
}

void SessionsListWidget::setSessionTypes(QList<TeraData> session_types)
{
    for (const TeraData &st:session_types){
        if (!m_ids_session_types.contains(st.getId())){
            m_ids_session_types[st.getId()] = new TeraData(st);
            // Add to session list
            QListWidgetItem* s = new QListWidgetItem(st.getName());
            s->setData(Qt::UserRole,st.getId());
            s->setCheckState(Qt::Checked);
            s->setForeground(QColor(st.getFieldValue("session_type_color").toString()));
            s->setFont(QFont("Arial",10));
            ui->lstFilters->addItem(s);
        }else{
            // Existing, must update values
            *m_ids_session_types[st.getId()] = st;
            for (int i=0; i<ui->lstFilters->count(); i++){
                QListWidgetItem* item = ui->lstFilters->item(i);
                if (item->data(Qt::UserRole).toInt() == st.getId()){
                    item->setText(st.getName());
                }
            }
        }
    }

    ui->calMonth1->setSessionTypes(m_ids_session_types.values());
    ui->calMonth2->setSessionTypes(m_ids_session_types.values());
    ui->calMonth3->setSessionTypes(m_ids_session_types.values());

    // Query sessions
    if (m_currentSessions == 0 && !m_sessionsLoading){
        querySessions();
    }
}

const TeraData *SessionsListWidget::hasResumableSession(const int &id_session_type, const QDate &target_date)
{
    TeraData *rval = nullptr;
    // Check if we have a session that can be resumed for that date
    for(TeraData* session:std::as_const(m_ids_sessions)){
        if (id_session_type == session->getFieldValue("id_session_type").toInt()){
            int session_status = session->getFieldValue("session_status").toInt();
            if (session_status == TeraSessionStatus::STATUS_INPROGRESS || session_status == TeraSessionStatus::STATUS_NOTSTARTED){
                QDateTime session_start_time = session->getFieldValue("session_start_datetime").toDateTime().toLocalTime();

                if (session_start_time.date() == target_date){
                    // Adds duration to have 1h since it was ended
                    session_start_time = session_start_time.addSecs(session->getFieldValue("session_duration").toInt());
                    // Allow for +/- 1h30 around the time of the session, in case of planned session
                    if(qAbs(session_start_time.secsTo(QDateTime::currentDateTime())) <= 5400){
                        rval = session;
                        break;
                    }
                }
            }
        }
    }
    return rval;
}

void SessionsListWidget::processSessionsReply(QList<TeraData> sessions, QUrlQuery args)
{
    for(TeraData &session:sessions){
        if (session.getId() == m_currentIdSession && sessions.count() == 1){
            // This is a session we requested info on
            showSessionEditor(&session);
            m_currentIdSession = -1;
            return;
        }
        updateSession(&session, sessions.count()==1);
    }
    //qDebug() << sessions.count();

    // Query next sessions if needed
    querySessions();
}

void SessionsListWidget::deleteDataReply(QString path, int id)
{
    if (id==0)
        return;

    if (path == WEB_SESSIONINFO_PATH){
        // A session got deleted - check if it affects the current display
        if (m_listSessions_items.contains(id)){
            ui->tableSessions->removeRow(m_listSessions_items[id]->row());
            delete m_ids_sessions[id];
            m_ids_sessions.remove(id);
            m_listSessions_items.remove(id);

            // Update calendars
            ui->calMonth1->setData(m_ids_sessions.values());
            ui->calMonth2->setData(m_ids_sessions.values());
            ui->calMonth3->setData(m_ids_sessions.values());

            m_currentSessions--;
            m_totalSessions--;

            emit sessionsCountUpdated(m_totalSessions);
        }
    }

}

void SessionsListWidget::btnViewSession_clicked()
{
    // Check if the sender is a QToolButton (from the action column)
    QToolButton* action_btn = dynamic_cast<QToolButton*>(sender());
    QTableWidgetItem* session_item = nullptr;
    if (action_btn){
        // Select row according to the session id of that button
        int id_session = action_btn->property("id_session").toInt();
        session_item = m_listSessions_items[id_session];
    }

    if (session_item){
        displaySessionDetails(session_item);
    }
}

void SessionsListWidget::btnViewSessionAssets_clicked()
{
    QToolButton* button = dynamic_cast<QToolButton*>(sender());
    QTableWidgetItem* session_item = nullptr;
    if (button){
        int id_session = button->property("id_session").toInt();
        session_item = m_listSessions_items[id_session];
        if (session_item){
            displaySessionDetails(session_item, true);
        }
    }
}

void SessionsListWidget::btnViewSessionTests_clicked()
{
    QToolButton* button = dynamic_cast<QToolButton*>(sender());
    QTableWidgetItem* session_item = nullptr;
    if (button){
        int id_session = button->property("id_session").toInt();
        session_item = m_listSessions_items[id_session];
        if (session_item){
            displaySessionDetails(session_item, false, true);
        }
    }
}

void SessionsListWidget::btnResumeSession_clicked()
{
    // Check if the sender is a QToolButton (from the action column)
    QToolButton* action_btn = dynamic_cast<QToolButton*>(sender());

    if (action_btn){
        // Select row according to the session id of that button
        int id_session = action_btn->property("id_session").toInt();
        int id_session_type = action_btn->property("id_session_type").toInt();
        emit startSessionRequested(id_session_type, id_session);
    }
}

void SessionsListWidget::sessionAssetsCountChanged(int id_session, int new_count)
{
    if (m_ids_sessions.contains(id_session)){
        // Check if we need to toggle the "asset download" icon
        TeraData session_data(*m_ids_sessions[id_session]); // Local copy to prevent deletion when calling updateSession
        session_data.setFieldValue("session_assets_count", new_count);
        updateSession(&session_data, false);
    }
}

void SessionsListWidget::sessionTestsCountChanged(int id_session, int new_count)
{
    if (m_ids_sessions.contains(id_session)){
        // Check if we need to toggle the "test" icon
        TeraData session_data(*m_ids_sessions[id_session]); // Local copy to prevent deletion when calling updateSession
        session_data.setFieldValue("session_tests_count", new_count);
        updateSession(&session_data, false);
    }
}

void SessionsListWidget::on_btnDelSession_clicked()
{
    if (!m_allowDeletion)
        return;

    // Check if the sender is a QToolButton (from the action column)
    QToolButton* action_btn = dynamic_cast<QToolButton*>(sender());
    if (action_btn && action_btn != ui->btnDelSession){
        // Select row according to the session id of that button
        int id_session = action_btn->property("id_session").toInt();
        QTableWidgetItem* session_item = m_listSessions_items[id_session];
        if (session_item)
            ui->tableSessions->selectRow(session_item->row());
    }

    if (ui->tableSessions->selectedItems().count()==0)
        return;

    GlobalMessageBox diag;
    QMessageBox::StandardButton answer = QMessageBox::No;
    if (ui->tableSessions->selectionModel()->selectedRows().count() == 1){
        QTableWidgetItem* base_item = ui->tableSessions->item(ui->tableSessions->currentRow(),0);
        answer = diag.showYesNo(tr("Suppression?"), tr("Êtes-vous sûrs de vouloir supprimer """) + base_item->text() + """?");
    }else{
        answer = diag.showYesNo(tr("Suppression?"), tr("Êtes-vous sûrs de vouloir supprimer toutes les séances sélectionnées?"));
    }

    if (answer == QMessageBox::Yes){
        // We must delete!
        foreach(QModelIndex selected_row, ui->tableSessions->selectionModel()->selectedRows()){
            QTableWidgetItem* base_item = ui->tableSessions->item(selected_row.row(),0); // Get item at index 0 which is linked to session id
            //m_comManager->doDelete(TeraData::getPathForDataType(TERADATA_SESSION), m_listSessions_items.key(base_item));
            m_comManager->doDelete(TeraData::getPathForDataType(TERADATA_SESSION), m_listSessions_items.key(base_item));
        }
    }
}


void SessionsListWidget::on_tableSessions_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    Q_UNUSED(previous)
    ui->btnDelSession->setEnabled(current);
}


void SessionsListWidget::on_lstFilters_itemChanged(QListWidgetItem *changed)
{
    QList<int> ids;

    for (int i=0; i<ui->lstFilters->count(); i++){
        if (ui->lstFilters->item(i)->checkState() == Qt::Checked){
            ids.append(ui->lstFilters->item(i)->data(Qt::UserRole).toInt());
        }
    }

    ui->calMonth1->setFilters(ids);
    ui->calMonth2->setFilters(ids);
    ui->calMonth3->setFilters(ids);

    if (!changed)
        return;
    // Update session tables
    QString current_ses_type = changed->text();
    for (int i=0; i<ui->tableSessions->rowCount(); i++){
        if (ui->tableSessions->item(i,2)->text() == current_ses_type){
            ui->tableSessions->setRowHidden(i, changed->checkState()==Qt::Unchecked);
        }
    }
}


void SessionsListWidget::on_btnNextCal_clicked()
{
    QDate new_date;
    new_date.setDate(ui->calMonth1->yearShown(), ui->calMonth1->monthShown(), 1);
    new_date = new_date.addMonths(1);

    updateCalendars(new_date);
}

void SessionsListWidget::updateCalendars(QDate left_date)
{
    ui->calMonth1->setCurrentPage(left_date.year(),left_date.month());
    ui->calMonth1->setMinimumDate(QDate(left_date.year(),left_date.month(), 1));
    ui->calMonth1->setMaximumDate(QDate(left_date.year(),left_date.month(), left_date.daysInMonth()));
    ui->lblMonth1->setText(Utils::toCamelCase(QLocale().monthName(left_date.month())) + " " + QString::number(left_date.year()));

    left_date = left_date.addMonths(1);
    ui->calMonth2->setCurrentPage(left_date.year(),left_date.month());
    ui->calMonth2->setMinimumDate(QDate(left_date.year(),left_date.month(), 1));
    ui->calMonth2->setMaximumDate(QDate(left_date.year(),left_date.month(), left_date.daysInMonth()));
    ui->lblMonth2->setText(Utils::toCamelCase(QLocale().monthName(left_date.month())) + " " + QString::number(left_date.year()));

    left_date = left_date.addMonths(1);
    ui->calMonth3->setCurrentPage(left_date.year(),left_date.month());
    ui->calMonth3->setMinimumDate(QDate(left_date.year(),left_date.month(), 1));
    ui->calMonth3->setMaximumDate(QDate(left_date.year(),left_date.month(), left_date.daysInMonth()));
    ui->lblMonth3->setText(Utils::toCamelCase(QLocale().monthName(left_date.month())) + " " + QString::number(left_date.year()));

    // Check if we must enable the previous month button
    QDate min_date = getMinimumSessionDate();

    if (ui->calMonth1->yearShown()<=min_date.year() && ui->calMonth1->monthShown()<=min_date.month())
        ui->btnPrevCal->setEnabled(false);
    else
        ui->btnPrevCal->setEnabled(true);
}

QDate SessionsListWidget::getMinimumSessionDate()
{
    QDate min_date = QDate::currentDate();
    for (TeraData* session:std::as_const(m_ids_sessions)){
        QDate session_date = session->getFieldValue("session_start_datetime").toDateTime().toLocalTime().date();
        if (session_date < min_date)
            min_date = session_date;
    }

    return min_date;
}

QDate SessionsListWidget::getMaximumSessionDate()
{
    QDate max_date;
    for (TeraData* session:std::as_const(m_ids_sessions)){
        QDate session_date = session->getFieldValue("session_start_datetime").toDateTime().toLocalTime().date();
        if (session_date > max_date)
            max_date = session_date;
    }
    return max_date;
}

void SessionsListWidget::newSessionRequest(const QDateTime &session_datetime)
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }
    m_diag_editor = new BaseDialog(this);

    TeraData* new_session = new TeraData(TERADATA_SESSION);
    new_session->setFieldValue("session_name", tr("Nouvelle séance"));
    new_session->setFieldValue("session_start_datetime", session_datetime);
    new_session->setFieldValue("session_status", TeraSessionStatus::STATUS_NOTSTARTED);
    new_session->setFieldValue("id_project", m_currentIdProject);
    new_session->setFieldValue("id_creator_user", m_comManager->getCurrentUser().getId());
    if (m_viewMode == VIEW_PARTICIPANT_SESSIONS)
        new_session->setFieldValue("participant_uuid", m_currentUuid);
    if (m_viewMode == VIEW_USER_SESSIONS)
        new_session->setFieldValue("user_uuid", m_currentUuid);
    if (m_viewMode == VIEW_DEVICE_SESSIONS)
        new_session->setFieldValue("device_uuid", m_currentUuid);

    SessionWidget* ses_widget = new SessionWidget(m_comManager, new_session, m_diag_editor);
    m_diag_editor->setCentralWidget(ses_widget);

    m_diag_editor->setWindowTitle(tr("Séance"));
    m_diag_editor->setMinimumSize(this->width(), this->height());


    connect(ses_widget, &SessionWidget::closeRequest, m_diag_editor, &QDialog::accept);
    connect(ses_widget, &SessionWidget::dataWasChanged, m_diag_editor, &QDialog::accept);
    connect(ses_widget, &SessionWidget::dataWasDeleted, m_diag_editor, &QDialog::accept);

    m_diag_editor->open();
}

void SessionsListWidget::setSessionsLoading(const bool &loading)
{
    ui->progSessionsLoad->setVisible(loading);
    ui->frameCalendar->setVisible(!loading);
    ui->tableSessions->setVisible(!loading);
    ui->frameSessionsControls->setVisible(!loading);
    m_sessionsLoading = loading;
}

void SessionsListWidget::querySessions()
{
    if (m_totalSessions == 0){
        //qWarning() << "SessionsListWidget::querySessions(): No sessions to query - aborting!";
        return;
    }
    ui->tableSessions->setSortingEnabled(false);
    int sessions_left = m_totalSessions - m_currentSessions;
    //qDebug() << m_totalSessions << m_currentSessions << sessions_left << m_currentIdSession;
    setSessionsLoading(sessions_left > 0);

    if (sessions_left>0){
        ui->progSessionsLoad->setValue(m_currentSessions);
        QUrlQuery query;
        switch(m_viewMode){
        case VIEW_NONE:
            break;
        case VIEW_PARTICIPANT_SESSIONS:
            query.addQueryItem(WEB_QUERY_ID_PARTICIPANT, QString::number(m_currentId));
            break;
        case VIEW_USER_SESSIONS:
            query.addQueryItem(WEB_QUERY_ID_USER, QString::number(m_currentId));
            break;
        case VIEW_DEVICE_SESSIONS:
            query.addQueryItem(WEB_QUERY_ID_DEVICE, QString::number(m_currentId));
            break;
        }

        query.addQueryItem(WEB_QUERY_OFFSET, QString::number(m_currentSessions));
        query.addQueryItem(WEB_QUERY_LIMIT, QString::number(QUERY_SESSION_LIMIT_PER_QUERY)); // Limit number of sessions per query
        query.addQueryItem(WEB_QUERY_WITH_SESSIONTYPE, "1");
        query.addQueryItem(WEB_QUERY_LIST, "1");
        //queryDataRequest(WEB_SESSIONINFO_PATH, query);
        m_comManager->doGet(WEB_SESSIONINFO_PATH, query);
    }else{
        // No more session to query

        // Update calendar view
        on_lstFilters_itemChanged(nullptr);
        updateCalendars(getMaximumSessionDate().addMonths(-2));
        //updateCalendars(getMinimumSessionDate());
        ui->calMonth1->setData(m_ids_sessions.values());
        ui->calMonth2->setData(m_ids_sessions.values());
        ui->calMonth3->setData(m_ids_sessions.values());

        ui->tableSessions->setSortingEnabled(true);
        ui->tableSessions->resizeColumnsToContents();
    }

}

bool SessionsListWidget::eventFilter(QObject *o, QEvent *e)
{
    if( o == ui->tableSessions && e->type() == QEvent::KeyRelease )
    {
        QKeyEvent* key_event = dynamic_cast<QKeyEvent*>(e);
        if (key_event){
            if (key_event->key() == Qt::Key_Delete){
                on_btnDelSession_clicked();
            }
        }
    }

    return QWidget::eventFilter(o,e);
}

void SessionsListWidget::updateSession(const TeraData *session, const bool &auto_position)
{
    int id_session = session->getId();

    QTableWidgetItem* name_item;
    TableDateWidgetItem* date_item;
    QTableWidgetItem* type_item;
    QTableWidgetItem* duration_item;
    QTableWidgetItem* user_item;
    QTableWidgetItem* status_item;
    QToolButton* btnDownload = nullptr;
    QToolButton* btnResume = nullptr;
    QToolButton* btnTests = nullptr;

    if (m_listSessions_items.contains(id_session)){
       // Already there, get items
       //qDebug() << "Updating session " << id_session;
       name_item = m_listSessions_items[id_session];
       date_item = dynamic_cast<TableDateWidgetItem*>(ui->tableSessions->item(name_item->row(), 1));
       type_item = ui->tableSessions->item(name_item->row(), 2);
       status_item = ui->tableSessions->item(name_item->row(), 3);
       duration_item = ui->tableSessions->item(name_item->row(), 4);
       user_item = ui->tableSessions->item(name_item->row(), 5);
       if (ui->tableSessions->cellWidget(name_item->row(), 6))
           if (ui->tableSessions->cellWidget(name_item->row(), 6)->layout()){
               if (ui->tableSessions->cellWidget(name_item->row(), 6)->layout()->itemAt(2))
                  btnDownload = dynamic_cast<QToolButton*>(ui->tableSessions->cellWidget(name_item->row(), 6)->layout()->itemAt(2)->widget());
               if (ui->tableSessions->cellWidget(name_item->row(), 6)->layout()->itemAt(3))
                  btnTests = dynamic_cast<QToolButton*>(ui->tableSessions->cellWidget(name_item->row(), 6)->layout()->itemAt(3)->widget());
               if (ui->tableSessions->cellWidget(name_item->row(), 6)->layout()->itemAt(4))
                  btnResume = dynamic_cast<QToolButton*>(ui->tableSessions->cellWidget(name_item->row(), 6)->layout()->itemAt(4)->widget());
           }

       if (m_ids_sessions[id_session]->hasFieldName("session_assets_count")){
           m_totalAssets -= m_ids_sessions[id_session]->getFieldValue("session_assets_count").toInt();
       }
       if (m_ids_sessions[id_session]->hasFieldName("session_tests_count")){
           m_totalTests -= m_ids_sessions[id_session]->getFieldValue("session_tests_count").toInt();
       }
       delete m_ids_sessions[id_session];
    }else{
        //ui->tableSessions->setSortingEnabled(false); // Disable sorting so we know the correct inserted row

        if (ui->tableSessions->rowCount() < m_currentSessions+1){
            ui->tableSessions->setRowCount(ui->tableSessions->rowCount()+1);
        }
        int current_row = m_currentSessions;
        name_item = new QTableWidgetItem(QIcon(TeraData::getIconFilenameForDataType(TERADATA_SESSION)),"");
        ui->tableSessions->setItem(current_row, 0, name_item);
        date_item = new TableDateWidgetItem("");
        ui->tableSessions->setItem(current_row, 1, date_item);
        type_item = new QTableWidgetItem("");
        ui->tableSessions->setItem(current_row, 2, type_item);
        status_item = new QTableWidgetItem("");
        ui->tableSessions->setItem(current_row, 3, status_item);
        duration_item = new QTableWidgetItem("");
        duration_item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->tableSessions->setItem(current_row, 4, duration_item);
        user_item = new QTableWidgetItem("");
        ui->tableSessions->setItem(current_row, 5, user_item);

        // Create action buttons
        QFrame* action_frame = new QFrame();
        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(1);
        layout->setAlignment(Qt::AlignLeft);
        action_frame->setLayout(layout);

        // View session
        QToolButton* btnView = new QToolButton(action_frame);
        btnView->setIcon(m_viewIcon);
        btnView->setProperty("id_session", session->getId());
        btnView->setCursor(Qt::PointingHandCursor);
        btnView->setMaximumWidth(32);
        btnView->setToolTip(tr("Ouvrir"));
        connect(btnView, &QToolButton::clicked, this, &SessionsListWidget::btnViewSession_clicked);
        layout->addWidget(btnView);

        // Delete
        QToolButton* btnDelete = new QToolButton(action_frame);
        btnDelete->setIcon(m_deleteIcon);
        btnDelete->setProperty("id_session", session->getId());
        btnDelete->setCursor(Qt::PointingHandCursor);
        btnDelete->setMaximumWidth(32);
        btnDelete->setToolTip(tr("Supprimer"));
        if (m_allowDeletion)
            connect(btnDelete, &QToolButton::clicked, this, &SessionsListWidget::on_btnDelSession_clicked);
        else
            btnDelete->setVisible(false);
        layout->addWidget(btnDelete);

        // Download data
        btnDownload = new QToolButton(action_frame);
        btnDownload->setIcon(m_downloadIcon);
        btnDownload->setProperty("id_session", session->getId());
        btnDownload->setCursor(Qt::PointingHandCursor);
        btnDownload->setMaximumWidth(32);
        btnDownload->setToolTip(tr("Voir les données"));
        connect(btnDownload, &QToolButton::clicked, this, &SessionsListWidget::btnViewSessionAssets_clicked);
        layout->addWidget(btnDownload);

        // Tests display
        btnTests = new QToolButton(action_frame);
        btnTests->setIcon(m_testIcon);
        btnTests->setProperty("id_session", session->getId());
        btnTests->setCursor(Qt::PointingHandCursor);
        btnTests->setMaximumWidth(32);
        btnTests->setToolTip(tr("Voir les évaluations"));
        connect(btnTests, &QToolButton::clicked, this, &SessionsListWidget::btnViewSessionTests_clicked);
        layout->addWidget(btnTests);

        // Resume session
        btnResume = new QToolButton(action_frame);
        btnResume->setIcon(m_resumeIcon);
        btnResume->setProperty("id_session", session->getId());
        btnResume->setProperty("id_session_type", session->getFieldValue("id_session_type").toInt());
        btnResume->setCursor(Qt::PointingHandCursor);
        btnResume->setMaximumWidth(32);
        btnResume->setToolTip(tr("Continuer la séance"));
        connect(btnResume, &QToolButton::clicked, this, &SessionsListWidget::btnResumeSession_clicked);
        layout->addWidget(btnResume);

        ui->tableSessions->setCellWidget(current_row, 6, action_frame);

        m_listSessions_items[id_session] = name_item;

        m_currentSessions++;

        //ui->tableSessions->setSortingEnabled(true); // Reenable sorting

    }
    m_ids_sessions[id_session] = new TeraData(*session);

    // Update values
    name_item->setText(session->getName());
    date_item->setDate(session->getFieldValue("session_start_datetime"));

    if (session->hasFieldName("session_type_name")){
        type_item->setText(session->getFieldValue("session_type_name").toString());
        if (session->hasFieldName("session_type_color"))
            type_item->setForeground(QColor(session->getFieldValue("session_type_color").toString()));
    }else{
        // No session type in that structure - old API - use previous stored values
        int session_type = session->getFieldValue("id_session_type").toInt();
        if (m_ids_session_types.contains(session_type)){
            type_item->setText(m_ids_session_types[session_type]->getFieldValue("session_type_name").toString());
            type_item->setForeground(QColor(m_ids_session_types[session_type]->getFieldValue("session_type_color").toString()));
        }else{
            type_item->setText("Inconnu");
        }
    }

    duration_item->setText(QTime(0,0).addSecs(session->getFieldValue("session_duration").toInt()).toString("hh:mm:ss"));
    TeraSessionStatus::SessionStatus session_status = static_cast<TeraSessionStatus::SessionStatus>(session->getFieldValue("session_status").toInt());
    status_item->setText(TeraSessionStatus::getStatusName(session_status));
    // Set color depending on status_item
    //status_item->setTextColor(QColor(TeraSessionStatus::getStatusColor(session_status)));
    status_item->setForeground(Qt::black);
    status_item->setBackground(QColor(TeraSessionStatus::getStatusColor(session_status)));
    //QColor back_color = TeraForm::getGradientColor(3, 18, 33, static_cast<int>(session_date.daysTo(QDateTime::currentDateTime())));
    //back_color.setAlphaF(0.5);
    //date_item->setBackgroundColor(back_color);

    // Session creator
    if (session->hasFieldName("session_creator_user"))
        user_item->setText(session->getFieldValue("session_creator_user").toString());
    else if(session->hasFieldName("session_creator_device"))
        user_item->setText(tr("Appareil: ") + session->getFieldValue("session_creator_device").toString());
    else if(session->hasFieldName("session_creator_participant"))
        user_item->setText(tr("Participant: ") + session->getFieldValue("session_creator_participant").toString());
    else if(session->hasFieldName("session_creator_service"))
        user_item->setText(tr("Service: ") + session->getFieldValue("session_creator_service").toString());
    else {
        user_item->setText(tr("Inconnu"));
    }

    // Download data
    if (btnDownload){
        if (session->hasFieldName("session_assets_count")){
            int asset_count = session->getFieldValue("session_assets_count").toInt();
            bool has_assets = asset_count>0;
            //btnDownload->setVisible(has_assets);
            btnDownload->setEnabled(has_assets);
            if (!has_assets){
                btnDownload->setIcon(QIcon());
            }else{
                btnDownload->setIcon(m_downloadIcon);
            }
            m_totalAssets += asset_count;
            ui->btnAssetsBrowser->setVisible(m_totalAssets>0 && m_viewMode == ViewMode::VIEW_PARTICIPANT_SESSIONS);
        }else{
            //btnDownload->hide();
            btnDownload->setEnabled(false);
        }
    }

    // Show Tests
    if (btnTests){
        if (session->hasFieldName("session_tests_count")){
            int test_count = session->getFieldValue("session_tests_count").toInt();
            bool has_tests = test_count>0;
            //btnTests->setVisible(has_tests);
            btnTests->setEnabled(has_tests);
            if (!has_tests){
                btnTests->setIcon(QIcon());
            }else{
                btnTests->setIcon(m_testIcon);
            }
            m_totalTests += test_count;
            ui->btnTestsBrowser->setVisible(m_totalTests>0 && m_viewMode == ViewMode::VIEW_PARTICIPANT_SESSIONS);
        }else{
            //btnTests->hide();
            btnTests->setEnabled(false);
        }
    }

    // Resume session
    if (btnResume){
        btnResume->hide();
        if (session->hasFieldName("session_start_datetime")){
            QDateTime session_date = session->getFieldValue("session_start_datetime").toDateTime().toLocalTime();
            // Check if session type can be resumed
            int id_session_type = session->getFieldValue("id_session_type").toInt();
            TeraData* session_type = m_ids_session_types[id_session_type];
            if (session_type){
                if (session_type->getFieldValue("session_type_online").toBool() == true)
                    btnResume->setVisible(session_date.date() == QDate::currentDate());
            }
        }
    }

    // If only 1 session added, check to put it at the right place in the table
    if (m_currentSessions > 1 && auto_position){
        // Check if we have a sorting or not
        /*int sort_column = ui->tableSessions->horizontalHeader()->sortIndicatorSection();
        Qt::SortOrder sort_order = ui->tableSessions->horizontalHeader()->sortIndicatorOrder();
        // Reapply sorting
        ui->tableSessions->sortByColumn(sort_column, sort_order);*/

        // Select added item
        ui->tableSessions->selectRow(name_item->row());
        ui->tableSessions->scrollToItem(name_item, QAbstractItemView::PositionAtCenter);
    }
}

void SessionsListWidget::showSessionEditor(TeraData *session_info)
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }
    m_diag_editor = new BaseDialog(this);

    //int id_session = m_listSessions_items.key(ui->tableSessions->item(session_item->row(),0));
    //TeraData* ses_data = m_ids_sessions[id_session];
    //if (ses_data){
    session_info->setFieldValue("id_project", m_currentIdProject);
    SessionWidget* ses_widget = new SessionWidget(m_comManager, session_info, m_diag_editor);
    ses_widget->alwaysShowAssets(m_allowFileTransfers);
    ses_widget->setTestTypes(m_testTypes);
    if (m_currentSessionShowAssets)
        ses_widget->showAssets();
    if (m_currentSessionShowTests)
        ses_widget->showTests();
    ses_widget->enableEmails(m_allowEmails);

    m_diag_editor->setCentralWidget(ses_widget);

    m_diag_editor->setWindowTitle(tr("Séance"));
    //m_diag_editor->setMinimumSize(2*this->width()/3, 5*this->height()/6);
    QList<QScreen*> screens = QGuiApplication::screens();
    QSize base_size = screens.first()->size();
    m_diag_editor->setMinimumSize(3*base_size.width()/4, 3*base_size.height()/4);

    connect(ses_widget, &SessionWidget::closeRequest, m_diag_editor, &QDialog::accept);
    connect(ses_widget, &SessionWidget::assetsCountChanged, this, &SessionsListWidget::sessionAssetsCountChanged);
    connect(ses_widget, &SessionWidget::testsCountChanged, this, &SessionsListWidget::sessionTestsCountChanged);

    m_diag_editor->open();
}

void SessionsListWidget::displaySessionDetails(QTableWidgetItem *session_item, bool show_assets, bool show_tests)
{
    // Query full session information
    m_currentIdSession = m_listSessions_items.key(ui->tableSessions->item(session_item->row(),0));
    m_currentSessionShowAssets = show_assets;
    m_currentSessionShowTests = show_tests;

    QUrlQuery args;
    args.addQueryItem(WEB_QUERY_ID_SESSION, QString::number(m_currentIdSession));
    if (m_comManager)
        m_comManager->doGet(WEB_SESSIONINFO_PATH, args);
}


void SessionsListWidget::on_btnPrevCal_clicked()
{
    QDate new_date;
    new_date.setDate(ui->calMonth1->yearShown(), ui->calMonth1->monthShown(), 1);
    new_date = new_date.addMonths(-1);

    updateCalendars(new_date);
}

void SessionsListWidget::currentCalendarDateChanged(QDate current_date)
{
    // Clear current selection
    ui->tableSessions->clearSelection();

    // Temporarly set multi-selection
    QAbstractItemView::SelectionMode current_mode = ui->tableSessions->selectionMode();
    ui->tableSessions->setSelectionMode(QAbstractItemView::MultiSelection);

    // Select all the sessions in the list that fits with that date
    QTableWidgetItem* first_item = nullptr;
    //foreach(TeraData* session, m_ids_sessions){
    for(TeraData* session: std::as_const(m_ids_sessions)){
        if (session->getFieldValue("session_start_datetime").toDateTime().toLocalTime().date() == current_date){
            QTableWidgetItem* session_item = m_listSessions_items.value(session->getId());
            if (session_item){
                ui->tableSessions->selectRow(session_item->row());
                if (!first_item)
                    first_item = session_item;
            }
        }
    }

    // Resets selection mode
    ui->tableSessions->setSelectionMode(current_mode);

    // Ensure first session is visible
    if (first_item){
        ui->tableSessions->scrollToItem(first_item);
    }
}

void SessionsListWidget::currentCalendarDateActivated(QDate current_date)
{
    // Add a new session
    QDateTime session_datetime;
    session_datetime.setDate(current_date);
    QTime session_time = QTime::currentTime();
    int minutes = (session_time.minute() / 15) * 15;
    session_time.setHMS(session_datetime.time().addSecs(3600).hour(), minutes, 0);
    session_datetime.setTime(session_time);

    newSessionRequest(session_datetime);
}


void SessionsListWidget::on_btnCheckSessionTypes_clicked()
{
    for (int i=0; i<ui->lstFilters->count(); i++){
        ui->lstFilters->item(i)->setCheckState(Qt::Checked);
    }
    ui->btnUncheckSessionTypes->show();
    ui->btnCheckSessionTypes->hide();
}


void SessionsListWidget::on_btnUncheckSessionTypes_clicked()
{
    for (int i=0; i<ui->lstFilters->count(); i++){
        ui->lstFilters->item(i)->setCheckState(Qt::Unchecked);
    }
    ui->btnUncheckSessionTypes->hide();
    ui->btnCheckSessionTypes->show();
}


void SessionsListWidget::on_btnFilterSessionsTypes_clicked()
{
    ui->frameFilterSessionTypes->setVisible(ui->btnFilterSessionsTypes->isChecked());
}


void SessionsListWidget::on_tableSessions_itemDoubleClicked(QTableWidgetItem *item)
{
    displaySessionDetails(item);
}

void SessionsListWidget::on_btnAssetsBrowser_clicked()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }
    m_diag_editor = new BaseDialog(this);

    AssetsWidget* asset_widget = new AssetsWidget(m_comManager, m_diag_editor);

    switch(m_viewMode){
        case VIEW_NONE:
            qWarning() << "None view selected - unable to display assets!";
            break;
        case VIEW_PARTICIPANT_SESSIONS:
            asset_widget->displayAssetsForParticipant(m_currentId);
            break;
        case VIEW_USER_SESSIONS:
            qWarning() << "Assets display for user not implemented yet!";
            // asset_widget->displayAssetsForUser(m_currentId);
            break;
        case VIEW_DEVICE_SESSIONS:
            qWarning() << "Assets display for device not implemented yet!";
            // asset_widget->displayAssetsForDevice(m_currentId);
            break;
    }

    m_diag_editor->setCentralWidget(asset_widget);

    m_diag_editor->setWindowTitle(tr("Explorateur de données"));
    m_diag_editor->setMinimumSize(3*this->width()/4, 3*this->height()/4);

    m_diag_editor->open();
}


void SessionsListWidget::on_btnTestsBrowser_clicked()
{
    if (m_diag_editor){
        m_diag_editor->deleteLater();
    }
    m_diag_editor = new BaseDialog(this);

    TestsWidget* test_widget = new TestsWidget(m_comManager, m_diag_editor);
    switch(m_viewMode){
        case VIEW_NONE:
            qWarning() << "None view selected - unable to display assets!";
            break;
        case VIEW_PARTICIPANT_SESSIONS:
            test_widget->displayTestsForParticipant(m_currentId);
            break;
        case VIEW_USER_SESSIONS:
            qWarning() << "Tests display for user not implemented yet!";
            // test_widget->displayTestsForUser(m_currentId);
            break;
        case VIEW_DEVICE_SESSIONS:
            qWarning() << "Tests display for device not implemented yet!";
            // test_widget->displayTestsForDevice(m_currentId);
            break;
    }

    m_diag_editor->setCentralWidget(test_widget);

    m_diag_editor->setWindowTitle(tr("Explorateur d'évaluations"));
    m_diag_editor->setMinimumSize(3*this->width()/4, 3*this->height()/4);

    m_diag_editor->open();
}

