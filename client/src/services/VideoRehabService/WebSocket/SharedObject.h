#ifndef SHAREDOBJECT_H
#define SHAREDOBJECT_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QWebChannel>

#include "drivers/PTZ/ICameraDriver.h"
#include "drivers/PTZ/Vivotek8111.h"

//#include "camsettings.h"

class SharedObject : public QObject
{
    Q_OBJECT
public:
    SharedObject(QObject *parent = nullptr);
    ~SharedObject();

    void startPTZCameraDriver(const int& camera_src, const QString &camera_name, const QString &hostname, const int port, const QString &user, const QString &password);
    void stopPTZCameraDriver();
    ICameraDriver* getPTZCameraDriver() const;

    void setContactInformation(const QString &name, const QString &uuid);
    void setCurrentCameraName(const QString &name);
    void setCurrentCameraIndex(const int &index);
    void setCurrentAudioSrcName(const QString &name);
    void setSecondAudioSrcName(const QString &name);
    void setSecondVideoName(const QString &name);
    void setPTZCapabilities(const bool &zoom, const bool &presets, const bool &settings);
    void sendCameraSettings(const QString &uuid, CameraImageSettings &settings, const QString &owner="");
    void getCameraSettings(const QString &uuid);
    void removeVideoSource(const QString &name);

    void setExtraParams(const QString &params);

    void sendCurrentVideoSource();
    void sendCurrentAudioSource();
    void sendSecondSources();
    void sendExtraParams();
    void sendPTZCapabilities();

    void sendContactInformation();

    bool isPageReady();

    void startRecording();
    void stopRecording();


public slots:

    Q_INVOKABLE QString getContactInformation();
    Q_INVOKABLE QString getCurrentVideoSource();
    Q_INVOKABLE QString getCurrentAudioSource();
    Q_INVOKABLE void getSecondSources();
    Q_INVOKABLE bool getLocalMirror();
    Q_INVOKABLE void getExtraParams();

    Q_INVOKABLE QString getAllSettings();

    Q_INVOKABLE void setPageReady();
    Q_INVOKABLE void setVideoError(QString context, QString error);
    Q_INVOKABLE void setAudioError(QString context, QString error);
    Q_INVOKABLE void setGeneralError(QString context, QString error);

    Q_INVOKABLE void zoomInClicked(QString uuid);
    Q_INVOKABLE void zoomOutClicked(QString uuid);
    Q_INVOKABLE void zoomMinClicked(QString uuid);
    Q_INVOKABLE void zoomMaxClicked(QString uuid);

    Q_INVOKABLE void camSettingsClicked(QString uuid);
    Q_INVOKABLE void gotoPresetClicked(QString uuid, int preset);
    Q_INVOKABLE void setPresetClicked(QString uuid, int preset);
    Q_INVOKABLE void imageClicked(QString uuid, int x, int y, int w, int h);

    Q_INVOKABLE void cameraChanged(QString name, int index);
    Q_INVOKABLE void setLocalMirror(const bool &mirror);

    Q_INVOKABLE void dataForwardReceived(QString json);

    Q_INVOKABLE void newRemoteConnection();

signals:

    void newContactInformation(QString);
    void newVideoSource(QString);
    void newAudioSource(QString);
    void newSecondSources(QString);
    void newDataForward(QString);
    void newRemoteStream();
    void newExtraParams(QString);
    void newPTZCapabilities(QString, bool, bool, bool);
    void videoSourceRemoved(QString);

    void zoomIn(QString uuid);
    void zoomOut(QString uuid);
    void zoomMin(QString uuid);
    void zoomMax(QString uuid);

    void gotoPreset(QString uuid, int preset);
    void setPreset(QString uuid, int preset);

    void move(QString uuid, int x, int y, int w, int h);

    void camSettings(QString uuid);                                     // Request to get camSettings for that UUID
    void setCamSettings(QString uuid, CameraImageSettings& settings);   // Request to set camSettings (if local uuid) or to send camSettings (if remote uuid)

    void setLocalMirrorSignal(bool mirror);

    void pageIsReady();
    void currentCameraWasChanged();
    void videoErrorOccured(QString context, QString error);
    void audioErrorOccured(QString context, QString error);
    void generalErrorOccured(QString context, QString error);

    void startRecordingRequested();
    void stopRecordingRequested();

private:
    ICameraDriver* m_ptzCameraDriver;

protected:



    QString serializeContactInfo();
    QString serializeAudioSource();
    QString serializeVideoSource();
    QString serialize2ndSources();
    QString serializePtzCapabilities();

    QString serializeAll();

    QString m_userName;
    QString m_userUUID;
    QString m_cameraName;
    QString m_audioName;
    QString m_2ndAudioName;
    QString m_2ndVideoName;
    int     m_cameraIndex;
    bool    m_localMirror;
    QString m_extraParams;

    bool    m_camCanZoom;
    bool    m_camHasPresets;
    bool    m_camHasSettings;
    QString m_camPTZName;

    bool    m_pageIsReady;

};

#endif // SHAREDOBJECT_H
