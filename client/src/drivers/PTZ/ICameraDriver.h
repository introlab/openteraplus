#ifndef ICAMERA_DRIVER_H
#define ICAMERA_DRIVER_H

#include <QObject>
#include "CameraUtilities.h"

class ICameraDriver: public QObject
{
    Q_OBJECT
public:
    ICameraDriver(){}
    virtual ~ICameraDriver(){}
    virtual void init(QString instanceStr) = 0;
    virtual void getCameraInfo(CameraRequest infoRequested) = 0;
    virtual void setCameraInfo(CameraInfo info) = 0;

public slots:
    /**
     * @brief onQuitDriver
     * This slot is called when we need to close the connection with the
     * medical device
     */
    virtual void onQuitDriver() = 0;

signals:
    /**
     * @brief dataReady
     * signal sent to controller when a data is read from the device
     */
    void cameraDataReady(CameraInfo);
    ///
    /// \brief cameraConnected
    ///
    void cameraConnected(CameraInfo);
    ///
    /// \brief cameraDisconnected
    ///
    void cameraDisconnected(CameraInfo);
    ///
    /// \brief cameraError
    /// \param errorID
    /// \param errorStr
    ///
    void cameraError(CameraInfo);

protected:
    CameraInfo m_cameraInfo;

};

#endif
