#include "CameraUtilities.h"

///////////////////////////////////////////////////////////
/// \brief CameraPointNClick::CameraPointNClick
///
CameraPointNClick::CameraPointNClick()
{
    clearCameraPointNClick();
}

CameraPointNClick::CameraPointNClick(const CameraPointNClick &cpy)
{
    m_coordinates = cpy.m_coordinates;
    m_screenResolution = cpy.m_screenResolution;
}

CameraPointNClick &CameraPointNClick::operator=(const CameraPointNClick &cpy)
{
    m_coordinates = cpy.m_coordinates;
    m_screenResolution = cpy.m_screenResolution;
    return *this;
}

void CameraPointNClick::clearCameraPointNClick()
{
    m_coordinates.setX(0);
    m_coordinates.setY(0);

    m_screenResolution.setHeight(0);
    m_screenResolution.setWidth(0);
}

void CameraPointNClick::setCoordinates(QPoint coord)
{
    m_coordinates = coord;
}

void CameraPointNClick::setScreenResolution(QSize resolution)
{
    m_screenResolution = resolution;
}

QPoint CameraPointNClick::coordinates()
{
    return m_coordinates;
}

QSize CameraPointNClick::screenResolution()
{
    return m_screenResolution;
}

bool CameraPointNClick::fromXml(QString xmlStr)
{
    QDomDocument doc;
    if(doc.setContent(xmlStr))
    {
        QDomElement e = doc.firstChildElement("CameraPointNClick");
        return fromXml(e);
    }

    return false;
}

bool CameraPointNClick::fromXml(QDomElement &e)
{
    if(e.tagName() == "CameraPointNClick")
    {
        if(e.hasAttribute("coord_x"))
            m_coordinates.setX(e.attributeNode("coord_x").value().toInt());
        else
            m_coordinates.setX(0);

        if(e.hasAttribute("coord_y"))
            m_coordinates.setY(e.attributeNode("coord_y").value().toInt());
        else
            m_coordinates.setY(0);

        if(e.hasAttribute("screen_h"))
            m_screenResolution.setHeight(e.attributeNode("screen_h").value().toInt());
        else
            m_screenResolution.setHeight(0);

        if(e.hasAttribute("screen_w"))
            m_screenResolution.setWidth(e.attributeNode("screen_w").value().toInt());
        else
            m_screenResolution.setWidth(0);


        return true;
    }

    return false;
}

void CameraPointNClick::toXml(QDomDocument &doc, QDomElement &e)
{
    //Create element
    //QDomDocument doc;
    QDomElement myElement = doc.createElement("CameraPointNClick");

    //Set attributes
    myElement.setAttribute("coord_x",m_coordinates.x());
    myElement.setAttribute("coord_y",m_coordinates.y());

    myElement.setAttribute("screen_w",m_screenResolution.width());
    myElement.setAttribute("screen_h",m_screenResolution.height());

    //Appending child to root node (parent)
    if(e.isNull())
        e = myElement;
    else
        e.appendChild(myElement);
}

QString CameraPointNClick::toXmlStr()
{
    QDomDocument tmpDoc;
    QDomElement tmpElt;
    this->toXml(tmpDoc,tmpElt);
    tmpDoc.appendChild(tmpElt);

    return tmpDoc.toString();
}


////////////////////////////////////////////////////////
/// \brief CameraRequest::CameraConfig
///
CameraRequest::CameraRequest()
{
    clearCameraRequest();
}
CameraRequest::CameraRequest(const CameraRequest &cpy)
{
    m_cameraFlag = cpy.m_cameraFlag;
    m_cameraAction = cpy.m_cameraAction;
}


CameraRequest &CameraRequest::operator=(const CameraRequest &cpy)
{
    m_cameraFlag = cpy.m_cameraFlag;
    m_cameraAction = cpy.m_cameraAction;
    return *this;
}

void CameraRequest::clearCameraRequest()
{
    m_cameraFlag = CRF_UNKNOWN;
    m_cameraAction = CRA_UNKOWN;
}

void CameraRequest::setCameraFlag(CameraRequest::CameraRequestFlags flag)
{
    m_cameraFlag = flag;
}

