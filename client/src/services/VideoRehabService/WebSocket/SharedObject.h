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

//#include "camsettings.h"

class SharedObject : public QObject
{
    Q_OBJECT
public:
    SharedObject(QObject *parent = nullptr);

    void setContactInformation(const QString &name, const QString &uuid);
    void setCurrentCameraName(const QString &name);
    void setCurrentCameraIndex(const int &index);
    void setCurrentAudioSrcName(const QString &name);
    void setSecondAudioSrcName(const QString &name);
    void setSecondVideoName(const QString &name);
    //void sendCameraSettings(const QString &uuid, CameraImageSettings &settings, const QString &owner="");
    void getCameraSettings(const QString &uuid);

    void setLocalMirror(const bool &mirror);
    void setExtraParams(const QString &params);

    void sendContactInformation();
    void sendCurrentVideoSource();
    void sendCurrentAudioSource();
    void sendSecondSources();
    void sendExtraParams();


public slots:

    Q_INVOKABLE void getContactInformation();
    Q_INVOKABLE void getCurrentVideoSource();
    Q_INVOKABLE void getCurrentAudioSource();
    Q_INVOKABLE void getSecondSources();
    Q_INVOKABLE void getLocalMirror();
    Q_INVOKABLE void getExtraParams();

    Q_INVOKABLE void zoomInClicked(QString uuid);
    Q_INVOKABLE void zoomOutClicked(QString uuid);
    Q_INVOKABLE void zoomMinClicked(QString uuid);
    Q_INVOKABLE void zoomMaxClicked(QString uuid);

    Q_INVOKABLE void camSettingsClicked(QString uuid);
    Q_INVOKABLE void gotoPresetClicked(QString uuid, int preset);
    Q_INVOKABLE void setPresetClicked(QString uuid, int preset);
    Q_INVOKABLE void imageClicked(QString uuid, int x, int y, int w, int h);

    Q_INVOKABLE void cameraChanged(QString name, int index);

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

    void zoomIn(QString uuid);
    void zoomOut(QString uuid);
    void zoomMin(QString uuid);
    void zoomMax(QString uuid);

    void gotoPreset(QString uuid, int preset);
    void setPreset(QString uuid, int preset);

    void move(QString uuid, int x, int y, int w, int h);

    void camSettings(QString uuid);                                     // Request to get camSettings for that UUID
    //void setCamSettings(QString uuid, CameraImageSettings& settings);   // Request to set camSettings (if local uuid) or to send camSettings (if remote uuid)

    void setLocalMirrorSignal(bool mirror);
protected:

    //TODO add more information as needed...
    void sendContactInformation(const QString &name, const QString & uuid);
    void sendCurrentVideoSource(const QString &name, const int &index);

    QString m_userName;
    QString m_userUUID;
    QString m_cameraName;
    QString m_audioName;
    QString m_2ndAudioName;
    QString m_2ndVideoName;
    int     m_cameraIndex;
    bool    m_localMirror;
    QString m_extraParams;

};

#endif // SHAREDOBJECT_H