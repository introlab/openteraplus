#include  <ptz_test.h>
#include  <QApplication>
#include <QVideoWidget>
#include <QMouseEvent>
#include <QAbstractVideoSurface>
#include <QMediaObject>
#include <QCameraViewfinderSettings>
#include <QCameraImageCapture>
#include <QtCore>
#include <QGraphicsSceneMouseEvent>

PTZTestMainWindow::PTZTestMainWindow(QWidget *parent)
    : QMainWindow(parent),m_camera(nullptr), m_scene(nullptr), m_view(nullptr), m_videoItem(nullptr)

{
    m_ui.setupUi(this);

    connect(&m_ptz, &ICameraDriver::cameraConnected, this, &PTZTestMainWindow::cameraConnected);

    //Create player
    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_ui.widget_CameraView);

    m_view->setScene(m_scene);
    m_ui.widget_CameraView->layout()->addWidget(m_view);

    m_videoItem = new MyVideoItem();
    connect(m_videoItem, &MyVideoItem::videoClicked,this,&PTZTestMainWindow::videoClicked);
    m_scene->addItem(m_videoItem);

    fillCameraInfoCombo();
}

void PTZTestMainWindow::on_pushButton_RTSP_Start_clicked()
{
    qDebug() << "void PTZTestMainWindow::on_pushButton_RTSP_Start_clicked()";
    //GET RTSP FEED
    QString rtsp = m_ui.lineEdit_RTSP_URL->text();

    //TODO Hard coded device for now
    m_virtualCamera.init(rtsp/*, "/akvcam/video0"*/);
    m_virtualCamera.start();
}

void PTZTestMainWindow::on_pushButton_RTSP_Stop_clicked()
{
    qDebug() << "void PTZTestMainWindow::on_pushButton_RTSP_Stop_clicked()";
    m_virtualCamera.stop();
}

void PTZTestMainWindow::on_pushButton_Quit_clicked()
{
    QApplication::quit();
}

void PTZTestMainWindow::on_pushButton_PTZ_Set_clicked()
{
    //Get all values
    QString hostname = m_ui.lineEdit_PTZ_Hostname->text();
    int port = m_ui.lineEdit_PTZ_Port->text().toInt();
    QString user = m_ui.lineEdit_PTZ_User->text();
    QString password = m_ui.lineEdit_PTZ_Password->text();

    m_ptz.init(hostname, port, user, password);

}

void PTZTestMainWindow::fillCameraInfoCombo()
{
    auto cameraList = QCameraInfo::availableCameras();

    foreach(auto info, cameraList)
    {
        qDebug() << info.deviceName();
        qDebug() << info.description();
        m_ui.comboBox_Camera->insertItem(-1,info.description(), info.deviceName());
    }
}

int main (int argc, char* argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    PTZTestMainWindow window;
    window.show();
    return app.exec();
}

void PTZTestMainWindow::on_comboBox_Camera_currentIndexChanged(int index)
{
    qDebug() << "PTZTestMainWindow::on_comboBox_Camera_currentIndexChanged " << index;


}

void PTZTestMainWindow::on_comboBox_Camera_activated(int index)
{
    qDebug() << "PTZTestMainWindow::on_comboBox_Camera_activated(int index)" << index;

    //Get camera name
    auto camera_name = m_ui.comboBox_Camera->itemData(index).toByteArray();

    qDebug() << "Camera Name " << camera_name;

    QCameraInfo info(camera_name);
    qDebug() << info;

    if (m_camera)
    {
        m_camera->stop();
        m_camera->deleteLater();
    }

    m_camera = new QCamera(info);
    m_camera->setCaptureMode(QCamera::CaptureVideo);
    m_camera->setViewfinder(m_videoItem);
    m_camera->start();
}

 void PTZTestMainWindow::videoClicked(int x, int y, QSizeF screenSize)
 {
    QSize mySize(screenSize.toSize());
    QPoint point(x,y);
    m_ptz.setPointNClick(point, mySize);
 }

 void PTZTestMainWindow::cameraConnected(CameraInfo info)
 {
    qDebug() << " void PTZTestMainWindow::cameraConnected(CameraInfo)";
    qDebug() << info.toXmlStr();
 }

 MyVideoItem::MyVideoItem(QGraphicsItem *item)
     : QGraphicsVideoItem(item)
 {

 }

 void MyVideoItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
 {

    qDebug() << "MyVideoItem::mousePressEvent(QGraphicsSceneMouseEvent *event)";
    qDebug() << event->pos();
    qDebug() << "scale" << this->scale();
    qDebug() << "size" << this->size();
    qDebug() << "nativeSize" << this->nativeSize();
    emit  videoClicked(event->pos().x(), event->pos().y(), this->size());
    QGraphicsVideoItem::mousePressEvent(event);
 }

 void MyVideoItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
 {
    qDebug() << " void MyVideoItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)";
    QGraphicsVideoItem::mouseReleaseEvent(event);
 }
