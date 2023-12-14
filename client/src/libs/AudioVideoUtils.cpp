#include "AudioVideoUtils.h"
#include <QCameraDevice>
#include <QAudioDevice>
#include <QMediaDevices>

#include <QDebug>

AudioVideoUtils::AudioVideoUtils(QObject *parent) : QObject(parent) {

}

QStringList AudioVideoUtils::getAudioDeviceNames()
{
    QStringList names;


    auto audio_devices = QMediaDevices::audioInputs();
    //QList<QAudioDeviceInfo> audio_devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    foreach(const QAudioDevice &input, audio_devices){
//TODO FIX LINUX
#if 0
//#ifdef Q_OS_LINUX
        // On linux, since Qt use ALSA API, we must filter the returned device list...
        if (input.deviceName().startsWith("alsa_input") || input.deviceName() == "default"){
            QString filtered_name = input.deviceName();
            if (input.deviceName() != "default"){
                QStringList name_parts = filtered_name.split(".");
                // Removes first part - usually is "alsa_input"
                if (name_parts.count()>1)
                    filtered_name = name_parts[1];

                // Split using "_" to remove first and last part, and replace others with spaces
                name_parts = filtered_name.split("_");
                if (name_parts.count()>2){
                    filtered_name = "";
                    for (int i=1; i<name_parts.count()-1; i++){
                        if (i>1)
                            filtered_name += " ";
                        filtered_name += name_parts[i];
                    }
                }else{
                    // No audio name...
                    name_parts = filtered_name.split("-");
                    if (name_parts.count()>2){
                        filtered_name = name_parts[1].replace("_", ":");
                    }
                }

            }
            names.append(filtered_name);
        }
#else \
    //TODO Not sure!
        names.append(input.description());
#endif
    }
    return names;
}

QStringList AudioVideoUtils::getVideoDeviceNames()
{
    QStringList names;

    auto inputs = QMediaDevices::videoInputs();
    foreach (const QCameraDevice &info, inputs)
    {
        QString name = info.description();
        if (!name.contains(" IR ")) // Remove infrared cameras.
            names.append(info.description());
    }
#ifdef Q_OS_WINDOWS
    // Also query virtual cameras, since they are not detected anymore by QMediaDevices
    names.append(AudioVideoUtils::getVirtualCameras());

    //names.append("OpenTeraCam");
#endif
    return names;
}

#ifdef Q_OS_WINDOWS
QStringList AudioVideoUtils::getVirtualCameras()
{
    //qDebug() << "AudioVideoUtils::getVirtualCameras()";

    IEnumMoniker *pEnum = nullptr;
    HRESULT hr = AudioVideoUtils::enumerateCameras(&pEnum);

    if (FAILED(hr))
    {
        //qDebug("enumerate Fails");
        return QStringList();
    }

    IMoniker *pMoniker = nullptr;
    QString devicePath;

    for (int i = 0; pEnum->Next(1, &pMoniker, nullptr) == S_OK; i++) {
        IBaseFilter *filter = nullptr;
        hr = pMoniker->BindToObject(nullptr,
                                    nullptr,
                                    IID_IBaseFilter,
                                    reinterpret_cast<void **>(&filter));

        if (FAILED(hr)) {
            pMoniker->Release();
            pMoniker = nullptr;

            continue;
        }

        CLSID clsid;

        if (FAILED(filter->GetClassID(&clsid))) {
            filter->Release();
            pMoniker->Release();
            pMoniker = nullptr;

            continue;
        }

        filter->Release();

        if (clsid != CLSID_VirtualCameraSource) {
            pMoniker->Release();
            pMoniker = nullptr;

            continue;
        }

        IPropertyBag *pPropBag = nullptr;
        hr = pMoniker->BindToStorage(nullptr,
                                     nullptr,
                                     IID_IPropertyBag,
                                     reinterpret_cast<void **>(&pPropBag));

        if (SUCCEEDED(hr)) {
            VARIANT var;
            VariantInit(&var);
            hr = pPropBag->Read(L"FriendlyName", &var, 0);

            if (SUCCEEDED(hr))
                devicePath = QString::fromWCharArray(var.bstrVal);
            else
                devicePath = QString("OpenTeraCam").arg(i);

            pPropBag->Release();
        }

        pMoniker->Release();
        pMoniker = nullptr;

        break;
    }

    pEnum->Release();

    QStringList webcams;

    if (!devicePath.isEmpty())
        webcams << devicePath;


    //qDebug() << "AudioVideoUtils: " << webcams;
    return webcams;
}

HRESULT AudioVideoUtils::enumerateCameras(IEnumMoniker **ppEnum)
{
    // Create the System Device Enumerator.
    ICreateDevEnum *pDevEnum = nullptr;
    HRESULT hr = CoInitialize(nullptr);

    if (SUCCEEDED(hr)) {

        hr = CoCreateInstance(CLSID_SystemDeviceEnum,
                              nullptr,
                              CLSCTX_INPROC_SERVER,
                              IID_ICreateDevEnum,
                              reinterpret_cast<void **>(&pDevEnum));

        if (SUCCEEDED(hr)) {
            // Create an enumerator for the category.
            hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, ppEnum, 0);

            if (hr == S_FALSE)
                hr = VFW_E_NOT_FOUND;

            pDevEnum->Release();
        }
    }

    return hr;
}
#endif