void CameraRequest::setCameraAction(CameraRequest::CameraRequestAction flag)
{
    m_cameraAction = flag;
}

CameraRequest::CameraRequestFlags CameraRequest::getCameraFlag()
{
    return m_cameraFlag;
}

CameraRequest::CameraRequestAction CameraRequest::getCameraAction()
{
    return m_cameraAction;
}

bool CameraRequest::fromXml(QString xmlStr)
{
    QDomDocument doc;
    if(doc.setContent(xmlStr))
    {
        QDomElement e = doc.firstChildElement("CameraRequest");
        return fromXml(e);
    }

    return false;
}

bool CameraRequest::fromXml(QDomElement &e)
{

    if(e.tagName() == "CameraRequest")
    {
        if(e.hasAttribute("requestFlag"))
            m_cameraFlag = (CameraRequestFlags)e.attributeNode("requestFlag").value().toInt();
        else
            m_cameraFlag = CRF_UNKNOWN;

        if(e.hasAttribute("requestAction"))
            m_cameraAction = (CameraRequestAction)e.attributeNode("requestAction").value().toInt();
        else
            m_cameraAction = CRA_UNKOWN;

        return true;
    }

    return false;
}

void CameraRequest::toXml(QDomDocument &doc, QDomElement &e)
{
    //Create element
    //QDomDocument doc;
    QDomElement myElement = doc.createElement("CameraRequest");

    //Set attributes
    myElement.setAttribute("requestFlag",m_cameraFlag);
    myElement.setAttribute("requestAction",m_cameraAction);

    //Appending child to root node (parent)
    if(e.isNull())
        e = myElement;
    else
        e.appendChild(myElement);
}

QString CameraRequest::toXmlStr()
{
    QDomDocument tmpDoc;
    QDomElement tmpElt;
    this->toXml(tmpDoc,tmpElt);
    tmpDoc.appendChild(tmpElt);

    return tmpDoc.toString();
}



//////////////////////////////////////////////
/// \brief CameraInfo::CameraInfo
///
CameraInfo::CameraInfo()
{
    clearCameraInfo();
}

CameraInfo::~CameraInfo()
{

}

CameraInfo::CameraInfo(const CameraInfo &cpy)
{
    m_deviceName = cpy.m_deviceName;
    m_deviceConnectionInfo = cpy.m_deviceConnectionInfo;
    m_deviceStatus = cpy.m_deviceStatus;
    m_deviceError = cpy.m_deviceError;
    m_deviceFunct = cpy.m_deviceFunct;
    m_unit = cpy.m_unit;
    m_pan = cpy.m_pan;
    m_tilt = cpy.m_tilt;
    m_zoom = cpy.m_zoom;
    m_resolution = cpy.m_resolution;
    m_pointNClick = cpy.m_pointNClick;
    m_cameraRequest = cpy.m_cameraRequest;
    m_logPeriod = cpy.m_logPeriod;
    m_numberPreset = cpy.m_numberPreset;
    m_presetID = cpy.m_presetID;
    m_imageSettings = cpy.m_imageSettings;
}

CameraInfo &CameraInfo::operator=(const CameraInfo &cpy)
{
    m_deviceName = cpy.m_deviceName;
    m_deviceConnectionInfo = cpy.m_deviceConnectionInfo;
    m_deviceStatus = cpy.m_deviceStatus;
    m_deviceError = cpy.m_deviceError;
    m_deviceFunct = cpy.m_deviceFunct;
    m_unit = cpy.m_unit;
    m_pan = cpy.m_pan;
    m_tilt = cpy.m_tilt;
    m_zoom = cpy.m_zoom;
    m_resolution = cpy.m_resolution;
    m_pointNClick = cpy.m_pointNClick;
    m_cameraRequest = cpy.m_cameraRequest;
    m_logPeriod = cpy.m_logPeriod;
    m_numberPreset = cpy.m_numberPreset;
    m_presetID = cpy.m_presetID;
    m_imageSettings = cpy.m_imageSettings;
    return *this;
}

