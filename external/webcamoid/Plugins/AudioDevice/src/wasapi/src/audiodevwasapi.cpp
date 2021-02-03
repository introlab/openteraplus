/* Webcamoid, webcam capture application.
 * Copyright (C) 2011-2017  Gonzalo Exequiel Pedone
 *
 * Webcamoid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Webcamoid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Webcamoid. If not, see <http://www.gnu.org/licenses/>.
 *
 * Web-Site: http://webcamoid.github.io/
 */

#include <QMap>

#include "audiodevwasapi.h"

DEFINE_PROPERTYKEY(PKEY_Device_FriendlyName, 0xa45c254e, 0xdf1c, 0x4efd, 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0, 14);

#define MAX_ERRORS_READ_WRITE 5
#define EVENT_TIMEOUT 1000

typedef QMap<HRESULT, QString> ErrorCodesMap;

inline ErrorCodesMap initErrorCodesMap()
{
    ErrorCodesMap errorCodes = {
        // COM library errors.
        {REGDB_E_CLASSNOTREG  , "REGDB_E_CLASSNOTREG"  },
        {CLASS_E_NOAGGREGATION, "CLASS_E_NOAGGREGATION"},
        {E_NOINTERFACE        , "E_NOINTERFACE"        },
        {E_POINTER            , "E_POINTER"            },

        // IMMDeviceEnumerator errors.
        {E_INVALIDARG , "E_INVALIDARG" },
        {E_NOTFOUND   , "E_NOTFOUND"   },
        {E_OUTOFMEMORY, "E_OUTOFMEMORY"},

        // IAudioClient errors.
        {AUDCLNT_E_ALREADY_INITIALIZED         , "AUDCLNT_E_ALREADY_INITIALIZED"         },
        {AUDCLNT_E_WRONG_ENDPOINT_TYPE         , "AUDCLNT_E_WRONG_ENDPOINT_TYPE"         },
        {AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED     , "AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED"     },
        {AUDCLNT_E_BUFFER_SIZE_ERROR           , "AUDCLNT_E_BUFFER_SIZE_ERROR"           },
        {AUDCLNT_E_CPUUSAGE_EXCEEDED           , "AUDCLNT_E_CPUUSAGE_EXCEEDED"           },
        {AUDCLNT_E_DEVICE_INVALIDATED          , "AUDCLNT_E_DEVICE_INVALIDATED"          },
        {AUDCLNT_E_DEVICE_IN_USE               , "AUDCLNT_E_DEVICE_IN_USE"               },
        {AUDCLNT_E_ENDPOINT_CREATE_FAILED      , "AUDCLNT_E_ENDPOINT_CREATE_FAILED"      },
        {AUDCLNT_E_INVALID_DEVICE_PERIOD       , "AUDCLNT_E_INVALID_DEVICE_PERIOD"       },
        {AUDCLNT_E_UNSUPPORTED_FORMAT          , "AUDCLNT_E_UNSUPPORTED_FORMAT"          },
        {AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED  , "AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED"  },
        {AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL, "AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL"},
        {AUDCLNT_E_SERVICE_NOT_RUNNING         , "AUDCLNT_E_SERVICE_NOT_RUNNING"         },
        {AUDCLNT_E_NOT_INITIALIZED             , "AUDCLNT_E_NOT_INITIALIZED"             },
        {AUDCLNT_E_NOT_STOPPED                 , "AUDCLNT_E_NOT_STOPPED"                 },
        {AUDCLNT_E_EVENTHANDLE_NOT_SET         , "AUDCLNT_E_EVENTHANDLE_NOT_SET"         },
        {AUDCLNT_E_INVALID_SIZE                , "AUDCLNT_E_INVALID_SIZE"                },
        {AUDCLNT_E_OUT_OF_ORDER                , "AUDCLNT_E_OUT_OF_ORDER"                },
        {AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED    , "AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED"    },
        {AUDCLNT_E_BUFFER_ERROR                , "AUDCLNT_E_BUFFER_ERROR"                },
        {AUDCLNT_E_BUFFER_TOO_LARGE            , "AUDCLNT_E_BUFFER_TOO_LARGE"            },
        {AUDCLNT_E_BUFFER_OPERATION_PENDING    , "AUDCLNT_E_BUFFER_OPERATION_PENDING"    }
    };

    return errorCodes;
}

