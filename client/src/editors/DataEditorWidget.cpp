#include "DataEditorWidget.h"
#include <QApplication>
#include "GlobalMessageBox.h"

DataEditorWidget::DataEditorWidget(ComManager *comMan, const TeraData *data, QWidget *parent) :
    QWidget(parent),
    m_undoing(false)
{
    m_data = nullptr;
    m_editToggle = nullptr;
    m_frameSave = nullptr;
    m_saveButton = nullptr;
    m_cancelButton = nullptr;
    m_mainForm = nullptr;

    DataEditorWidget::setData(data);
    DataEditorWidget::setLoading();

    // Set ComManager pointer
    m_comManager = comMan;

    connect(m_comManager, &ComManager::networkError, this, &DataEditorWidget::comDataError);
    connect(m_comManager, &ComManager::queryResultsOK, this, &DataEditorWidget::queryDataReplyOK);
    connect(m_comManager, &ComManager::postResultsOK, this, & DataEditorWidget::postDataReplyOK);
    connect(m_comManager, &ComManager::deleteResultsOK, this, &DataEditorWidget::deleteDataReplyOK);

}

DataEditorWidget::~DataEditorWidget(){
    if (m_data)
        m_data->deleteLater();

    while (QApplication::overrideCursor())
        QApplication::restoreOverrideCursor();
}


void DataEditorWidget::setData(const TeraData* data)
{
    if (m_data){
        m_data->deleteLater();
        m_data = nullptr;
    }

    if (data != nullptr){
        m_data = new TeraData(*data);
        // Start editing if new data
        if (dataIsNew()){
            editToggleClicked();
        }
    }

}

void DataEditorWidget::setLimited(bool limited){
    m_limited = limited;

    if (m_frameSave && m_limited){
        m_frameSave->hide();
        if (m_cancelButton)
            m_cancelButton->hide();
    }

    if (m_editToggle && !dataIsNew()){
        m_editToggle->setVisible(!limited);
    }
    updateControlsState();
}

TeraData *DataEditorWidget::getData()
{
    return m_data;
}


/*void DataEditorWidget::setEditing(bool enabled){
    if (m_editState==STATE_WAITING)
        return; // Dont do anything if waiting

    if (m_editState==STATE_READY && enabled){
        setEditing();
        return;
    }

    if (m_editState==STATE_EDITING && !enabled){
        // Exit editing mode
        if (!m_undoing){
            // Check if data is valid
            if (validateData()){
                setWaiting();
                saveData();
            }else{
                setEditing(); // Return in editing mode
            }
        }else{
            setReady();
        }
        m_undoing = false;
    }
}*/

void DataEditorWidget::setReady(){
    if (isReady())
        return;

    m_editState = STATE_READY;
    setEnabled(true);
    setVisible(true);
    while (QApplication::overrideCursor())
        QApplication::restoreOverrideCursor();
    updateControlsState();
    m_undoing=false;
    emit stateReady();
}

void DataEditorWidget::setEditing(){
    if (isEditing())
        return;
    setEnabled(true);
    setVisible(true);
    while (QApplication::overrideCursor())
        QApplication::restoreOverrideCursor();
    m_editState = STATE_EDITING;
    updateControlsState();
    emit stateEditing();
}

void DataEditorWidget::setWaiting(){
    if (isWaiting() || isLoading())
        return;

    m_editState = STATE_WAITING;
    setEnabled(false);
    updateControlsState();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    emit stateWaiting();
}

void DataEditorWidget::setLoading(){
    if (isLoading())
        return;

    m_editState = STATE_LOADING;
    setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    setVisible(false);
    emit stateLoading();
}

void DataEditorWidget::refreshData(){
    updateFieldsValue();
}

void DataEditorWidget::disconnectSignals()
{
    disconnect(m_comManager, nullptr, this, nullptr);
}

void DataEditorWidget::queryDataRequest(const QString &path, const QUrlQuery &query_args)
{
    QString query_name = getQueryDataName(path, query_args);
    m_requests.append(query_name);
    m_comManager->doGet(path, query_args);
    setWaiting();
}

bool DataEditorWidget::hasPendingDataRequests()
{
    return !m_requests.isEmpty();
}

void DataEditorWidget::postDataRequest(const QString &path, const QString &json_data)
{
    QString query_name = getQueryDataName(path, QUrlQuery());
    m_requests.append(query_name);
    m_comManager->doPost(path, json_data);
    setWaiting();
}

void DataEditorWidget::deleteDataRequest(const QString &path, const int &id)
{
    QString query_name = getQueryDataName(path, QUrlQuery("del_id=" + QString::number(id)));
    m_requests.append(query_name);
    m_comManager->doDelete(path, id);
    setWaiting();
}

void DataEditorWidget::downloadDataRequest(const QString &save_path, const QString &path, const QUrlQuery &query_args)
{
    QString query_name = getQueryDataName(path, query_args);
    m_requests.append(query_name);
    m_comManager->doDownload(save_path, path, query_args);
    setWaiting();
}

ComManager *DataEditorWidget::getComManager()
{
    return m_comManager;
}

