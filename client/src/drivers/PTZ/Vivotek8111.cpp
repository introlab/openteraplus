#include "Vivotek8111.h"
#include <QStringList>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <math.h>

const int Vivotek8111::VIVOTEK_NUM_PRESET = 3;

Vivotek8111::Vivotek8111() : ICameraDriver(),
    m_lastStatusCode(0),
    m_firstTime(true),
    m_CameraManager(0),
    m_forwardResolution(false),
    m_hostname(""),
    m_port(0),
    m_user(""),
    m_password("")
{
    qDebug() << "Vivotek8111::Vivotek8111()";

    // Set Image Settings limits
    m_cameraInfo.imageSettings()->setBrightnessLimits(-5,5);
    m_cameraInfo.imageSettings()->setSaturationLimits(-5,5);
    m_cameraInfo.imageSettings()->setContrastLimits(-5,5);
    m_cameraInfo.imageSettings()->setSharpnessLimits(-3,3);
    m_cameraInfo.imageSettings()->setWhiteLimits(0,8);

    // Request timeout timer
    connect(&m_requestTimer, &QTimer::timeout, this, &Vivotek8111::requestTimeout);
    m_requestTimer.setInterval(5000);
    m_requestTimer.setSingleShot(true);
}

Vivotek8111::~Vivotek8111()
{
    closeNetworkAccessManager();
}

void Vivotek8111::init(QString config)
{
    //Create json object
    QJsonValue value(config);
    QJsonObject jsonObj = value.toObject();

    //Verify all fields
    if(jsonObj.contains("hostname") && jsonObj.contains("password") && jsonObj.contains("user") && jsonObj.contains("password"))
    {
        //Get hostname and port
        QString hostname = jsonObj.find("hostname").value().toString();
        int port = jsonObj.find("port").value().toInt();
        //Get user and password
        QString user = jsonObj.find("user").value().toString();
        QString password = jsonObj.find("password").value().toString();

        //Recall init with all parameters
        init(hostname, port, user, password);
    }

}

void Vivotek8111::init(const QString &hostname, const int port, const QString &user, const QString &password)
{
    m_hostname = hostname;
    m_port = port;
    m_user = user;
    m_password = password;

    m_cameraInfo.setDeviceName(CAMERA_NAME);
    m_cameraInfo.setDeviceConnection(QString("%1:%2").arg(m_hostname).arg(m_port));
    m_cameraInfo.setDeviceFunct(QList<CameraInfo::CameraInfoFunct>()
                                << CameraInfo::CIF_POINT_N_CLICK
                                << CameraInfo::CIF_ZOOM_REL
                                << CameraInfo::CIF_PRESET_POS
                                << CameraInfo::CIF_IMAGE_SETTINGS);
    m_cameraInfo.setNumberPreset(VIVOTEK_NUM_PRESET);

    m_CameraManager = new QNetworkAccessManager(this);

    connect(m_CameraManager, SIGNAL(finished(QNetworkReply*)),this, SLOT(cameraDataRdy(QNetworkReply*)));
    connect(m_CameraManager, SIGNAL(authenticationRequired ( QNetworkReply *, QAuthenticator *)),this, SLOT(cameraAuthenticationRequest( QNetworkReply *, QAuthenticator *)));

    //Start the connection by asking resolution
    connect(&m_connectionTimer,SIGNAL(timeout()),this,SLOT(connectionTimeout()));
    m_connectionTimer.setSingleShot(true);
    m_connectionTimer.setInterval(500);
    m_connectionTimer.start();

}

void Vivotek8111::getCameraInfo(CameraRequest infoRequested)
{
    bool emitSignal = true;

    //You can already fill some fields
    m_cameraInfo.setCameraRequest(infoRequested);

    switch(infoRequested.getCameraFlag())
    {
    case CameraRequest::CRF_STATUS:
    case CameraRequest::CRF_FUNCT:
    case CameraRequest::CRF_POINT_N_CLICK:
    case CameraRequest::CRF_ZOOM:
        break;
    case CameraRequest::CRF_RESOLUTION:
        //Send resolution only if you received it at least once
        if(m_firstTime)
        {
            m_forwardResolution = true;
            emitSignal = false;
        }
        break;
    case CameraRequest::CRF_IMG_SETTINGS:
        // Get images settings
        requestImageSettings();
        break;
    default:
        emitSignal = false;
        break;
    }

    if(emitSignal)
        emit cameraDataReady(m_cameraInfo);
}