Q_GLOBAL_STATIC_WITH_ARGS(ErrorCodesMap, errorCodes, (initErrorCodesMap()))

AudioDevWasapi::AudioDevWasapi(QObject *parent):
    AudioDev(parent)
{
    this->m_deviceEnumerator = nullptr;
    this->m_pDevice = nullptr;
    this->m_pAudioClient = nullptr;
    this->m_pCaptureClient = nullptr;
    this->m_pRenderClient = nullptr;
    this->m_hEvent = nullptr;
    this->m_cRef = 1;

    // Create DeviceEnumerator
    HRESULT hr;

    // Get device enumerator.
    if (FAILED(hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                                     nullptr,
                                     CLSCTX_ALL,
                                     __uuidof(IMMDeviceEnumerator),
                                     reinterpret_cast<void **>(&this->m_deviceEnumerator)))) {
        return;
    }

    if (FAILED(hr = this->m_deviceEnumerator->RegisterEndpointNotificationCallback(this))) {
        this->m_deviceEnumerator->Release();
        this->m_deviceEnumerator = nullptr;

        return;
    }

    this->updateDevices();
}

AudioDevWasapi::~AudioDevWasapi()
{
    this->uninit();
    this->m_deviceEnumerator->UnregisterEndpointNotificationCallback(this);
    this->m_deviceEnumerator->Release();
}

QString AudioDevWasapi::error() const
{
    return this->m_error;
}

QString AudioDevWasapi::defaultInput()
{
    return this->m_defaultSource;
}

QString AudioDevWasapi::defaultOutput()
{
    return this->m_defaultSink;
}

QStringList AudioDevWasapi::inputs()
{
    return this->m_sources;
}

QStringList AudioDevWasapi::outputs()
{
    return this->m_sinks;
}

QString AudioDevWasapi::description(const QString &device)
{
    return this->m_descriptionMap.value(device);
}

// Get native format for the default audio device.
AkAudioCaps AudioDevWasapi::preferredFormat(const QString &device)
{
    return this->m_sinks.contains(device)?
                AkAudioCaps(AkAudioCaps::SampleFormat_s16,
                            2,
                            44100):
                AkAudioCaps(AkAudioCaps::SampleFormat_u8,
                            1,
                            8000);
}

QList<AkAudioCaps::SampleFormat> AudioDevWasapi::supportedFormats(const QString &device)
{
    return this->m_supportedFormats.value(device);
}

QList<int> AudioDevWasapi::supportedChannels(const QString &device)
{
    return this->m_supportedChannels.value(device);
}

QList<int> AudioDevWasapi::supportedSampleRates(const QString &device)
{
    return this->m_supportedSampleRates.value(device);
}

bool AudioDevWasapi::init(const QString &device, const AkAudioCaps &caps)
{
    return this->init(device, caps, false);
}

