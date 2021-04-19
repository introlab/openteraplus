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

#include "desktopcaptureelement.h"
#include "desktopcaptureglobals.h"

Q_GLOBAL_STATIC(DesktopCaptureGlobals, globalDesktopCapture)

template<typename T>
inline QSharedPointer<T> ptr_init(QObject *obj=nullptr)
{
    if (!obj)
        return QSharedPointer<T>(new T());

    return QSharedPointer<T>(static_cast<T *>(obj));
}

DesktopCaptureElement::DesktopCaptureElement():
    AkMultimediaSourceElement(),
    m_screenCapture(ptr_init<ScreenDev>())
{
    QObject::connect(globalDesktopCapture,
                     SIGNAL(captureLibChanged(const QString &)),
                     this,
                     SIGNAL(captureLibChanged(const QString &)));
    QObject::connect(globalDesktopCapture,
                     SIGNAL(captureLibChanged(const QString &)),
                     this,
                     SLOT(captureLibUpdated(const QString &)));

    this->captureLibUpdated(globalDesktopCapture->captureLib());
}

DesktopCaptureElement::~DesktopCaptureElement()
{
    this->setState(AkElement::ElementStateNull);
}

AkFrac DesktopCaptureElement::fps() const
{
    return this->m_screenCapture->fps();
}

QStringList DesktopCaptureElement::medias()
{
    return this->m_screenCapture->medias();
}

QString DesktopCaptureElement::media() const
{
    return this->m_screenCapture->media();
}

QList<int> DesktopCaptureElement::streams() const
{
    return this->m_screenCapture->streams();
}

int DesktopCaptureElement::defaultStream(const QString &mimeType)
{
    return this->m_screenCapture->defaultStream(mimeType);
}

QString DesktopCaptureElement::description(const QString &media)
{
    return this->m_screenCapture->description(media);
}

AkCaps DesktopCaptureElement::caps(int stream)
{
    return this->m_screenCapture->caps(stream);
}

QString DesktopCaptureElement::captureLib() const
{
    return globalDesktopCapture->captureLib();
}

int DesktopCaptureElement::width() const
{
    return this->m_screenCapture->width();
}

int DesktopCaptureElement::height() const
{
    return this->m_screenCapture->height();;
}

QString DesktopCaptureElement::controlInterfaceProvide(const QString &controlId) const
{
    Q_UNUSED(controlId)

    return QString("qrc:/DesktopCapture/share/qml/main.qml");
}

void DesktopCaptureElement::controlInterfaceConfigure(QQmlContext *context,
                                                      const QString &controlId) const
{
    Q_UNUSED(controlId)

    context->setContextProperty("DesktopCapture", const_cast<QObject *>(qobject_cast<const QObject *>(this)));
    context->setContextProperty("controlId", this->objectName());
}

void DesktopCaptureElement::setFps(const AkFrac &fps)
{
    this->m_screenCapture->setFps(fps);
}

void DesktopCaptureElement::resetFps()
{
    this->m_screenCapture->resetFps();
}

void DesktopCaptureElement::setMedia(const QString &media)
{
    this->m_screenCapture->setMedia(media);
}

void DesktopCaptureElement::resetMedia()
{
    this->m_screenCapture->resetMedia();
}

void DesktopCaptureElement::setCaptureLib(const QString &captureLib)
{
    globalDesktopCapture->setCaptureLib(captureLib);
}

void DesktopCaptureElement::resetCaptureLib()
{
    globalDesktopCapture->resetCaptureLib();
}

bool DesktopCaptureElement::setState(AkElement::ElementState state)
{
    AkElement::ElementState curState = this->state();

    switch (curState) {
    case AkElement::ElementStateNull: {
        switch (state) {
        case AkElement::ElementStatePaused:
            return AkElement::setState(state);
        case AkElement::ElementStatePlaying:
            if (!this->m_screenCapture->init())
                return false;

            return AkElement::setState(state);
        case AkElement::ElementStateNull:
            break;
        }

        break;
    }
    case AkElement::ElementStatePaused: {
        switch (state) {
        case AkElement::ElementStateNull:
            return AkElement::setState(state);
        case AkElement::ElementStatePlaying:
            if (!this->m_screenCapture->init())
                return false;

            return AkElement::setState(state);
        case AkElement::ElementStatePaused:
            break;
        }

        break;
    }
    case AkElement::ElementStatePlaying: {
        switch (state) {
        case AkElement::ElementStateNull:
        case AkElement::ElementStatePaused:
            this->m_screenCapture->uninit();

            return AkElement::setState(state);
        case AkElement::ElementStatePlaying:
            break;
        }

        break;
    }
    }

    return false;
}

void DesktopCaptureElement::setWidth(const int width)
{
   this->m_screenCapture->setWidth(width);
}

void DesktopCaptureElement::resetWidth()
{
   this->m_screenCapture->resetWidth();
}

void DesktopCaptureElement::setHeight(const int height)
{
    this->m_screenCapture->setHeight(height);
}

void DesktopCaptureElement::resetHeight()
{
    this->m_screenCapture->resetHeight();
}

void DesktopCaptureElement::captureLibUpdated(const QString &captureLib)
{
    auto state = this->state();
    this->setState(AkElement::ElementStateNull);

    this->m_screenCapture =
            ptr_init<ScreenDev>(this->loadSubModule("DesktopCapture",
                                                    captureLib));

    QObject::connect(this->m_screenCapture.data(),
                     &ScreenDev::mediasChanged,
                     this,
                     &DesktopCaptureElement::mediasChanged);
    QObject::connect(this->m_screenCapture.data(),
                     &ScreenDev::mediaChanged,
                     this,
                     &DesktopCaptureElement::mediaChanged);
    QObject::connect(this->m_screenCapture.data(),
                     &ScreenDev::streamsChanged,
                     this,
                     &DesktopCaptureElement::streamsChanged);
    QObject::connect(this->m_screenCapture.data(),
                     &ScreenDev::streamsChanged,
                     this,
                     &DesktopCaptureElement::streamsChanged);
    QObject::connect(this->m_screenCapture.data(),
                     &ScreenDev::fpsChanged,
                     this,
                     &DesktopCaptureElement::fpsChanged);
    QObject::connect(this->m_screenCapture.data(),
                     &ScreenDev::sizeChanged,
                     this,
                     &DesktopCaptureElement::sizeChanged);
    QObject::connect(this->m_screenCapture.data(),
                     &ScreenDev::oStream,
                     this,
                     &DesktopCaptureElement::oStream,
                     Qt::DirectConnection);

    emit this->mediasChanged(this->medias());
    emit this->streamsChanged(this->streams());

    auto medias = this->medias();

    if (!medias.isEmpty())
        this->setMedia(medias.first());

    this->setState(state);
}
