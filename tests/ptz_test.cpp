#include  <ptz_test.h>
#include  <QApplication>
#include "VirtualCamera.h"

int main (int argc, char* argv[])
{
    QApplication app(argc, argv);

    //DL - Testing Virtual camera driver, should be put elsewhere
    VirtualCamera cam;
    cam.init("rtsp://10.0.1.10:554/live3.sdp", "/akvcam/video0");
    cam.start();

    return app.exec();
}