void CameraInfo::clearCameraInfo()
{
    m_deviceName = "Unkown";
    m_deviceConnectionInfo = "Unkown";
    m_deviceStatus = CIS_DISCONNECTED;
    m_deviceError = CIE_NO_ERROR;
    m_deviceFunct.clear();
    m_unit = CIU_UNKNOWN;
    m_pan = 0.0;
    m_tilt = 0.0;
    m_zoom = 0.0;
    m_resolution = QSize();
    m_pointNClick = CameraPointNClick();
    m_cameraRequest = CameraRequest();
    m_logPeriod = 0;
    m_numberPreset = 0;
    m_presetID = 0;
}

QString CameraInfo::deviceName() const
{
    return m_deviceName;
}

QString CameraInfo::deviceConnectionInfo() const
{
    return m_deviceConnectionInfo;
}

QList<CameraInfo::CameraInfoFunct> CameraInfo::deviceFunct() const
{
    return m_deviceFunct;
}

CameraInfo::CameraInfoStatus CameraInfo::deviceStatus() const
{
    return m_deviceStatus;
}

CameraInfo::CameraInfoError CameraInfo::deviceError() const
{
    return m_deviceError;
}

CameraInfo::CameraInfoUnit CameraInfo::unit() const
{
    return m_unit;
}

double CameraInfo::pan() const
{
    return m_pan;
}

double CameraInfo::tilt() const
{
    return m_tilt;
}

double CameraInfo::zoom() const
{
    return m_zoom;
}

QSize CameraInfo::resolution() const
{
    return m_resolution;
}

CameraImageSettings* CameraInfo::imageSettings()
{
    return &m_imageSettings;
}


CameraPointNClick *CameraInfo::pointNClick()
{
    return &m_pointNClick;
}

CameraRequest *CameraInfo::cameraRequest()
{
    return &m_cameraRequest;
}

int CameraInfo::logPeriod()
{
    return m_logPeriod;
}

int CameraInfo::numberPreset()
{
    return m_numberPreset;
}

int CameraInfo::presetID()
{
    return m_presetID;
}

void CameraInfo::setDeviceName(const QString &deviceName)
{
    m_deviceName = deviceName;
}

void CameraInfo::setDeviceConnection(const QString deviceConnectionInfo)
{
    m_deviceConnectionInfo = deviceConnectionInfo;
}

void CameraInfo::setDeviceFunct(const QList<CameraInfoFunct> functs)
{
    m_deviceFunct = functs;
}

void CameraInfo::setDeviceStatus(CameraInfo::CameraInfoStatus flag)
{
    m_deviceStatus = flag;
}

void CameraInfo::setDeviceError(CameraInfo::CameraInfoError flag)
{
    m_deviceError = flag;
}

void CameraInfo::setUnit(CameraInfoUnit unit)
{
    m_unit = unit;
}

void CameraInfo::setPan(double pan)
{
    m_pan = pan;
}

void CameraInfo::setTilt(double tilt)
{
    m_tilt = tilt;
}

void CameraInfo::setZoom(double zoom)
{
    m_zoom = zoom;
}

void CameraInfo::setResolution(QSize resolution)
{
    m_resolution = resolution;
}

void CameraInfo::setPointNClick(CameraPointNClick pNc)
{
    m_pointNClick = pNc;
}

void CameraInfo::setCameraRequest(CameraRequest request)
{
    m_cameraRequest = request;
}

void CameraInfo::setLogPeriod(int period)
{
    m_logPeriod = period;
}

void CameraInfo::setNumberPreset(int num)
{
    m_numberPreset = num;
}

