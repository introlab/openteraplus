#ifndef ICAMERA_DRIVER_H
#define ICAMERA_DRIVER_H

#include <QObject>
#include "CameraUtilities.h"

class ICameraDriver: public QObject
{
    Q_OBJECT
public:
    ICameraDriver();
    virtual ~ICameraDriver();
    virtual void init(QString instanceStr) = 0;
    virtual void init(const QString &hostname, const int port, const QString &user, const QString &password) = 0;
    virtual void getCameraInfo(CameraRequest infoRequested) = 0;
    virtual void setCameraInfo(CameraInfo info) = 0;

    QList<CameraInfo::CameraInfoFunct> getCameraFunctions();
    bool hasCameraFunction(CameraInfo::CameraInfoFunct funct);

public slots:
    /**
     * @brief onQuitDriver
     * This slot is called when we need to close the connection with the
     * medical device
     */
    virtual void onQuitDriver() = 0;

    virtual void setPointNClick(QPoint coordinates, QSize screenSize) = 0;
    virtual void setRelZoom(double value) = 0;
    virtual void zoomIn() = 0;
    virtual void zoomOut() = 0;
    virtual void zoomMax() = 0;
    virtual void zoomMin() = 0;
    virtual void gotoPresetID(int id) = 0;
    virtual void setPresetID(int id) = 0;
    virtual void requestInformation() = 0;
    virtual void requestImageSettings() = 0;

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