bool AudioDevWasapi::init(const QString &device,
                          const AkAudioCaps &caps,
                          bool justActivate)
{
    if (!this->m_deviceEnumerator) {
        this->m_error = "Device enumerator not created.";
        emit this->errorChanged(this->m_error);

        return false;
    }

    // Clear audio buffer.
    this->m_audioBuffer.clear();

    HRESULT hr;

    // Get audio device.
    if (FAILED(hr = this->m_deviceEnumerator->GetDevice(device.toStdWString().c_str(),
                                                   &this->m_pDevice))) {
        this->m_error = "GetDevice: " + errorCodes->value(hr);
        emit this->errorChanged(this->m_error);
        this->uninit();

        return false;
    }

    // Get an instance for the audio client.
    if (FAILED(hr = this->m_pDevice->Activate(__uuidof(IAudioClient),
                                              CLSCTX_ALL,
                                              nullptr,
                                              reinterpret_cast<void **>(&this->m_pAudioClient)))) {
        this->m_error = "Activate: " + errorCodes->value(hr);
        emit this->errorChanged(this->m_error);
        this->uninit();

        return false;
    }

    // Just get the audio client instance.
    if (justActivate)
        return true;

    // Get the minimum size of the buffer in 100-nanosecond units,
    // this means you must do:
    //
    // bufferSize(seconds) = 100e-9 * hnsRequestedDuration
    //
    // to get the size of the buffer in seconds.
    //
    REFERENCE_TIME hnsRequestedDuration;
    this->m_pAudioClient->GetDevicePeriod(nullptr, &hnsRequestedDuration);

    // Accumulate a minimum of 1 sec. of audio in the buffer.
    REFERENCE_TIME minDuration = 10e6;

    if (hnsRequestedDuration < minDuration)
        hnsRequestedDuration = minDuration;

    // Set audio device format.
    WAVEFORMATEX wfx;
    this->waveFormatFromAk(&wfx, caps);
    this->m_curCaps = caps;

    if (FAILED(hr = this->m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                                     AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
                                                     hnsRequestedDuration,
                                                     hnsRequestedDuration,
                                                     &wfx,
                                                     nullptr))) {
        this->m_error = "Initialize: " + errorCodes->value(hr);
        emit this->errorChanged(this->m_error);
        this->uninit();

        return false;
    }

    // Create an event handler for checking when an aundio frame is required
    // for reading or writing.
    this->m_hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    if (!this->m_hEvent) {
        this->m_error = "CreateEvent: Error creating event handler";
        emit this->errorChanged(this->m_error);
        this->uninit();

        return false;
    }

    // Set event handler.
    if (FAILED(this->m_pAudioClient->SetEventHandle(this->m_hEvent))) {
        this->m_error = "SetEventHandle: " + errorCodes->value(hr);
        emit this->errorChanged(this->m_error);
        this->uninit();

        return false;
    }

    // Get audio capture/render client.
    if (this->inputs().contains(device))
        hr = this->m_pAudioClient->GetService(__uuidof(IAudioCaptureClient),
                                              reinterpret_cast<void **>(&this->m_pCaptureClient));
    else
        hr = this->m_pAudioClient->GetService(__uuidof(IAudioRenderClient),
                                              reinterpret_cast<void **>(&this->m_pRenderClient));

    if (FAILED(hr)) {
        this->m_error = "GetService: " + errorCodes->value(hr);
        emit this->errorChanged(this->m_error);
        this->uninit();

        return false;
    }

    // Start audio client.
    if (FAILED(hr = this->m_pAudioClient->Start())) {
        this->m_error = "Start: " + errorCodes->value(hr);
        emit this->errorChanged(this->m_error);
        this->uninit();

        return false;
    }

    this->m_curDevice = device;

    return true;
}

QByteArray AudioDevWasapi::read(int samples)
{
    int bufferSize = samples
                     * this->m_curCaps.bps()
                     * this->m_curCaps.channels()
                     / 8;

    int nErrors = 0;

    // Read audio samples until audio buffer is full.
    while (this->m_audioBuffer.size() < bufferSize
           && nErrors < MAX_ERRORS_READ_WRITE) {
        // Wait until an audio frame can be read.
        if (WaitForSingleObject(this->m_hEvent, EVENT_TIMEOUT) != WAIT_OBJECT_0) {
            nErrors++;

            continue;
        }

        HRESULT hr;
        UINT32 samplesCount = 0;

        // Get the size in samples of the captured audio frame.
        if (FAILED(hr = this->m_pCaptureClient->GetNextPacketSize(&samplesCount))) {
            this->m_error = "GetNextPacketSize: " + errorCodes->value(hr);
            emit this->errorChanged(this->m_error);

            return QByteArray();
        }

        // Check if empty.
        if (samplesCount < 1)
            continue;

        BYTE *pData = nullptr;
        DWORD flags = 0;

        // Read audio buffer.
        if (FAILED(hr = this->m_pCaptureClient->GetBuffer(&pData,
                                                          &samplesCount,
                                                          &flags,
                                                          nullptr,
                                                          nullptr))) {
            this->m_error = "GetBuffer: " + errorCodes->value(hr);
            emit this->errorChanged(this->m_error);

            return QByteArray();
        }

        size_t bufferSize = samplesCount
                            * size_t(this->m_curCaps.bps()
                                     * this->m_curCaps.channels())
                            / 8;

        // This flag means we must ignore the incoming buffer and write zeros
        // to it.
        if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
            pData = new BYTE[bufferSize];
            memset(pData, 0, bufferSize);
        }

        // Copy audio frame to the audio buffer.
        QByteArray buffer(reinterpret_cast<const char *>(pData), int(bufferSize));

        if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            delete [] pData;

        this->m_audioBuffer.append(buffer);

        // Remove read samples from the audio device.
        if (FAILED(hr = this->m_pCaptureClient->ReleaseBuffer(samplesCount))) {
            this->m_error = "ReleaseBuffer: " + errorCodes->value(hr);
            emit this->errorChanged(this->m_error);

            return QByteArray();
        }
    }

    // In case of error and if the buffer is empty, return.
    if (this->m_audioBuffer.isEmpty())
        return QByteArray();

    QByteArray buffer = this->m_audioBuffer.mid(0, bufferSize);
    this->m_audioBuffer.remove(0, bufferSize);

    return buffer;
}

