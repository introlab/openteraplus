#include "SharedObject.h"
#include <QScreen>
#include <QGuiApplication>

SharedObject::SharedObject(QObject *parent) : QObject(parent)
{
    m_cameraIndex = -1;
    m_ptzCameraDriver = nullptr;
    m_imgSettingsDialog = nullptr;

    // Create Screen Controller
    // TODO: Don't always create it?
    QScreen* target_screen;
    if (QGuiApplication::screens().count() > 1){
        // More than one screen - always screen 2 that is shared using screen sharing
        target_screen = QGuiApplication::screens().at(1);
    }else{
        // One screen, share primary screen
        target_screen = QGuiApplication::primaryScreen();
    }
    m_screenController = new ScreenController(target_screen, parent);

    m_camPTZName = "";
}

SharedObject::~SharedObject()
{
    if (m_ptzCameraDriver){
        stopPTZCameraDriver();
    }

    if (m_screenController){
        m_screenController->deleteLater();
    }
}

void SharedObject::startPTZCameraDriver(const int &camera_src, const QString &camera_name, const QString &hostname, const int port, const QString &user, const QString &password)
{
    if (camera_src == 0){ // TODO - Better handle camera types
        // Vivotek
        m_ptzCameraDriver = new Vivotek8111();
        m_ptzCameraDriver->init(hostname, port, user, password);

    }
    if (m_ptzCameraDriver){
        bool canZoom = m_ptzCameraDriver->hasCameraFunction(CameraInfo::CIF_ZOOM_ABS) || m_ptzCameraDriver->hasCameraFunction(CameraInfo::CIF_ZOOM_REL);
        //bool canPtz = m_ptzCameraDriver->hasCameraFunction(CameraInfo::CIF_PAN_TILT_ABS) || m_ptzCameraDriver->hasCameraFunction(CameraInfo::CIF_PAN_TILT_REL) || m_ptzCameraDriver->hasCameraFunction(CameraInfo::CIF_POINT_N_CLICK);
        bool hasPresets = m_ptzCameraDriver->hasCameraFunction(CameraInfo::CIF_PRESET_POS);
        bool hasSettings = m_ptzCameraDriver->hasCameraFunction(CameraInfo::CIF_IMAGE_SETTINGS);
        m_camPTZName = camera_name;
        setPTZCapabilities(canZoom, hasPresets, hasSettings);
    }

}

void SharedObject::stopPTZCameraDriver()
{
    if (m_ptzCameraDriver){
        m_ptzCameraDriver->deleteLater();
        m_ptzCameraDriver = nullptr;
    }
    setPTZCapabilities(false, false, false);
}

ICameraDriver *SharedObject::getPTZCameraDriver() const
{
    return m_ptzCameraDriver;
}

void SharedObject::setContactInformation(const QString &name, const QString &uuid)
{
    m_userName = name;
    m_userUUID = uuid;
}

void SharedObject::setCurrentCameraName(const QString &name){
    m_cameraName = name;
}

void SharedObject::setCurrentAudioSrcName(const QString &name){
    m_audioName = name;
}

void SharedObject::setSecondVideoName(const QString &name){
    m_2ndVideoName = name;
}

void SharedObject::setPTZCapabilities(const bool &zoom, const bool &presets, const bool &settings)
{
    m_camCanZoom = zoom;
    m_camHasPresets = presets;
    m_camHasSettings = settings;
}

void SharedObject::removeVideoSource(const QString &name)
{
    emit videoSourceRemoved(name);
}

void SharedObject::setSecondAudioSrcName(const QString &name){
    m_2ndAudioName = name;
}

void SharedObject::setCurrentCameraIndex(const int &index){
    m_cameraIndex = index;
}

QString SharedObject::getContactInformation()
{
    //Will update client with info when requested.
    //sendContactInformation(m_userName,m_userUUID);

    return serializeContactInfo();
}

void SharedObject::zoomInClicked(QString uuid)
{
    //qDebug() << "Zoom In! UUID = " << uuid;
    if (m_ptzCameraDriver){
        m_ptzCameraDriver->zoomIn();
    }
    emit zoomIn(uuid);
}

void SharedObject::zoomOutClicked(QString uuid)
{
    //qDebug() << "Zoom Out! UUID = " << uuid;
    if (m_ptzCameraDriver){
        m_ptzCameraDriver->zoomOut();
    }
    emit zoomOut(uuid);
}

void SharedObject::zoomMinClicked(QString uuid)
{
    //qDebug() << "Zoom Min! UUID = " << uuid;
    if (m_ptzCameraDriver){
        m_ptzCameraDriver->zoomMin();
    }
    emit zoomMin(uuid);
}

void SharedObject::zoomMaxClicked(QString uuid)
{
    //qDebug() << "Zoom Max! UUID = " << uuid;
    if (m_ptzCameraDriver){
        m_ptzCameraDriver->zoomMax();
    }
    emit zoomMax(uuid);
}

