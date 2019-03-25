#include "UserWidget.h"
#include "ui_UserWidget.h"

#include <QtSerialPort/QSerialPortInfo>
#include <QtMultimedia/QCameraInfo>
#include <QtMultimedia/QCamera>
#include <QInputDialog>

#include <QtMultimedia/QAudioDeviceInfo>


UserWidget::UserWidget(ComManager *comMan, const TeraUser &data, QWidget *parent) :
    DataEditorWidget(comMan, parent),
    ui(new Ui::UserWidget),
    m_data(nullptr)
{
    setData(data);

    if (parent){
        ui->setupUi(parent);
        ui->btnEdit->hide();
        ui->btnDelete->hide();
    }else {
        ui->setupUi(this);
    }
    setAttribute(Qt::WA_StyledBackground); //Required to set a background image

    /*
    if (is_kit){
        m_data_type = TERADATA_KIT;
        // Remove usergroup list
        lblUserGroup->setVisible(false);
        cmbUserGroup->setVisible(false);
        // Remove enabled check
        chkEnabled->setVisible(false);
        chkAdmin->setVisible(false);
        //lblEnabled->setVisible(false);
        // Ensure kit is always enabled
        chkEnabled->setChecked(true);
        lblEmail->setVisible(false);
        txtEmail->setVisible(false);
        txtFirstName->setVisible(false);
        lblFirstName->setVisible(false);
        lblDesc->setVisible(true);
        txtDesc->setVisible(true);
    }else{
        m_data_type = TERADATA_USER;
        lblDesc->setVisible(false);
        txtDesc->setVisible(false);
        tabMain->removeTab(1); // Remove history tab
    }*/

    hideValidationIcons();
    //ui->txtCPassword->setVisible(false);


    // Connect signals and slots
    connectSignals();

    // UI signals
    /*connect(chkCamControl1,SIGNAL(stateChanged(int)),this,SLOT(componentChecked(int)));
    connect(chkONVIFAbsolute1,SIGNAL(stateChanged(int)),this,SLOT(componentChecked(int)));
    connect(chkWebRTC,SIGNAL(stateChanged(int)),this,SLOT(componentChecked(int)));
    connect(chkVirtualCam,SIGNAL(stateChanged(int)),this,SLOT(componentChecked(int)));
    connect(chk2ndAudioSrc,SIGNAL(stateChanged(int)),this,SLOT(componentChecked(int)));
    connect(chk2ndVideoSrc,SIGNAL(stateChanged(int)),this,SLOT(componentChecked(int)));
    connect(chkTeraWeb, SIGNAL(stateChanged(int)),this,SLOT(componentChecked(int)));
    connect(chkExternalPrograms, SIGNAL(stateChanged(int)),this,SLOT(componentChecked(int)));

    connect(btnControlPass1,SIGNAL(clicked(bool)),this,SLOT(showPassword(bool)));

    connect(cmbCam1Res,SIGNAL(currentIndexChanged(int)),this,SLOT(comboItemChanged()));
    connect(cmbControl1,SIGNAL(currentIndexChanged(int)),this,SLOT(comboItemChanged()));
    connect(cmbVirtualCam,SIGNAL(currentIndexChanged(int)),this,SLOT(comboItemChanged()));*/

    // Update accessible controls
    updateAccessibleControls();

    updateFieldsValue();

}

UserWidget::~UserWidget()
{    
    if (m_data)
        m_data->deleteLater();
    if (ui)
        delete ui;
}

void UserWidget::setData(const TeraUser &data){
    if (m_data)
        m_data->deleteLater();

    m_data = new TeraUser(data);

    // Query forms definition
    m_comManager->doQuery(WEB_DEFINITIONS_PATH, WEB_DEFINITIONS_PROFILE);
    m_comManager->doQuery(WEB_DEFINITIONS_PATH, WEB_DEFINITIONS_USER);

    /*
    if (m_data_type==TERADATA_KIT){
        m_data->setUserType(UserInfo::USERTYPE_KIT);
        // Query logs (if it is a kit)

        QList<SearchCriteria> scl;
        SearchCriteria sc;
        sc.search_type = SearchCriteria::SearchUserID;
        sc.criteria.append(m_data->id());
        scl.append(sc);
        emit dataLoadingRequest(TERADATA_KITLOG,scl);

        setLoading();
    }
    if (m_data_type==TERADATA_USER)
        m_data->setUserType(UserInfo::USERTYPE_USER);

    //emit listRequest(TERADATA_USERGROUP);
    tabMain->setVisible(true);

    // Query global user list
    if (m_users.isEmpty()){
        QList<QVariant> args;
        args.append(QString(""));
        emit cloudRequest(CloudCom::CloudComDict::GET_USER_ACCOUNT,args);
        setLoading();
    }else{
        updateFieldsValue();
        setReady();
    }*/

}

