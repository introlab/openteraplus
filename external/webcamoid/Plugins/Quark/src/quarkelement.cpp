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

#include "quarkelement.h"

QuarkElement::QuarkElement(): AkElement()
{
    this->m_nFrames = 16;
}

int QuarkElement::nFrames() const
{
    return this->m_nFrames;
}

QString QuarkElement::controlInterfaceProvide(const QString &controlId) const
{
    Q_UNUSED(controlId)

    return QString("qrc:/Quark/share/qml/main.qml");
}

void QuarkElement::controlInterfaceConfigure(QQmlContext *context, const QString &controlId) const
{
    Q_UNUSED(controlId)

    context->setContextProperty("Quark", const_cast<QObject *>(qobject_cast<const QObject *>(this)));
    context->setContextProperty("controlId", this->objectName());
}

void QuarkElement::setNFrames(int nFrames)
{
    if (this->m_nFrames == nFrames)
        return;

    this->m_nFrames = nFrames;
    emit this->nFramesChanged(nFrames);
}

void QuarkElement::resetNFrames()
{
    this->setNFrames(16);
}

AkPacket QuarkElement::iStream(const AkPacket &packet)
{
    QImage src = AkUtils::packetToImage(packet);

    if (src.isNull())
        return AkPacket();

    src = src.convertToFormat(QImage::Format_ARGB32);
    QImage oFrame(src.size(), src.format());

    if (src.size() != this->m_frameSize) {
        this->m_frames.clear();
        this->m_frameSize = src.size();
    }

    int nFrames = this->m_nFrames > 0? this->m_nFrames: 1;
    this->m_frames << src.copy();
    int diff = this->m_frames.size() - nFrames;

    for (int i = 0; i < diff; i++)
        this->m_frames.removeFirst();

    for (int y = 0; y < src.height(); y++) {
        QRgb *dstLine = reinterpret_cast<QRgb *>(oFrame.scanLine(y));

        for (int x = 0; x < src.width(); x++) {
            int frame = qrand() % this->m_frames.size();
            dstLine[x] = this->m_frames[frame].pixel(x, y);
        }
    }

    AkPacket oPacket = AkUtils::imageToPacket(oFrame, packet);
    akSend(oPacket)
}