bool AudioDevWasapi::write(const AkAudioPacket &packet)
{
    this->m_audioBuffer.append(packet.buffer());
    int nErrors = 0;

    while (!this->m_audioBuffer.isEmpty()
           && nErrors < MAX_ERRORS_READ_WRITE) {
        // Wait until an audio frame can be writen.
        if (WaitForSingleObject(this->m_hEvent, EVENT_TIMEOUT) != WAIT_OBJECT_0) {
            nErrors++;

            continue;
        }

        HRESULT hr;
        UINT32 samplesCount;

        // Get audio buffer size in samples.
        if (FAILED(hr = this->m_pAudioClient->GetBufferSize(&samplesCount))) {
            this->m_error = "GetBufferSize: " + errorCodes->value(hr);
            emit this->errorChanged(this->m_error);

            return false;
        }

        UINT32 numSamplesPadding;

        // Get the number of samples already present in the audio buffer.
        if (FAILED(hr = this->m_pAudioClient->GetCurrentPadding(&numSamplesPadding))) {
            this->m_error = "GetCurrentPadding: " + errorCodes->value(hr);
            emit this->errorChanged(this->m_error);

            return false;
        }

        // Calculate the difference. This is the number of samples we can write
        // to the audio buffer.
        UINT32 availableSamples = samplesCount - numSamplesPadding;

        // This is probably an impossible but well... check it.
        if (availableSamples < 1)
            continue;

        // Check how many samples we can write to the audio buffer.
        UINT32 samplesInBuffer = UINT32(this->m_audioBuffer.size()
                                        * 8
                                        / this->m_curCaps.bps()
                                        / this->m_curCaps.channels());
        UINT32 samplesToWrite = qMin(availableSamples, samplesInBuffer);

        BYTE *pData = nullptr;

        // Get the audio buffer.
        if (FAILED(hr = this->m_pRenderClient->GetBuffer(samplesToWrite, &pData))) {
            this->m_error = "GetBuffer: " + errorCodes->value(hr);
            emit this->errorChanged(this->m_error);

            return false;
        }

        // Copy the maximum number of audio samples we can write to the audio
        // buffer.
        size_t bufferSize = samplesToWrite
                            * size_t(this->m_curCaps.bps()
                                     * this->m_curCaps.channels())
                            / 8;

        memcpy(pData, this->m_audioBuffer.constData(), bufferSize);
        this->m_audioBuffer.remove(0, int(bufferSize));

        // Tell audio device how many samples we had written.
        if (FAILED(hr = this->m_pRenderClient->ReleaseBuffer(samplesToWrite, 0))) {
            this->m_error = "ReleaseBuffer: " + errorCodes->value(hr);
            emit this->errorChanged(this->m_error);

            return false;
        }
    }

    return true;
}