void CameraInfo::setPresetID(int id)
{
    m_presetID = id;
}
/*
bool CameraInfo::fromXml(QString xmlStr)
{
    QDomDocument doc;
    if(doc.setContent(xmlStr))
    {
        QDomElement e = doc.firstChildElement("CameraInfo");
        return fromXml(e);
    }

    return false;
}

bool CameraInfo::fromXml(QDomElement &e)
{
    if(e.tagName() == "CameraInfo")
    {
        if(e.hasAttribute("deviceName"))
            m_deviceName = e.attributeNode("deviceName").value();
        else
            m_deviceName = "Unknown";

        if(e.hasAttribute("deviceConnectionInfo"))
            m_deviceConnectionInfo = e.attributeNode("deviceConnectionInfo").value();
        else
            m_deviceConnectionInfo = "Unknown";

        if(e.hasAttribute("deviceStatus"))
            m_deviceStatus = (CameraInfoStatus)e.attributeNode("deviceStatus").value().toInt();
        else
            m_deviceStatus = CIS_DISCONNECTED;

        if(e.hasAttribute("deviceError"))
            m_deviceError = (CameraInfoError)e.attributeNode("deviceError").value().toInt();
        else
            m_deviceError = CIE_NO_ERROR;

        if(e.hasAttribute("deviceFunct"))
            m_deviceFunct.append((CameraInfoFunct)e.attributeNode("deviceFunct").value().toInt());
        else
            m_deviceFunct.clear();

        if(e.hasAttribute("unit"))
            m_unit = (CameraInfoUnit)e.attributeNode("unit").value().toInt();
        else
            m_unit = CIU_UNKNOWN;

        if(e.hasAttribute("pan"))
            m_pan = e.attributeNode("pan").value().toDouble();
        else
            m_pan = 0.0;

        if(e.hasAttribute("tilt"))
            m_tilt = e.attributeNode("tilt").value().toDouble();
        else
            m_tilt = 0.0;

        if(e.hasAttribute("zoom"))
            m_zoom = e.attributeNode("zoom").value().toDouble();
        else
            m_zoom = 0.0;

        if(e.hasAttribute("res_w"))
            m_resolution.setWidth(e.attributeNode("res_w").value().toInt());
        else
            m_resolution.setWidth(0);

        if(e.hasAttribute("res_h"))
            m_resolution.setHeight(e.attributeNode("res_h").value().toInt());
        else
            m_resolution.setHeight(0);

        if(e.hasAttribute("logPeriod"))
            m_logPeriod = e.attributeNode("logPeriod").value().toInt();
        else
            m_logPeriod = 0;

        if(e.hasAttribute("numPreset"))
            m_numberPreset = e.attributeNode("numPreset").value().toInt();
        else
            m_numberPreset = 0;

        if(e.hasAttribute("presetID"))
            m_presetID = e.attributeNode("presetID").value().toInt();
        else
            m_presetID = 0;


        //Read child node
        QDomNodeList n = e.childNodes();

        for(int i = 0;i<n.size();i++)
        {
            QDomElement tmpElement = n.at(i).toElement();
            if(tmpElement.tagName() == "CameraRequest")
                m_cameraRequest.fromXml(tmpElement);
            if(tmpElement.tagName() == "CameraPointNClick")
                m_pointNClick.fromXml(tmpElement);
            if(tmpElement.tagName() == "CameraImage")
                m_imageSettings.fromXml(tmpElement);
        }

        return true;
    }

    return false;
}

void CameraInfo::toXml(QDomDocument &doc, QDomElement &e)
{
    //Create element
    QDomElement myElement = doc.createElement("CameraInfo");

    //Set attributes
    myElement.setAttribute("deviceName",m_deviceName);
    myElement.setAttribute("deviceConnectionInfo",m_deviceConnectionInfo);
    myElement.setAttribute("deviceStatus",m_deviceStatus);
    myElement.setAttribute("deviceError",m_deviceError);
    myElement.setAttribute("deviceFunct",m_deviceFunct);
    myElement.setAttribute("unit",m_unit);
    myElement.setAttribute("pan",m_pan);
    myElement.setAttribute("tilt",m_tilt);
    myElement.setAttribute("zoom",m_zoom);
    myElement.setAttribute("res_w",m_resolution.width());
    myElement.setAttribute("res_h",m_resolution.height());
    myElement.setAttribute("logPeriod",m_logPeriod);
    myElement.setAttribute("numPreset",m_numberPreset);
    myElement.setAttribute("presetID",m_presetID);

    m_pointNClick.toXml(doc,myElement);
    m_cameraRequest.toXml(doc,myElement);
    m_imageSettings.toXml(doc,myElement);

    //Appending child to root node (parent)
    if(e.isNull())
        e = myElement;
    else
        e.appendChild(myElement);
}

QString CameraInfo::toXmlStr()
{
    QDomDocument tmpDoc;
    QDomElement tmpElt;
    this->toXml(tmpDoc,tmpElt);
    tmpDoc.appendChild(tmpElt);

    return tmpDoc.toString();
}*/

