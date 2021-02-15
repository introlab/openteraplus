#include "ICameraDriver.h"

ICameraDriver::ICameraDriver(){

}

ICameraDriver::~ICameraDriver(){

}


QList<CameraInfo::CameraInfoFunct> ICameraDriver::getCameraFunctions()
{
    return m_cameraInfo.deviceFunct();
}

bool ICameraDriver::hasCameraFunction(CameraInfo::CameraInfoFunct funct)
{
    return m_cameraInfo.deviceFunct().contains(funct);
}