bool AudioDevWasapi::uninit()
{
    bool ok = true;
    HRESULT hr;

    // Stop audio device.
    if (this->m_pAudioClient && FAILED(hr = this->m_pAudioClient->Stop())) {
        this->m_error = "Stop: " + errorCodes->value(hr);
        emit this->errorChanged(this->m_error);
        ok = false;
    }

    // Release interfaces.
    if (this->m_pCaptureClient) {
        this->m_pCaptureClient->Release();
        this->m_pCaptureClient = nullptr;
    }

    if (this->m_pRenderClient) {
        this->m_pRenderClient->Release();
        this->m_pRenderClient = nullptr;
    }

    if (this->m_pAudioClient) {
        this->m_pAudioClient->Release();
        this->m_pAudioClient = nullptr;
    }

    if (this->m_pDevice) {
        this->m_pDevice->Release();
        this->m_pDevice = nullptr;
    }

    if (this->m_hEvent) {
        CloseHandle(this->m_hEvent);
        this->m_hEvent = nullptr;
    }

    this->m_curDevice.clear();

    return ok;
}

HRESULT AudioDevWasapi::QueryInterface(const IID &riid, void **ppvObject)
{
    if (riid == __uuidof(IUnknown)
        || riid == __uuidof(IMMNotificationClient))
        *ppvObject = static_cast<IMMNotificationClient *>(this);
    else {
        *ppvObject = nullptr;

        return E_NOINTERFACE;
    }

    this->AddRef();

    return S_OK;
}

ULONG AudioDevWasapi::AddRef()
{
    return InterlockedIncrement(&this->m_cRef);
}

ULONG AudioDevWasapi::Release()
{
    ULONG lRef = InterlockedDecrement(&this->m_cRef);

    if (lRef == 0)
        delete this;

    return lRef;
}

bool AudioDevWasapi::waveFormatFromAk(WAVEFORMATEX *wfx,
                                      const AkAudioCaps &caps) const
{
    if (!wfx)
        return false;

    wfx->wFormatTag = caps.format() == AkAudioCaps::SampleFormat_flt?
                          WAVE_FORMAT_IEEE_FLOAT: WAVE_FORMAT_PCM;
    wfx->nChannels = WORD(caps.channels());
    wfx->nSamplesPerSec = DWORD(caps.rate());
    wfx->wBitsPerSample = WORD(caps.bps());
    wfx->nBlockAlign = wfx->nChannels * wfx->wBitsPerSample / 8;
    wfx->nAvgBytesPerSec = wfx->nSamplesPerSec * wfx->nBlockAlign;
    wfx->cbSize = 0;

    return true;
}

void AudioDevWasapi::fillDeviceInfo(const QString &device,
                                    EDataFlow dataFlow,
                                    QList<AkAudioCaps::SampleFormat> *supportedFormats,
                                    QList<int> *supportedChannels,
                                    QList<int> *supportedSampleRates) const
{
    if (!this->m_deviceEnumerator)
        return;

    HRESULT hr;
    IMMDevice *pDevice = nullptr;
    IAudioClient *pAudioClient = nullptr;

    // Test if the device is already running,
    if (this->m_curDevice != device) {
        // Get audio device.
        if (FAILED(hr = this->m_deviceEnumerator->GetDevice(device.toStdWString().c_str(),
                                                            &pDevice)))
            return;

        // Get an instance for the audio client.
        if (FAILED(hr = pDevice->Activate(__uuidof(IAudioClient),
                                          CLSCTX_ALL,
                                          nullptr,
                                          reinterpret_cast<void **>(&pAudioClient)))) {
            pDevice->Release();

            return;
        }
    } else {
        pDevice = this->m_pDevice;
        pAudioClient = this->m_pAudioClient;
    }

    static const QVector<AkAudioCaps::SampleFormat> preferredFormats = {
        AkAudioCaps::SampleFormat_flt,
        AkAudioCaps::SampleFormat_s32,
        AkAudioCaps::SampleFormat_s16,
        AkAudioCaps::SampleFormat_u8
    };

    for (auto &format: preferredFormats) {
        AkAudioCaps audioCaps(format, 1, 44100);
        WAVEFORMATEX wfx;
        WAVEFORMATEX *closestWfx = nullptr;
        this->waveFormatFromAk(&wfx, audioCaps);

        if (SUCCEEDED(hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED,
                                                           &wfx,
                                                           &closestWfx))) {
            supportedFormats->append(format);
            CoTaskMemFree(closestWfx);
        }
    }

    AkAudioCaps::SampleFormat format =
            dataFlow == eCapture?
                AkAudioCaps::SampleFormat_u8:
                AkAudioCaps::SampleFormat_s16;

    for (int channels = 1; channels < 3; channels++) {
        AkAudioCaps audioCaps(format, channels, 44100);
        WAVEFORMATEX wfx;
        WAVEFORMATEX *closestWfx = nullptr;
        this->waveFormatFromAk(&wfx, audioCaps);

        if (SUCCEEDED(hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED,
                                                           &wfx,
                                                           &closestWfx))) {
            supportedChannels->append(channels);
            CoTaskMemFree(closestWfx);
        }
    }

    for (auto &rate: this->m_commonSampleRates) {
        AkAudioCaps audioCaps(format, 1, rate);
        WAVEFORMATEX wfx;
        WAVEFORMATEX *closestWfx = nullptr;
        this->waveFormatFromAk(&wfx, audioCaps);

        if (SUCCEEDED(hr = pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED,
                                                           &wfx,
                                                           &closestWfx))) {
            supportedSampleRates->append(rate);
            CoTaskMemFree(closestWfx);
        }
    }

    if (this->m_curDevice != device) {
        pAudioClient->Release();
        pDevice->Release();
    }
}

