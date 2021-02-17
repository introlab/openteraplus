#ifndef CAMSETTINGS_H
#define CAMSETTINGS_H

#include <QDialog>
#include "CameraUtilities.h"

namespace Ui {
class CamImageSettingsDialog;
}


class CamImageSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CamImageSettingsDialog(QWidget *parent = 0);
    ~CamImageSettingsDialog();

    void setFromCameraInfo(CameraImageSettings settings);
    void setToCameraInfo(CameraImageSettings &settings);
    CameraImageSettings getCurrentImageSettings();

    void setBrightnessLimits(int min, int max);
    void setContrastLimits(int min, int max);
    void setSharpnessLimits(int min, int max);
    void setSaturationLimits(int min, int max);
    void setWhiteLimits(int min, int max);

    void setBrightness(int value);
    void setContrast(int value);
    void setSharpness(int value);
    void setSaturation(int value);
    void setWhite(int value);

    int getBrightness();
    int getContrast();
    int getSharpness();
    int getSaturation();
    int getWhite();

    void setTargetUUID(const QString &uuid);
    QString getTargetUUID();


private:
    CameraImageSettings m_orignal_settings;

    QString m_target;

    Ui::CamImageSettingsDialog *ui;

private slots:
    void brightnessChanged();
    void contrastChanged();
    void sharpnessChanged();
    void saturationChanged();
    void whiteChanged();

    void OKClicked();
    void CancelClicked();
    void ApplyClicked();

signals:
    void settingsChanged();
};

#endif // CAMSETTINGS_H
