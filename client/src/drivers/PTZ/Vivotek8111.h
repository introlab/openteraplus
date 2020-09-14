/*
 * Vivotek8111.h
 *
 *  Created on: 2014-05-28
 *  Author: Pierre Lepage
 */

#ifndef VIVOTEK8111_H_
#define VIVOTEK8111_H_

#include "CameraUtilities.h"
#include "ICameraDriver.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAbstractNetworkCache>
#include <QAuthenticator>
#include <QTimer>

#define CAMERA_NAME "VIVOTEK8111"

class VIVOTEK8111 :public ICameraDriver
{
    Q_OBJECT

public:

    static const int VIVOTEK_NUM_PRESET;

    /**
        Constructor
        \param config The RobotConfiguration (contains the camera configuration)
    */
    VIVOTEK8111();

    ///Destructor
    ~VIVOTEK8111();

    // ICameraDriver interface
    void init(QString config);
    void getCameraInfo(CameraRequest infoRequested);
    void setCameraInfo(CameraInfo info);

public slots:
    ///QTimer slot
    void connectionTimeout(void);

    ///QNetworkAccessManager Slot
    void cameraDataRdy(QNetworkReply* netReply);
    void cameraAuthenticationRequest( QNetworkReply * reply, QAuthenticator * authenticator );


    //ICameraDriver interface
    void onQuitDriver();
private:
    void closeNetworkAccessManager();
    void setPointNClick(QPoint coordinates, QSize screenSize);
    void setRelZoom(double value);
    void setPresetID(int id);
    void configPresetID(int id);
    void requestInformation();
    void requestImageSettings();
    void setImageSettings(CameraImageSettings settings);
    void setZoomSpeed(const int &speed);
    int convertZoomToSpeed(const double &value);

private:

    QVariant m_lastStatusCode;
    bool m_firstTime;
    QTimer m_connectionTimer;
    QNetworkAccessManager *m_CameraManager;
    bool m_forwardResolution;

    //DL Added (JSON configuration)
    QString m_hostname;
    quint16 m_port;
    QString m_user;
    QString m_password;

};


#endif