void DataEditorWidget::setEditorControls(TeraForm* mainForm, QPushButton *editToggle, QFrame *frameSave, QPushButton *saveButton, QPushButton *cancelButton)
{
    m_mainForm = mainForm;
    m_editToggle = editToggle;
    m_frameSave = frameSave;
    m_saveButton = saveButton;
    m_cancelButton = cancelButton;

    // Set initial state
    if (m_frameSave){
        m_frameSave->hide();
    }
    if (m_mainForm){
        m_mainForm->setDisabled(true);
    }

    // Connect signals
    if (m_cancelButton) connect(m_cancelButton, &QPushButton::clicked, this, &DataEditorWidget::undoButtonClicked);
    if (m_saveButton) connect(m_saveButton, &QPushButton::clicked, this, &DataEditorWidget::saveButtonClicked);
    if (m_editToggle) connect(m_editToggle, &QPushButton::clicked, this, &DataEditorWidget::editToggleClicked);

}

QString DataEditorWidget::getQueryDataName(const QString &path, const QUrlQuery &query_args)
{
    QString query_name = path;
    if (!query_args.isEmpty())
        query_name += "?" + query_args.toString();
    return query_name;
}

QComboBox *DataEditorWidget::buildRolesComboBox()
{

    QComboBox* item_roles = new QComboBox();
    item_roles->addItem(getRoleName(""), "");
    item_roles->addItem(getRoleName("admin"), "admin");
    item_roles->addItem(getRoleName("user"), "user");
    item_roles->setCurrentIndex(0);

    return item_roles;

}

QComboBox *DataEditorWidget::buildRolesComboBox(const QMap<int, QString> &roles)
{
    QComboBox* item_roles = new QComboBox();
    item_roles->addItem(getRoleName(""), ""); // Always add "empty role"

    for(int i=0; i<roles.count(); i++){
        item_roles->addItem(getRoleName(roles[roles.keys().at(i)]), roles.keys().at(i));
    }
    item_roles->setCurrentIndex(0);

    return item_roles;
}

QString DataEditorWidget::getRoleName(const QString &role)
{
    if (role == "admin")
        return tr("Administrateur");
    if (role == "user")
        return tr("Utilisateur");
    if (role == "")
        return tr("Aucun rôle");
    return role;
}

bool DataEditorWidget::isReady(){
    return m_editState == STATE_READY;
}

bool DataEditorWidget::isEditing(){
    return m_editState == STATE_EDITING;
}

bool DataEditorWidget::isWaiting(){
    return m_editState == STATE_WAITING;
}

bool DataEditorWidget::isLoading(){
    return m_editState == STATE_LOADING;
}

bool DataEditorWidget::isWaitingOrLoading()
{
    return isLoading() || isWaiting();
}

void DataEditorWidget::undoOrDeleteData(){
    //if (m_editState==STATE_EDITING){
        // If editing, undo the changes
        if (dataIsNew())
            // If a new dataset, undo = remove it
            deleteData();
        else
            undoData();

    /*}else{
        // Delete the data
        deleteData();
    }*/
}

void DataEditorWidget::queryDataReplyOK(const QString &path, const QUrlQuery &query_args)
{
    QString query_name = getQueryDataName(path, query_args);
    m_requests.removeOne(query_name);
    if (!hasPendingDataRequests() && isLoading())
        updateFieldsValue();

    if (m_requests.isEmpty())
        setEditing();
}

void DataEditorWidget::postDataReplyOK(const QString &path)
{
    QString query_name = getQueryDataName(path, QUrlQuery());
    m_requests.removeOne(query_name);
    if (!hasPendingDataRequests())
        updateFieldsValue();

    if (m_requests.isEmpty())
        setReady();

}

void DataEditorWidget::deleteDataReplyOK(const QString &path, const int &id)
{
    QString query_name = getQueryDataName(path, QUrlQuery("del_id=" + QString::number(id)));
    m_requests.removeOne(query_name);
    if (!hasPendingDataRequests())
        updateFieldsValue();

    if (m_requests.isEmpty())
        setReady();
}

void DataEditorWidget::comDataError(QNetworkReply::NetworkError error, QString error_str)
{
    Q_UNUSED(error)
    Q_UNUSED(error_str)
    setReady();
}

void DataEditorWidget::editToggleClicked()
{
    if (m_frameSave){
        m_frameSave->show();
        //m_editToggle->setDisabled(true);
        if (m_editToggle)
            m_editToggle->hide();
        if (m_mainForm){
            m_mainForm->setDisabled(false);
        }
    }
}

void DataEditorWidget::saveButtonClicked()
{
    if (!validateData()){
        QStringList invalids = m_mainForm->getInvalidFormDataLabels();

        if (!invalids.isEmpty()){
            QString msg = tr("Les champs suivants doivent être complétés:") +" <ul>";
            for (QString field:invalids){
                msg += "<li>" + field + "</li>";
            }
            msg += "</ul>";
            GlobalMessageBox msgbox(this);
            msgbox.showError(tr("Champs invalides"), msg);
        }
        return;
    }

     saveData(true);
     m_frameSave->hide();
     //m_editToggle->setDisabled(false);
     if (m_editToggle)
         m_editToggle->show();
     if (m_mainForm){
         m_mainForm->setDisabled(true);
     }

}

void DataEditorWidget::undoButtonClicked()
{
    undoOrDeleteData();

    /*if (parent())
        emit closeRequest();*/

    m_frameSave->hide();
    //m_editToggle->setDisabled(false);
    if (m_editToggle)
        m_editToggle->show();
    if (m_mainForm){
        m_mainForm->setDisabled(true);
    }
}

void DataEditorWidget::undoData(){
    m_undoing=true;
    updateFieldsValue();
    setReady();
}

void DataEditorWidget::deleteData()
{
    emit dataWasDeleted();
}

bool DataEditorWidget::dataIsNew()
{
    if (m_data){
        return m_data->isNew();
    }
    return true;
}