void SharedObject::camSettingsClicked(QString uuid)
{
    qDebug() << "SharedObject -> Cam Settings! UUID = " << uuid << m_userUUID;
    if (m_ptzCameraDriver){
        if (uuid == m_userUUID){
            // Local settings
            m_ptzCameraDriver->showImageSettingsDialog();
        }else{
            // Send current settings value
            sendCameraSettings(uuid, *m_ptzCameraDriver->getCameraImageSettings());
        }
    }
    //emit camSettings(uuid);
}

void SharedObject::gotoPresetClicked(QString uuid, int preset)
{
    //qDebug() << "SharedObject -> Goto Preset UUID = " << uuid << ", position = " << preset;
    if (m_ptzCameraDriver){
        m_ptzCameraDriver->gotoPresetID(preset);
    }
    emit gotoPreset(uuid, preset);
}

void SharedObject::setPresetClicked(QString uuid, int preset)
{
    if (m_ptzCameraDriver){
        m_ptzCameraDriver->setPresetID(preset);
    }
    emit setPreset(uuid, preset);
}

void SharedObject::imageClicked(QString uuid, int x, int y, int w, int h)
{
    qDebug() << "SharedObject -> Click UUID = " << uuid << ", x = " << x << " y = " << y << " width = " << w << " height = " << h;
    if (m_ptzCameraDriver){
        if (m_localMirror) // Image mirrored - invert x
            x = w-x;

        m_ptzCameraDriver->setPointNClick(QPoint(x,y), QSize(w,h));
    }
    emit move(uuid, x, y, w, h);
}

void SharedObject::screenClicked(QString uuid, int x, int y, int w, int h)
{
    qDebug() << "SharedObject -> Screen Clicked UUID = " << uuid << ", x = " << x << " y = " << y << " width = " << w << " height = " << h;
    if (uuid == m_userUUID)
        m_screenController->doClick(x, y, w, h);
}

void SharedObject::mouseUpEvent(QString uuid, int x, int y, int w, int h)
{
    if (uuid == m_userUUID)
        m_screenController->doMouseUp(x, y, w, h);
}

void SharedObject::mouseDownEvent(QString uuid, int x, int y, int w, int h)
{
    if (uuid == m_userUUID)
        m_screenController->doMouseDown(x, y, w, h);
}

void SharedObject::mouseMoveEvent(QString uuid, int x, int y, int w, int h)
{
    if (uuid == m_userUUID)
        m_screenController->doMouseMove(x, y, w, h);
}

QString SharedObject::getCurrentVideoSource()
{
    return serializeVideoSource();
    //Will update client with info when requested.
    //sendCurrentVideoSource(m_cameraName, m_cameraIndex);
}

QString SharedObject::getCurrentAudioSource()
{
    return serializeAudioSource();
    //Will update client with info when requested.
    //sendCurrentAudioSource();
}


void SharedObject::getSecondSources(){
    sendSecondSources();
}

void SharedObject::newRemoteConnection(){
    emit newRemoteStream();
}

void SharedObject::cameraChanged(QString name, int index){
    m_cameraName = name;
    m_cameraIndex = index;

    emit currentCameraWasChanged();
}


void SharedObject::sendContactInformation()
{
    QString contact_info = serializeContactInfo();
    qDebug() << "js: Sending : " << QString(contact_info);
    emit newContactInformation(contact_info);

}

void SharedObject::sendCurrentVideoSource(){


    QString video_src = serializeVideoSource();
   // qDebug() << "js: Sending Video Source: " << video_src;

    emit newVideoSource(video_src);
}

QString SharedObject::serializeContactInfo()
{
    //Create JSON Object for contact
    QJsonObject myObject
    {
        {"name", m_userName},
        {"uuid", m_userUUID}
    };

    QJsonDocument doc(myObject);

    QString rval = doc.toJson(QJsonDocument::Compact);

    return rval;

}

QString SharedObject::serializeAudioSource()
{
    QJsonObject myObject
    {
        {"name", m_audioName}
    };

    QJsonDocument doc(myObject);

    return doc.toJson(QJsonDocument::Compact);
}

QString SharedObject::serializeVideoSource()
{
    //Create JSON Object for contact
    QJsonObject myObject
    {
        {"name", m_cameraName},
        {"index", m_cameraIndex}
    };

    QJsonDocument doc(myObject);
    return doc.toJson(QJsonDocument::Compact);
}

QString SharedObject::serialize2ndSources()
{
    //Create JSON Object for contact
    QJsonObject myObject
    {
        {"audio", m_2ndAudioName},
        {"video", m_2ndVideoName}
    };

    QJsonDocument doc(myObject);
    return doc.toJson(QJsonDocument::Compact);
}

QString SharedObject::serializePtzCapabilities()
{
    //Create JSON Object for contact
    QJsonObject myObject //  &zoom, const bool &presets, const bool &settings
    {
        {"camera", m_camPTZName},
        {"zoom", m_camCanZoom},
        {"presets", m_camHasPresets},
        {"settings", m_camHasSettings}
    };

    QJsonDocument doc(myObject);
    return doc.toJson(QJsonDocument::Compact);
}


void SharedObject::sendCurrentAudioSource(){
    QString audio_src = serializeAudioSource();

    //qDebug() << "js: Sending : " << audio_src;

    emit newAudioSource(audio_src);
}