////////////////////////////////////////////////////////
CameraImageSettings::CameraImageSettings(){
    brightness=0;
    saturation=0;
    sharpness=0;
    white=0;
    contrast=0;

    for (int i=0; i<2; i++){
        brightnessLimits[i]=0;
        contrastLimits[i]=0;
        saturationLimits[i]=0;
        sharpnessLimits[i]=0;
        whiteLimits[i]=0;
    }
}

CameraImageSettings::CameraImageSettings(const CameraImageSettings &cpy){
    *this = cpy;
}

void CameraImageSettings::setBrightnessLimits(int min, int max){
    brightnessLimits[0]=min;
    brightnessLimits[1]=max;
}

void CameraImageSettings::setContrastLimits(int min, int max){
    contrastLimits[0]=min;
    contrastLimits[1]=max;
}

void CameraImageSettings::setSharpnessLimits(int min, int max){
    sharpnessLimits[0]=min;
    sharpnessLimits[1]=max;
}

void CameraImageSettings::setSaturationLimits(int min, int max){
    saturationLimits[0]=min;
    saturationLimits[1]=max;
}

void CameraImageSettings::setWhiteLimits(int min, int max){
    whiteLimits[0]=min;
    whiteLimits[1]=max;
}

void CameraImageSettings::getBrightnessLimits(int &min, int &max){
    min = brightnessLimits[0];
    max = brightnessLimits[1];
}

void CameraImageSettings::getContrastLimits(int &min, int &max){
    min = contrastLimits[0];
    max = contrastLimits[1];
}

void CameraImageSettings::getSharpnessLimits(int &min, int &max){
    min = sharpnessLimits[0];
    max = sharpnessLimits[1];
}

void CameraImageSettings::getSaturationLimits(int &min, int &max){
    min = saturationLimits[0];
    max = saturationLimits[1];
}

void CameraImageSettings::getWhiteLimits(int &min, int &max){
    min = whiteLimits[0];
    max = whiteLimits[1];
}

void CameraImageSettings::setBrightness(int value){
    brightness=value;
}

void CameraImageSettings::setContrast(int value){
    contrast=value;
}

void CameraImageSettings::setSharpness(int value){
    sharpness=value;
}

void CameraImageSettings::setSaturation(int value){
    saturation=value;
}

void CameraImageSettings::setWhite(int value){
    white=value;
}

int CameraImageSettings::getBrightness(){
    return brightness;
}

int CameraImageSettings::getContrast(){
    return contrast;
}

int CameraImageSettings::getSharpness(){
    return sharpness;
}

int CameraImageSettings::getSaturation(){
    return saturation;
}

int CameraImageSettings::getWhite(){
    return white;
}

CameraImageSettings &CameraImageSettings::operator=(const CameraImageSettings &cpy){
    brightness = cpy.brightness;
    saturation = cpy.saturation;
    sharpness = cpy.sharpness;
    contrast = cpy.contrast;
    white = cpy.white;

    for (int i=0; i<2; i++){
        brightnessLimits[i] = cpy.brightnessLimits[i];
        saturationLimits[i] = cpy.saturationLimits[i];
        sharpnessLimits[i] = cpy.sharpnessLimits[i];
        contrastLimits[i] = cpy.contrastLimits[i];
        whiteLimits[i] = cpy.whiteLimits[i];
    }

    return *this;
}

bool CameraImageSettings::fromXml(QString xmlStr){
    QDomDocument doc;
    if(doc.setContent(xmlStr))
    {
        QDomElement e = doc.firstChildElement("CameraImage");
        return fromXml(e);
    }

    return false;
}

bool CameraImageSettings::fromXml(QDomElement &e){
    if(e.tagName() == "CameraImage")
    {

        brightness = e.attribute("b","0").toInt();
        contrast = e.attribute("c","0").toInt();
        sharpness = e.attribute("sh","0").toInt();
        saturation = e.attribute("sa","0").toInt();
        white = e.attribute("w","0").toInt();

        brightnessLimits[0] = e.attribute("b_min","0").toInt();
        brightnessLimits[1] = e.attribute("b_max","0").toInt();
        contrastLimits[0] = e.attribute("c_min","0").toInt();
        contrastLimits[1] = e.attribute("c_max","0").toInt();
        sharpnessLimits[0] = e.attribute("sh_min","0").toInt();
        sharpnessLimits[1] = e.attribute("sh_max","0").toInt();
        saturationLimits[0] = e.attribute("sa_min","0").toInt();
        saturationLimits[1] = e.attribute("sa_max","0").toInt();
        whiteLimits[0] = e.attribute("w_min","0").toInt();
        whiteLimits[1] = e.attribute("w_max","0").toInt();

        return true;
    }

    return false;
}

