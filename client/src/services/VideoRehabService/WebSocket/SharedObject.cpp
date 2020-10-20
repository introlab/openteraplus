#include "SharedObject.h"

SharedObject::SharedObject(QObject *parent) : QObject(parent)
{
    m_cameraIndex = -1;
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
    emit zoomIn(uuid);
}

void SharedObject::zoomOutClicked(QString uuid)
{
    //qDebug() << "Zoom Out! UUID = " << uuid;
    emit zoomOut(uuid);
}

void SharedObject::zoomMinClicked(QString uuid)
{
    //qDebug() << "Zoom Min! UUID = " << uuid;
    emit zoomMin(uuid);
}

void SharedObject::zoomMaxClicked(QString uuid)
{
    //qDebug() << "Zoom Max! UUID = " << uuid;
    emit zoomMax(uuid);
}

void SharedObject::camSettingsClicked(QString uuid)
{
    qDebug() << "SharedObjet -> Cam Settings! UUID = " << uuid;
    emit camSettings(uuid);
}

void SharedObject::gotoPresetClicked(QString uuid, int preset)
{
    //qDebug() << "Goto Preset! UUID = " << uuid << ", position = " << preset;
    emit gotoPreset(uuid, preset);
}

void SharedObject::setPresetClicked(QString uuid, int preset)
{
    emit setPreset(uuid, preset);
}

void SharedObject::imageClicked(QString uuid, int x, int y, int w, int h)
{
    qDebug() << "Click! UUID = " << uuid << ", x = " << x << " y = " << y << " width = " << w << " height = " << h;
    emit move(uuid, x, y, w, h);
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
    qDebug() << "js: Sending Video Source: " << video_src;

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


void SharedObject::sendCurrentAudioSource(){
    QString audio_src = serializeAudioSource();

    qDebug() << "js: Sending : " << audio_src;

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
/*
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
}*/

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
        /*if (msg_type == "setCamSettings"){
            qDebug() << "Shared Object - Request to set cam settings";
            CameraImageSettings settings;

            settings.fromJSON(obj);

            if (obj.value("owner_uuid").toString() == obj.value("uuid").toString())
                emit setCamSettings(obj.value("uuid").toString(), settings); // Local settings
            else
                emit setCamSettings(obj.value("owner_uuid").toString(), settings); // Remote settings
        }*/

    }
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
    emit newPTZCapabilities(m_userUUID, m_camCanZoom, m_camHasPresets, m_camHasSettings);
}

bool SharedObject::isPageReady()
{
    return m_pageIsReady;
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
        {"secondAudioVideo", serialize2ndSources()}
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
