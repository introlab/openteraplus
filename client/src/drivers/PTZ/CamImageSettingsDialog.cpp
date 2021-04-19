#include "CamImageSettingsDialog.h"
#include "ui_CamImageSettingsDialog.h"

CamImageSettingsDialog::CamImageSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CamImageSettingsDialog)
{
    ui->setupUi(this);

    setWindowTitle(tr("Paramètres caméra"));
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    connect(ui->sldBrightness, &QSlider::valueChanged, this, &CamImageSettingsDialog::brightnessChanged);
    connect(ui->sldContrast, &QSlider::valueChanged, this, &CamImageSettingsDialog::contrastChanged);
    connect(ui->sldSaturation, &QSlider::valueChanged, this, &CamImageSettingsDialog::saturationChanged);
    connect(ui->sldSharpness, &QSlider::valueChanged, this, &CamImageSettingsDialog::sharpnessChanged);
    connect(ui->sldWhite, &QSlider::valueChanged, this, &CamImageSettingsDialog::whiteChanged);

    connect(ui->btnOK, &QPushButton::clicked, this, &CamImageSettingsDialog::OKClicked);
    connect(ui->btnApply, &QPushButton::clicked, this, &CamImageSettingsDialog::ApplyClicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &CamImageSettingsDialog::CancelClicked);

}

CamImageSettingsDialog::~CamImageSettingsDialog()
{
    delete ui;
}

void CamImageSettingsDialog::setFromCameraInfo(CameraImageSettings settings){
    int min, max;

    settings.getBrightnessLimits(min,max);
    setBrightnessLimits(min,max);
    settings.getSaturationLimits(min,max);
    setSaturationLimits(min,max);
    settings.getSharpnessLimits(min,max);
    setSharpnessLimits(min,max);
    settings.getContrastLimits(min,max);
    setContrastLimits(min,max);
    settings.getWhiteLimits(min,max);
    setWhiteLimits(min,max);

    setBrightness(settings.getBrightness());
    setContrast(settings.getContrast());
    setSharpness(settings.getSharpness());
    setSaturation(settings.getSaturation());
    setWhite(settings.getWhite());

    m_orignal_settings = settings;

}

void CamImageSettingsDialog::setToCameraInfo(CameraImageSettings &settings){

    settings = m_orignal_settings;

    settings.setBrightness(ui->sldBrightness->value());
    settings.setContrast(ui->sldContrast->value());
    settings.setSharpness(ui->sldSharpness->value());
    settings.setSaturation(ui->sldSaturation->value());
    settings.setWhite(ui->sldWhite->value());


}

CameraImageSettings CamImageSettingsDialog::getCurrentImageSettings()
{
    CameraImageSettings current_settings = m_orignal_settings;

    current_settings.setBrightness(ui->sldBrightness->value());
    current_settings.setContrast(ui->sldContrast->value());
    current_settings.setSharpness(ui->sldSharpness->value());
    current_settings.setSaturation(ui->sldSaturation->value());
    current_settings.setWhite(ui->sldWhite->value());

    return current_settings;
}


void CamImageSettingsDialog::setBrightnessLimits(int min, int max){
    ui->sldBrightness->setMinimum(min);
    ui->sldBrightness->setMaximum(max);
    ui->sldBrightness->setPageStep(1);
}

void CamImageSettingsDialog::setContrastLimits(int min, int max){
    ui->sldContrast->setMinimum(min);
    ui->sldContrast->setMaximum(max);
    ui->sldContrast->setPageStep(1);
}

void CamImageSettingsDialog::setSharpnessLimits(int min, int max){
    ui->sldSharpness->setMinimum(min);
    ui->sldSharpness->setMaximum(max);
    ui->sldSharpness->setPageStep(1);
}

void CamImageSettingsDialog::setSaturationLimits(int min, int max){
    ui->sldSaturation->setMinimum(min);
    ui->sldSaturation->setMaximum(max);
    ui->sldSaturation->setPageStep(1);
}

void CamImageSettingsDialog::setWhiteLimits(int min, int max){
    ui->sldWhite->setMinimum(min);
    ui->sldWhite->setMaximum(max);
    ui->sldWhite->setPageStep(1);
}

void CamImageSettingsDialog::setBrightness(int value){
    ui->sldBrightness->setValue(value);
}

void CamImageSettingsDialog::setContrast(int value){
    ui->sldContrast->setValue(value);
}

void CamImageSettingsDialog::setSharpness(int value){
    ui->sldSharpness->setValue(value);
}

void CamImageSettingsDialog::setSaturation(int value){
    ui->sldSaturation->setValue(value);
}

void CamImageSettingsDialog::setWhite(int value){
    ui->sldWhite->setValue(value);
}

int CamImageSettingsDialog::getBrightness(){
    return ui->sldBrightness->value();
}

int CamImageSettingsDialog::getContrast(){
    return ui->sldContrast->value();
}

int CamImageSettingsDialog::getSharpness(){
    return ui->sldSharpness->value();
}

int CamImageSettingsDialog::getSaturation(){
    return ui->sldSaturation->value();
}

int CamImageSettingsDialog::getWhite(){
    return ui->sldWhite->value();
}

void CamImageSettingsDialog::brightnessChanged(){
    ui->lblBrightnessValue->setText(QString::number(ui->sldBrightness->value()));
    if (isVisible())
        ui->btnApply->setEnabled(true);
}

void CamImageSettingsDialog::contrastChanged(){
    ui->lblContrastValue->setText(QString::number(ui->sldContrast->value()));
    if (isVisible())
        ui->btnApply->setEnabled(true);
}

void CamImageSettingsDialog::saturationChanged(){
    ui->lblSaturationValue->setText(QString::number(ui->sldSaturation->value()));
    if (isVisible())
        ui->btnApply->setEnabled(true);
}

void CamImageSettingsDialog::sharpnessChanged(){
    ui->lblSharpnessValue->setText(QString::number(ui->sldSharpness->value()));
    if (isVisible())
        ui->btnApply->setEnabled(true);
}

void CamImageSettingsDialog::whiteChanged(){
    ui->lblWhiteValue->setText(QString::number(ui->sldWhite->value()));
    if (isVisible())
        ui->btnApply->setEnabled(true);
}

void CamImageSettingsDialog::OKClicked(){
    if (ui->btnApply->isEnabled())
        emit settingsChanged(); // Save any change we might have pending
    accept();
}

void CamImageSettingsDialog::CancelClicked(){
    setFromCameraInfo(m_orignal_settings);
    emit settingsChanged();
    reject();
}

void CamImageSettingsDialog::ApplyClicked(){
    emit settingsChanged();
    ui->btnApply->setEnabled(false);
}

void CamImageSettingsDialog::setTargetUUID(const QString &uuid){
    m_target = uuid;
}

QString CamImageSettingsDialog::getTargetUUID(){
    return m_target;
}