void SharedObject::sendSecondSources(){
    //Create JSON Object for contact
    QJsonObject myObject
    {
        {"audio", m_2ndAudioName},
        {"video", m_2ndVideoName}
    };

    QJsonDocument doc(myObject);

    qDebug() << "js: Sending : " << QString(doc.toJson(QJsonDocument::Compact));

    emit newSecondSources(QString(doc.toJson(QJsonDocument::Compact)));
}

void SharedObject::sendCameraSettings(const QString &uuid, CameraImageSettings &settings, const QString &owner){
    //Create JSON Object for contact
    QJsonObject myObject = settings.toJSON();
    QString owner_id = owner;
    if (owner.isEmpty())
        owner_id = m_userUUID;

    myObject.insert("owner_uuid",QJsonValue::fromVariant(owner_id));
    myObject.insert("uuid", QJsonValue::fromVariant(uuid));
    myObject.insert("msgtype",QJsonValue::fromVariant("setCamSettings"));

    QJsonDocument doc(myObject);

    qDebug() << "js: Sending : " << QString(doc.toJson(QJsonDocument::Compact));

    emit newDataForward(QString(doc.toJson(QJsonDocument::Compact)));
}

void SharedObject::dataForwardReceived(QString json){
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    QJsonObject obj;
    // Convert message
    if (!doc.isNull()){
        if (doc.isObject()){
            obj = doc.object();
        }
    }

    if (!obj.isEmpty()){
        QString msg_type = obj.value("msgtype").toString();
        if (msg_type == "setCamSettings"){

            qDebug() << "Shared Object - Request to set cam settings";
            CameraImageSettings settings;
            settings.fromJSON(obj);

            if (obj.value("owner_uuid").toString() == m_userUUID && m_ptzCameraDriver)
                m_ptzCameraDriver->setImageSettings(settings);
            else{
                // TODO: Display cam settings dialog detached from ptzDriver
                if (m_imgSettingsDialog)
                    m_imgSettingsDialog->deleteLater();

                m_imgSettingsDialog = new CamImageSettingsDialog();
                m_imgSettingsDialog->setFromCameraInfo(settings);
                m_imgSettingsDialog->setTargetUUID(obj.value("owner_uuid").toString());

                connect(m_imgSettingsDialog, &CamImageSettingsDialog::settingsChanged, this, &SharedObject::camImageSettingsChanged);
                m_imgSettingsDialog->setModal(true);
                m_imgSettingsDialog->show();
            }
        }

    }
}

void SharedObject::camImageSettingsChanged(){
    CameraImageSettings settings =  m_imgSettingsDialog->getCurrentImageSettings();
    sendCameraSettings(m_imgSettingsDialog->getTargetUUID(), settings, m_imgSettingsDialog->getTargetUUID());
}

void SharedObject::setLocalMirror(const bool &mirror){
    m_localMirror = mirror;
    emit setLocalMirrorSignal(m_localMirror);
}

void SharedObject::setExtraParams(const QString &params)
{
    m_extraParams = params;
    emit newExtraParams(m_extraParams);
}

void SharedObject::sendExtraParams(){
    qDebug() << "js: Sending : " << m_extraParams;
    emit newExtraParams(m_extraParams);
}

void SharedObject::sendPTZCapabilities()
{
    emit newPTZCapabilities(m_camPTZName, m_camCanZoom, m_camHasPresets, m_camHasSettings);
}

bool SharedObject::isPageReady()
{
    return m_pageIsReady;
}

void SharedObject::startRecording()
{
    emit startRecordingRequested();
}

void SharedObject::stopRecording()
{
    emit stopRecordingRequested();
}

void SharedObject::pauseRecording()
{
    emit pauseRecordingRequested();
}

bool SharedObject::getLocalMirror(){
    return m_localMirror;
    //emit setLocalMirrorSignal(m_localMirror);
}

void SharedObject::getExtraParams()
{
    sendExtraParams();
}

QString SharedObject::getAllSettings()
{
    //Create JSON Object for contact
    QJsonObject myObject
    {
        {"contactInfo", serializeContactInfo()},
        {"audio", serializeAudioSource()},
        {"video", serializeVideoSource()},
        {"mirror", m_localMirror},
        {"extraParams", m_extraParams},
        {"secondAudioVideo", serialize2ndSources()},
        {"ptz", serializePtzCapabilities()},
        {"screenControl", true}
    };

    QJsonDocument doc(myObject);

    QString rval = QString(doc.toJson(QJsonDocument::Compact));

    qDebug() << "js: Sending : " << rval;

    return rval;
}

void SharedObject::setPageReady()
{
    m_pageIsReady = true;
    emit pageIsReady();
}

void SharedObject::setVideoError(QString context, QString error)
{
    emit videoErrorOccured(context, error);
}

void SharedObject::setAudioError(QString context, QString error)
{
    emit audioErrorOccured(context, error);
}

void SharedObject::setGeneralError(QString context, QString error)
{
    emit generalErrorOccured(context, error);
}