void CameraImageSettings::toXml(QDomDocument &doc, QDomElement &e){
    //Create element
    QDomElement myElement = doc.createElement("CameraImage");

    //Set attributes
    myElement.setAttribute("b",QString::number(brightness));
    myElement.setAttribute("c",QString::number(contrast));
    myElement.setAttribute("sh",QString::number(sharpness));
    myElement.setAttribute("sa",QString::number(saturation));
    myElement.setAttribute("w",QString::number(white));

    myElement.setAttribute("b_min",QString::number(brightnessLimits[0]));
    myElement.setAttribute("b_max",QString::number(brightnessLimits[1]));
    myElement.setAttribute("c_min",QString::number(contrastLimits[0]));
    myElement.setAttribute("c_max",QString::number(contrastLimits[1]));
    myElement.setAttribute("sh_min",QString::number(sharpnessLimits[0]));
    myElement.setAttribute("sh_max",QString::number(sharpnessLimits[1]));
    myElement.setAttribute("sa_min",QString::number(saturationLimits[0]));
    myElement.setAttribute("sa_max",QString::number(saturationLimits[1]));
    myElement.setAttribute("w_min",QString::number(whiteLimits[0]));
    myElement.setAttribute("w_max",QString::number(whiteLimits[1]));

    //Appending child to root node (parent)
    if(e.isNull())
        e = myElement;
    else
        e.appendChild(myElement);
}

QString CameraImageSettings::toXmlStr(void){
    QDomDocument tmpDoc;
    QDomElement tmpElt;
    this->toXml(tmpDoc,tmpElt);
    tmpDoc.appendChild(tmpElt);

    return tmpDoc.toString();
}

QJsonObject CameraImageSettings::toJSON(){
    //Create JSON Object for contact
    QJsonObject myObject
    {
        {"b", QString::number(brightness)},
        {"c", QString::number(contrast)},
        {"sa", QString::number(saturation)},
        {"sh", QString::number(sharpness)},
        {"w", QString::number(white)},
        {"b_min",QString::number(brightnessLimits[0])},
        {"b_max",QString::number(brightnessLimits[1])},
        {"c_min",QString::number(contrastLimits[0])},
        {"c_max",QString::number(contrastLimits[1])},
        {"sh_min",QString::number(sharpnessLimits[0])},
        {"sh_max",QString::number(sharpnessLimits[1])},
        {"sa_min",QString::number(saturationLimits[0])},
        {"sa_max",QString::number(saturationLimits[1])},
        {"w_min",QString::number(whiteLimits[0])},
        {"w_max",QString::number(whiteLimits[1])}

    };

    return myObject;
}
bool CameraImageSettings::fromJSON(QJsonObject &json){

    brightness = json.value("b").toString().toInt();
    contrast = json.value("c").toString().toInt();
    sharpness = json.value("sh").toString().toInt();
    saturation = json.value("sa").toString().toInt();
    white = json.value("w").toString().toInt();

    brightnessLimits[0] = json.value("b_min").toString().toInt();
    brightnessLimits[1] = json.value("b_max").toString().toInt();
    contrastLimits[0] = json.value("c_min").toString().toInt();
    contrastLimits[1] = json.value("c_max").toString().toInt();
    sharpnessLimits[0] = json.value("sh_min").toString().toInt();
    sharpnessLimits[1] = json.value("sh_max").toString().toInt();
    saturationLimits[0] = json.value("sa_min").toString().toInt();
    saturationLimits[1] = json.value("sa_max").toString().toInt();
    whiteLimits[0] = json.value("w_min").toString().toInt();
    whiteLimits[1] = json.value("w_max").toString().toInt();

    return true;
}