TeraUser* UserWidget::getData()
{
    return m_data;
}

bool UserWidget::dataIsNew(){
    if (m_data->getId()==0)
        return true;
    else
        return false;
}

void UserWidget::saveData(bool signal){
    //m_data->setUser(txtUserName->text());
/*
    if (m_data_type==TERADATA_USER){
        m_data->setUserGroup(cmbUserGroup->itemData(cmbUserGroup->currentIndex()).toUInt());
        m_data->setUserType(UserInfo::USERTYPE_USER);
        m_data->setEnabled(chkEnabled->checkState()==Qt::Checked);
        m_data->setSuperAdmin(chkAdmin->checkState()==Qt::Checked);
        m_data->setFirstName(txtFirstName->text());
        m_data->setLastName(txtLastName->text());
    }
    else{
        m_data->setUserType(UserInfo::USERTYPE_KIT);
        m_data->setEnabled(true); // Kit always are enabled
        m_data->setFirstName("");
        m_data->setLastName(txtLastName->text());
        m_data->setDesc(txtDesc->toPlainText());
    }

    */
    if (signal)
        emit dataWasChanged();

    if (parent())
        emit closeRequest(); // Ask to close that editor
}


void UserWidget::updateControlsState(){
    // Controls editing set
    bool edit = (m_editState==DataEditorWidget::STATE_EDITING);

    //txtFirstName->setEnabled(false); // Always disabled, since managed globally
    //txtLastName->setEnabled(false);
    /*ui->txtFirstName->setEnabled(edit);
    ui->txtLastName->setEnabled(edit);
    ui->txtCPassword->setEnabled(edit);
    ui->txtPassword->setEnabled(edit);
    ui->txtEmail->setEnabled(edit);
    ui->txtUserName->setEnabled(edit);
    ui->txtDesc->setEnabled(edit);

    if (m_data){
        if (dataIsNew()){
            ui->txtUserName->clear();
        }
    }

    ui->chkEnabled->setEnabled(edit);
    ui->chkAdmin->setEnabled(edit);*/

    if (edit)
        ui->tabMain->setVisible(true);

   /* if (m_data_type == TERADATA_USER){
        // Check if editing current user - disable user group change
        if (m_data && m_loggedUser->id()==m_data->id())
            cmbUserGroup->setEnabled(false);
        else
            cmbUserGroup->setEnabled(edit);
    }*/

    // Buttons update
    ui->btnEdit->setEnabled(isReady());
    ui->btnDelete->setEnabled(isReady());
    if (m_editState==STATE_EDITING){
        ui->btnSave->setVisible(!isReady());
        ui->btnUndo->setVisible(!isReady());
    }else{
        ui->btnSave->setVisible(false);
        ui->btnUndo->setVisible(false);
    }


}

void UserWidget::updateFieldsValue(){
    if (m_data){
        /*if (m_data_type==TERADATA_USER){
            txtFirstName->setText(m_data->firstname());
            txtLastName->setText(m_data->lastname());
        }else{
            txtLastName->setText(m_data->name());
        }*/
        /*ui->txtFirstName->setText(m_data->getFirstName());
        ui->txtLastName->setText(m_data->getLastName());*/

       /* chkEnabled->setChecked(m_data->enabled());
        chkAdmin->setChecked(m_data->isSuperAdmin());
        //txtUserName->setText(m_data->username()); // Updated when receiving profile
        txtPassword->clear();
        txtCPassword->clear();

        if (!dataIsNew())
            lblLastOnline->setText(m_data->lastOnline().toString());
        else
            lblLastOnline->setText("");

        setTitle(m_data->name());
        // Select usergroup
        if (m_data_type == TERADATA_USER){
            int index = cmbUserGroup->findData(m_data->id_usergroup());
            cmbUserGroup->setCurrentIndex(index);
        }

        // Load profile from user
        loadGlobalUser(m_data->uuid());

        txtDesc->setPlainText(m_data->desc());*/

    }
}

void UserWidget::updateAccessibleControls(){
    /*AccessInfo access = m_loggedUser->getAccess(TERA_ACCESS_USERS);
    btnDelete->setVisible(access.canDelete());
    btnEdit->setVisible(access.canEdit());

    access = m_loggedUser->getAccess(TERA_ACCESS_TECH);
    frmProfile->setVisible(access.canRead());
    txtProfile->setEnabled(access.canEdit());

    chkAdmin->setVisible(m_loggedUser->isSuperAdmin() && m_data_type==TERADATA_USER);*/

}

