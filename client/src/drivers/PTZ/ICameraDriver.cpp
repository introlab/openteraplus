#include "ICameraDriver.h"

ICameraDriver::ICameraDriver(){

    m_imgSettingsDialog = nullptr;
}

ICameraDriver::~ICameraDriver(){

    if (m_imgSettingsDialog)
        m_imgSettingsDialog->deleteLater();
}


QList<CameraInfo::CameraInfoFunct> ICameraDriver::getCameraFunctions()
{
    return m_cameraInfo.deviceFunct();
}

bool ICameraDriver::hasCameraFunction(CameraInfo::CameraInfoFunct funct)
{
    return m_cameraInfo.deviceFunct().contains(funct);
}

void ICameraDriver::showImageSettingsDialog()
{
    if (m_imgSettingsDialog)
        m_imgSettingsDialog->deleteLater();

    m_imgSettingsDialog = new CamImageSettingsDialog();
    m_imgSettingsDialog->setFromCameraInfo(*m_cameraInfo.imageSettings());

    connect(m_imgSettingsDialog, &CamImageSettingsDialog::settingsChanged, this, &ICameraDriver::camImageSettingsChanged);
    m_imgSettingsDialog->setModal(true);
    m_imgSettingsDialog->show();
}

void ICameraDriver::camImageSettingsChanged()
{
    *m_cameraInfo.imageSettings() = m_imgSettingsDialog->getCurrentImageSettings();
    setImageSettings(*m_cameraInfo.imageSettings());

}