HRESULT AudioDevWasapi::OnDeviceStateChanged(LPCWSTR pwstrDeviceId,
                                             DWORD dwNewState)
{
    Q_UNUSED(pwstrDeviceId)
    Q_UNUSED(dwNewState)

    return S_OK;
}

HRESULT AudioDevWasapi::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
    Q_UNUSED(pwstrDeviceId)

    // Device was installed

    return S_OK;
}

HRESULT AudioDevWasapi::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
    Q_UNUSED(pwstrDeviceId)

    // Device was uninstalled

    return S_OK;
}

HRESULT AudioDevWasapi::OnDefaultDeviceChanged(EDataFlow flow,
                                               ERole role,
                                               LPCWSTR pwstrDeviceId)
{
    if (role != eMultimedia)
        return S_OK;

    QString deviceId = QString::fromWCharArray(pwstrDeviceId);

    if (flow == eCapture) {
        this->m_defaultSource = deviceId;
        emit this->defaultInputChanged(deviceId);
    } else if (flow == eRender) {
        this->m_defaultSink = deviceId;
        emit this->defaultOutputChanged(deviceId);
    }

    return S_OK;
}

HRESULT AudioDevWasapi::OnPropertyValueChanged(LPCWSTR pwstrDeviceId,
                                               const PROPERTYKEY key)
{
    Q_UNUSED(pwstrDeviceId)
    Q_UNUSED(key)

    this->updateDevices();

    return S_OK;
}

