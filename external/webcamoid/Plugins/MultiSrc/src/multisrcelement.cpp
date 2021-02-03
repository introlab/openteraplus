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

#include "multisrcelement.h"
#include "multisrcglobals.h"

Q_GLOBAL_STATIC(MultiSrcGlobals, globalMultiSrc)

template<typename T>
inline QSharedPointer<T> ptr_init(QObject *obj=nullptr)
{
    if (!obj)
        return QSharedPointer<T>(new T());

    return QSharedPointer<T>(static_cast<T *>(obj));
}

MultiSrcElement::MultiSrcElement():
    AkMultimediaSourceElement(),
    m_mediaSource(ptr_init<MediaSource>())
{
    QObject::connect(globalMultiSrc,
                     SIGNAL(codecLibChanged(const QString &)),
                     this,
                     SIGNAL(codecLibChanged(const QString &)));
    QObject::connect(globalMultiSrc,
                     SIGNAL(codecLibChanged(const QString &)),
                     this,
                     SLOT(codecLibUpdated(const QString &)));

    this->codecLibUpdated(globalMultiSrc->codecLib());
}

MultiSrcElement::~MultiSrcElement()
{
    this->setState(AkElement::ElementStateNull);
}

QStringList MultiSrcElement::medias()
{
    return this->m_mediaSource->medias();
}

QString MultiSrcElement::media() const
{
    return this->m_mediaSource->media();
}

QList<int> MultiSrcElement::streams() const
{
    return this->m_mediaSource->streams();
}

bool MultiSrcElement::loop() const
{
    return this->m_mediaSource->loop();
}

QList<int> MultiSrcElement::listTracks(const QString &type)
{
    return this->m_mediaSource->listTracks(type);
}

QString MultiSrcElement::streamLanguage(int stream)
{
    return this->m_mediaSource->streamLanguage(stream);
}

int MultiSrcElement::defaultStream(const QString &mimeType)
{
    return this->m_mediaSource->defaultStream(mimeType);
}

QString MultiSrcElement::description(const QString &media)
{
    return this->m_mediaSource->description(media);
}

AkCaps MultiSrcElement::caps(int stream)
{
    return this->m_mediaSource->caps(stream);
}

qint64 MultiSrcElement::maxPacketQueueSize() const
{
    return this->m_mediaSource->maxPacketQueueSize();
}

bool MultiSrcElement::showLog() const
{
    return this->m_mediaSource->showLog();
}

QString MultiSrcElement::codecLib() const
{
    return globalMultiSrc->codecLib();
}

QString MultiSrcElement::controlInterfaceProvide(const QString &controlId) const
{
    Q_UNUSED(controlId)

    return QString("qrc:/MultiSrc/share/qml/main.qml");
}

void MultiSrcElement::controlInterfaceConfigure(QQmlContext *context,
                                                const QString &controlId) const
{
    Q_UNUSED(controlId)

    context->setContextProperty("MultiSrc", const_cast<QObject *>(qobject_cast<const QObject *>(this)));
    context->setContextProperty("controlId", this->objectName());
}

void MultiSrcElement::setMedia(const QString &media)
{
    this->m_mediaSource->setMedia(media);
}

void MultiSrcElement::setStreams(const QList<int> &streams)
{
    this->m_mediaSource->setStreams(streams);
}

void MultiSrcElement::setLoop(bool loop)
{
    this->m_mediaSource->setLoop(loop);
}

void MultiSrcElement::setMaxPacketQueueSize(qint64 maxPacketQueueSize)
{
    this->m_mediaSource->setMaxPacketQueueSize(maxPacketQueueSize);
}

void MultiSrcElement::setShowLog(bool showLog)
{
    this->m_mediaSource->setShowLog(showLog);
}

void MultiSrcElement::setCodecLib(const QString &codecLib)
{
    globalMultiSrc->setCodecLib(codecLib);
}

void MultiSrcElement::resetMedia()
{
    this->m_mediaSource->resetMedia();
}

void MultiSrcElement::resetStreams()
{
    this->m_mediaSource->resetStreams();
}

void MultiSrcElement::resetLoop()
{
    this->m_mediaSource->resetLoop();
}

void MultiSrcElement::resetMaxPacketQueueSize()
{
    this->m_mediaSource->resetMaxPacketQueueSize();
}

void MultiSrcElement::resetShowLog()
{
    this->m_mediaSource->resetShowLog();
}

void MultiSrcElement::resetCodecLib()
{
    globalMultiSrc->resetCodecLib();
}

bool MultiSrcElement::setState(AkElement::ElementState state)
{
    if (!this->m_mediaSource->setState(state))
        return false;

    return AkElement::setState(state);
}

void MultiSrcElement::codecLibUpdated(const QString &codecLib)
{
    auto state = this->state();
    this->setState(AkElement::ElementStateNull);

    auto media = this->m_mediaSource->media();
    auto loop = this->m_mediaSource->loop();
    auto showLog = this->m_mediaSource->showLog();

    this->m_mutexLib.lock();

    this->m_mediaSource =
            ptr_init<MediaSource>(this->loadSubModule("MultiSrc", codecLib));

    QObject::connect(this->m_mediaSource.data(),
                     SIGNAL(oStream(const AkPacket &)),
                     this,
                     SIGNAL(oStream(const AkPacket &)),
                     Qt::DirectConnection);
    QObject::connect(this->m_mediaSource.data(),
                     SIGNAL(error(const QString &)),
                     this,
                     SIGNAL(error(const QString &)));
    QObject::connect(this->m_mediaSource.data(),
                     SIGNAL(maxPacketQueueSizeChanged(qint64)),
                     this,
                     SIGNAL(maxPacketQueueSizeChanged(qint64)));
    QObject::connect(this->m_mediaSource.data(),
                     SIGNAL(showLogChanged(bool)),
                     this,
                     SIGNAL(showLogChanged(bool)));
    QObject::connect(this->m_mediaSource.data(),
                     SIGNAL(loopChanged(bool)),
                     this,
                     SIGNAL(loopChanged(bool)));
    QObject::connect(this->m_mediaSource.data(),
                     SIGNAL(mediasChanged(const QStringList &)),
                     this,
                     SIGNAL(mediasChanged(const QStringList &)));
    QObject::connect(this->m_mediaSource.data(),
                     SIGNAL(mediaChanged(const QString &)),
                     this,
                     SIGNAL(mediaChanged(const QString &)));
    QObject::connect(this->m_mediaSource.data(),
                     SIGNAL(streamsChanged(const QList<int> &)),
                     this,
                     SIGNAL(streamsChanged(const QList<int> &)));

    this->m_mutexLib.unlock();

    this->m_mediaSource->setMedia(media);
    this->m_mediaSource->setLoop(loop);
    this->m_mediaSource->setShowLog(showLog);

    emit this->streamsChanged(this->streams());
    emit this->maxPacketQueueSizeChanged(this->maxPacketQueueSize());

    this->setState(state);
}
