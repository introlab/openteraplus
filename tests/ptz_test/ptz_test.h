#include "ui_ptz_test.h"
#include <QMainWindow>
#include "VirtualCamera.h"
#include "Vivotek8111.h"
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QCameraImageCapture>
#include <QCameraViewfinderSettings>
#include "CameraUtilities.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>


class MyVideoItem : public QGraphicsVideoItem {
    Q_OBJECT
public:
    MyVideoItem(QGraphicsItem *parent=nullptr);

signals:
    void videoClicked(int x, int y, QSizeF screenSize);

protected:

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

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
        QCamera *m_camera;
        QGraphicsScene *m_scene;
        QGraphicsView *m_view;
        MyVideoItem *m_videoItem;



private slots:
        void on_pushButton_RTSP_Start_clicked();
        void on_pushButton_RTSP_Stop_clicked();
        void on_pushButton_Quit_clicked();
        void on_pushButton_PTZ_Set_clicked();
        void videoClicked(int x, int y, QSizeF screenSize);
        void cameraConnected(CameraInfo info);


        void fillCameraInfoCombo();
        void on_comboBox_Camera_currentIndexChanged(int index);
        void on_comboBox_Camera_activated(int index);
};