void UserWidget::deleteData(){

}

void UserWidget::setWaiting(){
    ui->btnDelete->setEnabled(false);
    ui->btnEdit->setEnabled(false);
    DataEditorWidget::setWaiting();
}

void UserWidget::setReady(){
    hideValidationIcons();
/*    if (!_limited)
        btnDelete->setEnabled(true);
    else
        btnDelete->setEnabled(false);*/
    ui->btnEdit->setEnabled(true);

    DataEditorWidget::setReady();
}

bool UserWidget::validateData(){
   /* QList<Data_Validation> errors;

    hideValidationIcons();

    if (!m_data)
        return false;

    // Create a local copy of the data, in case of a user undo
    UserInfo* data = new UserInfo(*m_data);

    saveData(false);
    errors = m_data->validate();

    if (txtPassword->text()!=""){
        if (txtPassword->text()!=txtCPassword->text())
            errors.append(VALIDATE_PASSWORD);
    }

    if (errors.contains(VALIDATE_ID)){
        // Check if that error is OK, since that can happens if creating global user at the same time
        if (dataIsNew())
            errors.removeAll(VALIDATE_ID);
    }

    if (dataIsNew() && txtPassword->text()==""){
        errors.append(VALIDATE_PASSWORD);
    }
    // Copy old data
    *m_data = *data;

    delete data;

    if (errors.count()==0)
        return true;

    for (int i=0; i<errors.count(); i++){
        switch (errors.at(i)){
        case VALIDATE_FIRSTNAME:
            icoFirstName->setVisible(true);
            break;
        case VALIDATE_LASTNAME:
        case VALIDATE_NAME:
            icoLastName->setVisible(true);
            break;
        case VALIDATE_USERGROUP:
            icoUserGroup->setVisible(true);
            break;
        case VALIDATE_ID:
            icoLinked->setVisible(true);
            break;
        case VALIDATE_PASSWORD:
            icoPassword->setVisible(true);
            break;
        default:
            break;
        }
    }*/

    return false;
}

void UserWidget::hideValidationIcons(){

}

void UserWidget::objectDefReceived(const QString& def, const QString &type)
{
    if (type == WEB_DEFINITIONS_PROFILE)
        ui->wdgProfile->buildUiFromStructure(def);

    if (type == WEB_DEFINITIONS_USER)
        ui->wdgUser->buildUiFromStructure(def);
}

void UserWidget::hideProfileValidationIcons(){
    /*ui->valControlAdr1->setVisible(false);
    ui->valControlPort1->setVisible(false);*/
}

bool UserWidget::validateProfile(){
    // Check if profile in the "simplified" profile is valid
    bool errors = false;

    return !errors;
}

void UserWidget::buildProfileFromUI(){

}

void UserWidget::setLimited(bool limited){
    m_limited = limited;

    setEditing();

}

void UserWidget::connectSignals()
{
    connect(ui->btnEdit, &QPushButton::clicked, this, &UserWidget::btnEdit_clicked);
    connect(ui->btnDelete, &QPushButton::clicked, this, &UserWidget::btnDelete_clicked);
    connect(ui->btnUndo, &QPushButton::clicked, this, &UserWidget::btnUndo_clicked);
    connect(ui->btnSave, &QPushButton::clicked, this, &UserWidget::btnSave_clicked);
    //connect(ui->txtPassword, &QLineEdit::textChanged, this, &UserWidget::txtPassword_textChanged);

    connect(m_comManager, &ComManager::objectDefinitionReceived, this, &UserWidget::objectDefReceived);

}
void UserWidget::btnEdit_clicked()
{
    setEditing(true);
}

void UserWidget::btnDelete_clicked()
{
    undoOrDeleteData();
}

void UserWidget::btnSave_clicked()
{
    /* if (!dataIsNew())//btnExisting->isChecked())
         m_data->setUuid(m_profile.getUUID());

     if (!validateData())
         return;


     // If new account, wait until it is created before saving. Otherwise, save it now.
     if (!dataIsNew())
         setEditing(false); // This will save TeRA related stuff
         */

}

void UserWidget::txtPassword_textChanged(const QString &new_pass)
{
    Q_UNUSED(new_pass)
    /*if (ui->txtPassword->text().isEmpty()){
        ui->txtCPassword->setVisible(false);
        ui->txtCPassword->clear();
    }else{
        ui->txtCPassword->setVisible(true);
    }*/
}

void UserWidget::btnUndo_clicked()
{
    undoOrDeleteData();

    if (parent())
        emit closeRequest();


}
