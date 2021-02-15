#ifndef CAMERAOBJECT_H
#define CAMERAOBJECT_H

#include <QDebug>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QSize>
#include <QPoint>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

class CameraPointNClick
{
public:
    CameraPointNClick();
    CameraPointNClick(const CameraPointNClick &cpy);
    ///Operator=
    CameraPointNClick& operator= (const CameraPointNClick &cpy);

    void clearCameraPointNClick();

    void setCoordinates(QPoint coord);
    void setScreenResolution(QSize resolution);

    QPoint coordinates();
    QSize screenResolution();

    bool fromXml(QString xmlStr);
    bool fromXml(QDomElement &e);
    void toXml(QDomDocument &doc, QDomElement &e);
    QString toXmlStr(void);

private:
    QPoint m_coordinates;
    QSize m_screenResolution;
};

/**
 * @brief The CameraRequest class
 * This class contain the action to accomplish by the camera based on the flag
 * Flags can be ORed
 */
class CameraRequest
{
public:
    enum CameraRequestFlags
    {
        CRF_UNKNOWN = 0x0000,
        CRF_FUNCT = 0x0001,
        CRF_PAN_POS = 0x0002,
        CRF_TILT_POS = 0x0004,
        CRF_ZOOM =0x0008,
        CRF_RESOLUTION = 0x0010,
        CRF_POINT_N_CLICK = 0x0020,
        CRF_LOG_PERIOD = 0x0040,
        CRF_STATUS = 0x0080,
        CRF_PRESET = 0x0100,
        CRF_IMG_SETTINGS = 0x0200
    };

    enum CameraRequestAction
    {
        CRA_UNKOWN,
        CRA_GET,
        CRA_SET
    };

    CameraRequest();
    CameraRequest(const CameraRequest &cpy);
    ///Operator=
    CameraRequest& operator= (const CameraRequest &cpy);

    void clearCameraRequest();

    void setCameraFlag(CameraRequestFlags flag);
    void setCameraAction(CameraRequestAction flag);

    CameraRequestFlags getCameraFlag();
    CameraRequestAction getCameraAction();

    bool fromXml(QString xmlStr);
    bool fromXml(QDomElement &e);
    void toXml(QDomDocument &doc, QDomElement &e);
    QString toXmlStr(void);

private:
    CameraRequestFlags m_cameraFlag;
    CameraRequestAction m_cameraAction;

    //TODO : Put a timeStamp ?
};


class CameraImageSettings
{
public:
    CameraImageSettings();
    CameraImageSettings(const CameraImageSettings &cpy);
    CameraImageSettings& operator=(const CameraImageSettings &cpy);

    void setBrightnessLimits(int min, int max);
    void setContrastLimits(int min, int max);
    void setSharpnessLimits(int min, int max);
    void setSaturationLimits(int min, int max);
    void setWhiteLimits(int min, int max);

    void getBrightnessLimits(int &min, int &max);
    void getContrastLimits(int &min, int &max);
    void getSharpnessLimits(int &min, int &max);
    void getSaturationLimits(int &min, int &max);
    void getWhiteLimits(int &min, int &max);

    void setBrightness(int value);
    void setContrast(int value);
    void setSharpness(int value);
    void setSaturation(int value);
    void setWhite(int value);

    int getBrightness();
    int getContrast();
    int getSharpness();
    int getSaturation();
    int getWhite();

    ///XML Functionnalities
    bool fromXml(QString xmlStr);
    bool fromXml(QDomElement &e);
    void toXml(QDomDocument &doc, QDomElement &e);
    QString toXmlStr(void);

    //JSON functionalities
    QJsonObject toJSON();
    bool fromJSON(QJsonObject &json);
private:
    int brightness;
    int contrast;
    int saturation;
    int sharpness;
    int white;

    int brightnessLimits[2];
    int contrastLimits[2];
    int saturationLimits[2];
    int sharpnessLimits[2];
    int whiteLimits[2];

};

