#include  <ptz_test.h>
#include  <QApplication>
#include "VirtualCamera.h"
#include "Vivotek8111.h"

int main (int argc, char* argv[])
{
    QApplication app(argc, argv);

    //DL - Testing Virtual camera driver, should be put elsewhere
    //VirtualCamera cam;
    //cam.init("rtsp://10.0.1.10:554/live3.sdp", "/akvcam/video0");
    //cam.start();

    VIVOTEK8111 vivotek;
    vivotek.init("10.0.1.10", 80, "", "");

    return app.exec();
}