void Vivotek8111::setCameraInfo(CameraInfo info)
{

    switch(info.cameraRequest()->getCameraFlag())
    {
    case CameraRequest::CRF_POINT_N_CLICK:
        //Remember last pointNClick
        m_cameraInfo.setPointNClick((*info.pointNClick()));
        //Apply the cmd
        setPointNClick(info.pointNClick()->coordinates(),info.pointNClick()->screenResolution());
        break;
    case CameraRequest::CRF_ZOOM:
        //remember last zoom
        m_cameraInfo.setZoom(info.zoom());
        //Apply the cmd
        setRelZoom(info.zoom());
        break;
    case CameraRequest::CRF_PRESET:
        if (info.cameraRequest()->getCameraAction()==CameraRequest::CRA_GET){
            m_cameraInfo.setPresetID(info.presetID());  //mostly for log....
            gotoPresetID(info.presetID());
        }
        if (info.cameraRequest()->getCameraAction()==CameraRequest::CRA_SET){ // Set preset
            setPresetID(info.presetID());
        }
        break;
    case CameraRequest::CRF_IMG_SETTINGS:
        setImageSettings(*info.imageSettings());
        break;
    default:
        break;
    }

}

void Vivotek8111::setPointNClick(QPoint coordinates, QSize screenSize)
{
    //Nothing for the moment
    QString path =  QString("http://%1:%2%3").arg(m_hostname).arg(m_port).arg("/cgi-bin/camctrl/camctrl.cgi");

    QNetworkRequest request;
    request.setUrl(QUrl(path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    //NOTICE : QUrl does not support anymore the addQueryItem in Qt 5.0.0+
    QUrlQuery  params;

    params.addQueryItem("video","0");
    params.addQueryItem("x",QString("%1").arg(coordinates.x()));
    params.addQueryItem("y",QString("%1").arg(coordinates.y()));
    params.addQueryItem("videosize",QString("%1x%2").arg(screenSize.width()).arg(screenSize.height()));
    params.addQueryItem("resolution",QString("%1x%2").arg(m_cameraInfo.resolution().width()).arg(m_cameraInfo.resolution().height()));
    params.addQueryItem("stretch","1");

    doPost(request, params);
}

void Vivotek8111::setRelZoom(double value)
{

    //setZoomSpeed(convertZoomToSpeed(value));

    QString path =  QString("http://%1:%2%3").arg(m_hostname).arg(m_port).arg("/cgi-bin/camctrl/camctrl.cgi");

    QNetworkRequest request;
    request.setUrl(QUrl(path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    QUrlQuery  params;
    bool proceed = true;
    if(value > 0)
        params.addQueryItem("zoom","tele");
    else if(value < 0)
        params.addQueryItem("zoom","wide");
    else
        proceed = false;    //with 0 dont move


    if(m_CameraManager && proceed)
    {
        doPost(request, params);

        //Special patch for max and min
        if(value >= 100 || value <= -100)
        {
            for(int i=0;i<6;i++)
                doPost(request, params);
        }
    }
}

void Vivotek8111::zoomIn()
{
    setRelZoom(0.05);
}

void Vivotek8111::zoomOut()
{
    setRelZoom(-0.05);
}

void Vivotek8111::zoomMax()
{
    setRelZoom(100);
}

void Vivotek8111::zoomMin()
{
    setRelZoom(-100);
}

void Vivotek8111::gotoPresetID(int id)
{
    QString path =  QString("http://%1:%2%3").arg(m_hostname).arg(m_port).arg("/cgi-bin/camctrl/recall.cgi");

    QNetworkRequest request;
    request.setUrl(QUrl(path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    QUrlQuery  params;
    params.addQueryItem("recall",QString("pos_%1").arg(id));

    doPost(request, params);
}

void Vivotek8111::setPresetID(int id)
{
    QString path =  QString("http://%1:%2%3").arg(m_hostname).arg(m_port).arg("/cgi-bin/operator/preset.cgi");

    QNetworkRequest request;
    request.setUrl(QUrl(path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    // Delete old position
    QUrlQuery  params;
    params.addQueryItem("delpos",QString("pos_%1").arg(id));

    doPost(request, params);

    // Create "new" position
    params.clear();
    params.addQueryItem("addpos",QString("pos_%1").arg(id));
    doPost(request, params);


    //NOTIFY_TEXT_MESSAGE(this,"Position #" + QString::number(id+1) + " définie.");

}

void Vivotek8111::requestInformation()
{
    //qDebug() << QString("requestInformation"),this->metaObject()->className();
    QString path = QString("http://%1:%2%3").arg(m_hostname).arg(m_port).arg("/cgi-bin/admin/getparam.cgi");

    QNetworkRequest request;
    request.setUrl(QUrl(path));
    //request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    QUrlQuery  params;

    params.addQueryItem("videoin_c0_s0_resolution","");

    //Post command
    doPost(request, params);
}

void Vivotek8111::requestImageSettings(){
    //qDebug() << QString("requestImageSettings"),this->metaObject()->className();
    QString path = QString("http://%1:%2%3").arg(m_hostname).arg(m_port).arg("/cgi-bin/admin/getparam.cgi");

    QNetworkRequest request;
    request.setUrl(QUrl(path));
    //request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    QUrlQuery  params;
    params.addQueryItem("image_c0","");
    params.addQueryItem("videoin_c0_autotrackingwhitebalance","");

    //Post command
    doPost(request, params);
}

void Vivotek8111::setImageSettings(CameraImageSettings settings){
    //qDebug() << QString("requestImageSettings"),this->metaObject()->className();
    QString path = QString("http://%1:%2%3").arg(m_hostname).arg(m_port).arg("/cgi-bin/admin/setparam.cgi");

    QNetworkRequest request;
    request.setUrl(QUrl(path));
    //request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    QUrlQuery  params;
    params.addQueryItem("image_c0_brightness",QString::number(settings.getBrightness()));
    params.addQueryItem("image_c0_saturation",QString::number(settings.getSaturation()));
    params.addQueryItem("image_c0_contrast",QString::number(settings.getContrast()));
    params.addQueryItem("image_c0_sharpness",QString::number(settings.getSharpness()));
    params.addQueryItem("videoin_c0_autotrackingwhitebalance",QString::number(settings.getWhite()));
    params.addQueryItem("videoin_c0_whitebalancemode","0");

    //Post command
    doPost(request, params);
}

void Vivotek8111::setZoomSpeed(const int &speed)
{
    //QString path =  QString("http://%1:%2%3").arg(m_hostname).arg(m_port).arg("/cgi-bin/admin/setparam.cgi");
    QString path =  QString("http://%1:%2%3").arg(m_hostname).arg(m_port).arg("/cgi-bin/admin/camctrl.cgi");

    QNetworkRequest request;
    request.setUrl(QUrl(path));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");

    QUrlQuery  params;
    //params.addQueryItem("camctrl_c0_zoomspeed",QString("%1").arg(speed));
    params.addQueryItem("speedzoom",QString("%1").arg(speed));

    doPost(request, params);
}

int Vivotek8111::convertZoomToSpeed(const double &value)
{
    double absValue = qAbs(value);

    int speedfactor = 0;

    absValue = qMin(absValue,5.0);

    if(absValue < 1.0)
        speedfactor =  floor((1.0 - absValue) * -5.0);
    else
        speedfactor =(int)absValue - 1;


    return speedfactor;
}

void Vivotek8111::doPost(const QNetworkRequest &request, const QUrlQuery &args)
{
    if (m_CameraManager){
         m_CameraManager->post(request, args.query(QUrl::FullyEncoded).toUtf8());
         if (!m_requestTimer.isActive())
             m_requestTimer.start();
    }
}

/// PUBLIC SLOTS
void Vivotek8111::cameraDataRdy(QNetworkReply *netReply)
{
    m_requestTimer.stop();
    m_lastStatusCode = netReply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    //qDebug() << QString("cameraDataRdy - Http Status : %1").arg(m_lastStatusCode.toInt()),this->metaObject()->className();

    // "200 OK" received?
    if (m_lastStatusCode.toInt()>=200 && m_lastStatusCode.toInt() < 300 )
    {
        QByteArray data = netReply->readAll();

        QStringList myList = QString(data).split("&");

        for (int i = 0; i < myList.size(); i++)
        {
            if (myList[i].contains("videoin_c0_s0_resolution"))
            {
                QSize resolution;

                if(myList[i].contains("QCIF"))
                {
                    resolution.setWidth(176);
                    resolution.setHeight(120);
                }
                else if(myList[i].contains("CIF"))
                {
                    resolution.setWidth(352);
                    resolution.setHeight(240);
                }
                else if(myList[i].contains("4CIF"))
                {
                    resolution.setWidth(704);
                    resolution.setHeight(480);
                }
                else if(myList[i].contains("D1"))
                {
                    resolution.setWidth(720);
                    resolution.setHeight(480);
                }

                m_cameraInfo.setDeviceError(CameraInfo::CIE_NO_ERROR);
                m_cameraInfo.setResolution(resolution);
                m_cameraInfo.setUnit(CameraInfo::CIU_PIXEL);

                //qDebug() << QString("cameraDataRdy - Rcv resolution wxh : %1 x %2").arg(resolution.width()).arg(resolution.height()),this->metaObject()->className();

                //Detect if this is the first time you go here
                if(m_firstTime)
                {
                    //Your not answering to any particular request, just informing that the camera is On
                    m_cameraInfo.cameraRequest()->clearCameraRequest();
                    m_cameraInfo.setDeviceStatus(CameraInfo::CIS_CONNECTED);
                    m_connectionTimer.stop();
                    m_firstTime = false;
                    emit cameraConnected(m_cameraInfo);
                }

                if(m_forwardResolution)
                {
                    m_forwardResolution = false;

                    //You need to reconstruct the original request of the remote object
                    CameraRequest remoteRequest;
                    remoteRequest.setCameraAction(CameraRequest::CRA_GET);
                    remoteRequest.setCameraFlag(CameraRequest::CRF_RESOLUTION);

                    //You can already fill some fields
                    m_cameraInfo.setCameraRequest(remoteRequest);

                    emit cameraDataReady(m_cameraInfo);
                }

                break;
            }

            if (myList[i].contains("image_c0_brightness") ||
                myList[i].contains("image_c0_saturation") ||
                myList[i].contains("image_c0_contrast") ||
                myList[i].contains("image_c0_sharpness") ||
                myList[i].contains("videoin_c0_whitebalancecontrol")){
                // Parse value

                QStringList items = myList[i].split("\n");
                for (int j=0; j<items.count(); j++){
                    QStringList values = items.at(j).split("'");
                    if (items.at(j).contains("image_c0_brightness")){
                        if (values.count()==3){
                            m_cameraInfo.imageSettings()->setBrightness(values.at(1).toInt());
                        }
                    }
                    if (items.at(j).contains("image_c0_saturation")){
                        if (values.count()==3){
                            m_cameraInfo.imageSettings()->setSaturation(values.at(1).toInt());
                        }
                    }
                    if (items.at(j).contains("image_c0_contrast")){
                        if (values.count()==3){
                            m_cameraInfo.imageSettings()->setContrast(values.at(1).toInt());
                        }
                    }
                    if (items.at(j).contains("image_c0_sharpness")){
                        if (values.count()==3){
                            m_cameraInfo.imageSettings()->setSharpness(values.at(1).toInt());
                        }
                    }
                    if (items.at(j).contains("videoin_c0_autotrackingwhitebalance")){
                        if (values.count()==3){
                            m_cameraInfo.imageSettings()->setWhite(values.at(1).toInt());
                        }
                    }
                }

            }

        }
    }
    else if(m_lastStatusCode.toInt() >= 400)
    {
        //Client or Server error
        m_cameraInfo.cameraRequest()->clearCameraRequest();
        if (m_connectionTimer.isActive()){
            // Error occured at connection - abort.
            m_cameraInfo.setDeviceError(CameraInfo::CIE_NO_CONNECTION);
            m_connectionTimer.stop();
        }else{
            m_cameraInfo.setDeviceError(CameraInfo::CIE_PROTOCOL_ERROR);
        }
        emit cameraError(m_cameraInfo);

    }

    netReply->deleteLater();
}

void Vivotek8111::cameraAuthenticationRequest(QNetworkReply *reply, QAuthenticator *authenticator)
{
    authenticator->setUser(m_user);
    authenticator->setPassword(m_password);

    //Your responsible of this
    reply->deleteLater();
}

void Vivotek8111::connectionTimeout()
{
    //For the vivotek you need to acquire the resolution first
    requestInformation();

    //And images settings
    requestImageSettings();
}

void Vivotek8111::requestTimeout()
{
    qDebug() << "Vivotek8111::requestTimeout()";
    m_cameraInfo.setDeviceError(CameraInfo::CIE_NO_CONNECTION);
    emit cameraError(m_cameraInfo);
}

void Vivotek8111::onQuitDriver()
{
    closeNetworkAccessManager();
}

void Vivotek8111::closeNetworkAccessManager()
{
    if(m_CameraManager != 0)
    {
        m_firstTime = true;
        m_CameraManager->deleteLater();
        m_CameraManager = 0;

        m_cameraInfo.cameraRequest()->clearCameraRequest();
        m_cameraInfo.setDeviceStatus(CameraInfo::CIS_DISCONNECTED);
        emit cameraDisconnected(m_cameraInfo);
    }
}