/**
 * @brief The CameraInfo class
 * This Abstract class is common for all data sent between camera device threads
 * and is the only object sent through Vigil framework.
 */
class CameraInfo
{
public:

    enum CameraInfoFunct
    {
        CIF_NONE = 0x00,
        CIF_PAN_TILT_ABS= 0x01,
        CIF_PAN_TILT_REL= 0x02,
        CIF_POINT_N_CLICK= 0x04,
        CIF_ZOOM_ABS= 0x08,
        CIF_ZOOM_REL= 0x10,
        CIF_PERIODIC_LOGGING = 0x20,
        CIF_PRESET_POS  =   0x40,
        CIF_IMAGE_SETTINGS = 0x80,
        CIF_ALL  = 0xFF
    };

    enum CameraInfoUnit
    {
        CIU_UNKNOWN = 0x00,
        CIU_RAD = 0x01,
        CIU_DEG = 0x02,
        CIU_PIXEL = 0x04
    };

    enum CameraInfoStatus
    {
        CIS_CONNECTING,
        CIS_CONNECTED,
        CIS_DISCONNECTED
    };

    enum CameraInfoError
    {
        CIE_NO_ERROR,
        CIE_NO_CONNECTION,
        CIE_INVALID_LOGIN,
        CIE_PROTOCOL_ERROR
    };


    ///
    /// \brief CameraInfo
    ///
    CameraInfo();
    ///
    /// \brief ~CameraInfo
    ///
    virtual ~CameraInfo();
    ///
    /// \brief CameraInfo
    /// \param cpy
    ///
    CameraInfo(const CameraInfo &cpy);
    ///Operator=
    CameraInfo& operator= (const CameraInfo &cpy);

    void clearCameraInfo();

    ///
    /// \brief deviceName
    /// \return
    ///
    QString deviceName() const;
    QString deviceConnectionInfo() const;
    QList<CameraInfoFunct> deviceFunct() const;
    CameraInfoStatus deviceStatus() const;
    CameraInfoError deviceError() const;
    CameraInfoUnit unit() const;
    CameraImageSettings *imageSettings();

    double pan() const;
    double tilt() const;
    double zoom() const;
    QSize resolution() const;
    CameraPointNClick *pointNClick();
    CameraRequest *cameraRequest();
    int logPeriod();
    int numberPreset();
    int presetID();
    ///
    /// \brief setDeviceName
    /// \param deviceName
    ///
    void setDeviceName(const QString &deviceName);
    void setDeviceConnection(const QString deviceConnectionInfo);
    void setDeviceFunct(const QList<CameraInfoFunct> functs);
    void setDeviceStatus(CameraInfoStatus flag);
    void setDeviceError(CameraInfoError flag);
    void setUnit(CameraInfoUnit unit);
    void setPan(double pan);
    void setTilt(double tilt);
    void setZoom(double zoom);
    void setResolution(QSize resolution);
    void setPointNClick(CameraPointNClick pNc);
    void setCameraRequest(CameraRequest request);
    void setLogPeriod(int period);
    void setNumberPreset(int num);
    void setPresetID(int id);

    ///XML Functionnalities
    /*bool fromXml(QString xmlStr);
    bool fromXml(QDomElement &e);
    void toXml(QDomDocument &doc, QDomElement &e);
    QString toXmlStr(void);*/


protected:
    ///device name
    QString                 m_deviceName;
    QString                 m_deviceConnectionInfo;
    QList<CameraInfoFunct>  m_deviceFunct;
    CameraInfoStatus        m_deviceStatus;
    CameraInfoError         m_deviceError;
    CameraImageSettings     m_imageSettings;

    CameraInfoUnit m_unit;
    double m_pan;
    double m_tilt;
    double m_zoom;
    QSize m_resolution;
    CameraPointNClick m_pointNClick;

    CameraRequest m_cameraRequest;

    int m_logPeriod; //in ms

    int m_numberPreset;
    int m_presetID;
};


#endif
