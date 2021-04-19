/*
 * Vivotek8111.h
 *
 *  Created on: 2014-05-28
 *  Author: Pierre Lepage
 *  Modified by : Simon Briere, Dominic Letourneau
 */

#ifndef Vivotek8111_H_
#define Vivotek8111_H_

#include "CameraUtilities.h"
#include "ICameraDriver.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAbstractNetworkCache>
#include <QAuthenticator>
#include <QTimer>

#define CAMERA_NAME "Vivotek8111"

class Vivotek8111 :public ICameraDriver
{
    Q_OBJECT

public:

    static const int VIVOTEK_NUM_PRESET;

    /**
        Constructor
        \param config The RobotConfiguration (contains the camera configuration)
    */
    Vivotek8111();

    ///Destructor
    ~Vivotek8111();

    // ICameraDriver interface
    void init(QString config);
    void init(const QString &hostname, const int port, const QString &user, const QString &password);
    void getCameraInfo(CameraRequest infoRequested);
    void setCameraInfo(CameraInfo info);

public slots:
    ///QTimer slot
    void connectionTimeout(void);
    void requestTimeout(void);

    ///QNetworkAccessManager Slot
    void cameraDataRdy(QNetworkReply* netReply);
    void cameraAuthenticationRequest( QNetworkReply * reply, QAuthenticator * authenticator );


    //ICameraDriver interface
    void onQuitDriver();

    void closeNetworkAccessManager();
    void setPointNClick(QPoint coordinates, QSize screenSize);
    void setRelZoom(double value);
    void zoomIn();
    void zoomOut();
    void zoomMax();
    void zoomMin();
    void setPresetID(int id);
    void gotoPresetID(int id);
    void requestInformation();
    void requestImageSettings();
    void setImageSettings(CameraImageSettings settings);
    void setZoomSpeed(const int &speed);
    int convertZoomToSpeed(const double &value);

private:

    QVariant m_lastStatusCode;
    bool m_firstTime;
    QTimer m_connectionTimer;
    QTimer m_requestTimer;
    QNetworkAccessManager *m_CameraManager;
    bool m_forwardResolution;

    //DL Added (JSON configuration)
    QString m_hostname;
    quint16 m_port;
    QString m_user;
    QString m_password;

    void doPost(const QNetworkRequest& request, const QUrlQuery& args);

};


#endif

