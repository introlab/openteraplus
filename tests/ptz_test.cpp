#include  <ptz_test.h>
#include  <QApplication>
#include <QVideoWidget>
#include <QMouseEvent>
#include <QAbstractVideoSurface>
#include <QMediaObject>
#include <QCameraViewfinderSettings>
#include <QCameraImageCapture>
#include <QtCore>


PTZTestMainWindow::PTZTestMainWindow(QWidget *parent)
    : QMainWindow(parent), m_cameraViewfinder(nullptr), m_camera(nullptr)
{
    m_ui.setupUi(this);

    //Setup camera view
    //m_cameraViewfinder = new QCameraViewfinder(m_ui.widget_CameraView);
    //m_ui.widget_CameraView->layout()->addWidget(m_cameraViewfinder);


    connect(&m_ptz, &ICameraDriver::cameraConnected, this, &PTZTestMainWindow::cameraConnected);

    fillCameraInfoCombo();
}







void PTZTestMainWindow::on_pushButton_RTSP_Start_clicked()
{
    qDebug() << "void PTZTestMainWindow::on_pushButton_RTSP_Start_clicked()";
    //GET RTSP FEED
    QString rtsp = m_ui.lineEdit_RTSP_URL->text();

    //TODO Hard coded device for now
    m_virtualCamera.init(rtsp, "/akvcam/video0");
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
        if (m_cameraViewfinder)
            m_cameraViewfinder->deleteLater();
    }

    m_camera = new QCamera(info);
    m_camera->setCaptureMode(QCamera::CaptureVideo);

    m_camera->load();
    QCameraImageCapture capture(m_camera);

    auto resolutions = capture.supportedResolutions();
    qDebug() << "resolutions" << resolutions;


    //m_camera->load();
    m_cameraViewfinder = new MyVideoWidget(m_camera, m_ui.widget_CameraView);


    connect(m_cameraViewfinder, &MyVideoWidget::videoClicked, this, &PTZTestMainWindow::videoClicked);

    //auto testView = new QVideoWidget(m_ui.widget_CameraView);
    //QCameraViewfinderSettings settings;
    auto formats = m_camera->supportedViewfinderSettings();

    foreach(auto format, formats) {
        qDebug() << format.resolution();
    }

    auto status = m_camera->status();
    m_ui.widget_CameraView->layout()->addWidget(m_cameraViewfinder);
    m_camera->setViewfinder(m_cameraViewfinder);
    m_cameraViewfinder->show();
    qDebug() << m_camera->errorString();
    m_camera->start();
}

MyVideoWidget::MyVideoWidget(QCamera *camera, QWidget *parent)
    :   QVideoWidget(parent), m_camera(camera)
{

}

void MyVideoWidget::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "MyVideoWidget::mousePressEvent(QMouseEvent *event)";
    auto x = event->x();
    auto y = event->y();

    qDebug() << x << "," << y;
    auto media = this->mediaObject();
    if (m_camera)
    {
        auto settings  = m_camera->viewfinderSettings();
        auto resolution = settings.resolution();
        qDebug() << resolution;
        emit videoClicked(x, y);
    }

    QVideoWidget::mousePressEvent(event);
}

 void PTZTestMainWindow::videoClicked(int x, int y)
 {
    qDebug() << QString("void PTZTestMainWindow::videoClicked(int x=%1, int y=%2)").arg(x).arg(y);
    QSize screenSize(640,480);
    QPoint point(x,y);
    m_ptz.setPointNClick(point, screenSize);
 }

 void PTZTestMainWindow::cameraConnected(CameraInfo info)
 {
    qDebug() << " void PTZTestMainWindow::cameraConnected(CameraInfo)";
    qDebug() << info.toXmlStr();
 }