void AudioDevWasapi::updateDevices()
{
    if (!this->m_deviceEnumerator) {
        this->m_error = "Device enumerator not created.";
        emit this->errorChanged(this->m_error);

        return;
    }

    decltype(this->m_sources) inputs;
    decltype(this->m_sinks) outputs;
    decltype(this->m_defaultSink) defaultSink;
    decltype(this->m_defaultSource) defaultSource;
    decltype(this->m_descriptionMap) descriptionMap;
    decltype(this->m_supportedFormats) supportedFormats;
    decltype(this->m_supportedChannels) supportedChannels;
    decltype(this->m_supportedSampleRates) supportedSampleRates;

    for (auto &dataFlow: QVector<EDataFlow> {eCapture, eRender}) {
        HRESULT hr;
        IMMDevice *defaultDevice = nullptr;

        if (SUCCEEDED(hr = this->m_deviceEnumerator->GetDefaultAudioEndpoint(dataFlow,
                                                                             eMultimedia,
                                                                             &defaultDevice))) {
            LPWSTR deviceId;

            if (SUCCEEDED(hr = defaultDevice->GetId(&deviceId))) {
                if (dataFlow == eCapture)
                    defaultSource = QString::fromWCharArray(deviceId);
                else
                    defaultSink = QString::fromWCharArray(deviceId);

                CoTaskMemFree(deviceId);
            }

            defaultDevice->Release();
        }

        IMMDeviceCollection *endPoints = nullptr;

        if (SUCCEEDED(hr = this->m_deviceEnumerator->EnumAudioEndpoints(dataFlow,
                                                                        eMultimedia,
                                                                        &endPoints))) {
            UINT nDevices = 0;

            if (SUCCEEDED(hr = endPoints->GetCount(&nDevices)))
                for (UINT i = 0; i < nDevices; i++) {
                    IMMDevice *device = nullptr;

                    if (SUCCEEDED(endPoints->Item(i, &device))) {
                        LPWSTR deviceId;

                        if (SUCCEEDED(hr = device->GetId(&deviceId))) {
                            IPropertyStore *properties = nullptr;

                            if (SUCCEEDED(hr = device->OpenPropertyStore(STGM_READ, &properties))) {
                                PROPVARIANT friendlyName;
                                PropVariantInit(&friendlyName);

                                if (SUCCEEDED(hr = properties->GetValue(PKEY_Device_FriendlyName,
                                                                        &friendlyName))) {
                                    auto devId = QString::fromWCharArray(deviceId);

                                    QList<AkAudioCaps::SampleFormat> _supportedFormats;
                                    QList<int> _supportedChannels;
                                    QList<int> _supportedSampleRates;
                                    this->fillDeviceInfo(devId,
                                                         dataFlow,
                                                         &_supportedFormats,
                                                         &_supportedChannels,
                                                         &_supportedSampleRates);

                                    if (_supportedFormats.isEmpty())
                                        _supportedFormats =
                                                this->m_supportedFormats.value(devId);

                                    if (_supportedChannels.isEmpty())
                                        _supportedChannels =
                                                this->m_supportedChannels.value(devId);

                                    if (_supportedSampleRates.isEmpty())
                                        _supportedSampleRates =
                                                this->m_supportedSampleRates.value(devId);

                                    if (!_supportedFormats.isEmpty()
                                        && !_supportedChannels.isEmpty()
                                        && !_supportedSampleRates.isEmpty()) {
                                        if (dataFlow == eCapture)
                                            inputs << devId;
                                        else
                                            outputs << devId;

                                        descriptionMap[devId] =
                                                QString::fromWCharArray(friendlyName.pwszVal);
                                        supportedFormats[devId] = _supportedFormats;
                                        supportedChannels[devId] = _supportedChannels;
                                        supportedSampleRates[devId] = _supportedSampleRates;
                                    }

                                    PropVariantClear(&friendlyName);
                                }

                                properties->Release();
                            }

                            CoTaskMemFree(deviceId);
                        }

                        device->Release();
                    }
                }

            endPoints->Release();
        }
    }

    if (this->m_supportedFormats != supportedFormats)
        this->m_supportedFormats = supportedFormats;

    if (this->m_supportedChannels != supportedChannels)
        this->m_supportedChannels = supportedChannels;

    if (this->m_supportedSampleRates != supportedSampleRates)
        this->m_supportedSampleRates = supportedSampleRates;

    if (this->m_descriptionMap != descriptionMap)
        this->m_descriptionMap = descriptionMap;

    if (this->m_sources != inputs) {
        this->m_sources = inputs;
        emit this->inputsChanged(inputs);
    }

    if (this->m_sinks != outputs) {
        this->m_sinks = outputs;
        emit this->outputsChanged(outputs);
    }

    if (defaultSource.isEmpty() && !inputs.isEmpty())
        defaultSource = inputs.first();

    if (defaultSink.isEmpty() && !outputs.isEmpty())
        defaultSink = outputs.first();

    if (this->m_defaultSource != defaultSource) {
        this->m_defaultSource = defaultSource;
        emit this->defaultInputChanged(defaultSource);
    }

    if (this->m_defaultSink != defaultSink) {
        this->m_defaultSink = defaultSink;
        emit this->defaultOutputChanged(defaultSink);
    }
}
