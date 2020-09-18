#include "ui_ptz_test.h"
#include <QMainWindow>
#include "VirtualCamera.h"
#include "Vivotek8111.h"
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QCameraViewfinderSettings>
#include "CameraUtilities.h"

class MyVideoWidget : public QVideoWidget {
    Q_OBJECT
public:
    MyVideoWidget(QCamera *camera, QWidget *parent=nullptr);

signals:
    void videoClicked(int x, int y);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    QCamera *m_camera;
};

class PTZTestMainWindow : public QMainWindow
{
    Q_OBJECT
    public:
        PTZTestMainWindow(QWidget *parent = nullptr);

    protected:
        Ui::MainWindow m_ui;
        Vivotek8111 m_ptz;
        VirtualCamera m_virtualCamera;
        MyVideoWidget *m_cameraViewfinder;
        QCamera *m_camera;

private slots:
        void on_pushButton_RTSP_Start_clicked();
        void on_pushButton_RTSP_Stop_clicked();
        void on_pushButton_Quit_clicked();
        void on_pushButton_PTZ_Set_clicked();
        void videoClicked(int x, int y);
        void cameraConnected(CameraInfo info);


        void fillCameraInfoCombo();
        void on_comboBox_Camera_currentIndexChanged(int index);
        void on_comboBox_Camera_activated(int index);
};
