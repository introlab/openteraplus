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

#include "frameoverlapelement.h"

FrameOverlapElement::FrameOverlapElement(): AkElement()
{
    this->m_nFrames = 16;
    this->m_stride = 4;
}

int FrameOverlapElement::nFrames() const
{
    return this->m_nFrames;
}

int FrameOverlapElement::stride() const
{
    return this->m_stride;
}

QString FrameOverlapElement::controlInterfaceProvide(const QString &controlId) const
{
    Q_UNUSED(controlId)

    return QString("qrc:/FrameOverlap/share/qml/main.qml");
}

void FrameOverlapElement::controlInterfaceConfigure(QQmlContext *context,
                                                    const QString &controlId) const
{
    Q_UNUSED(controlId)

    context->setContextProperty("FrameOverlap", const_cast<QObject *>(qobject_cast<const QObject *>(this)));
    context->setContextProperty("controlId", this->objectName());
}

void FrameOverlapElement::setNFrames(int nFrames)
{
    if (this->m_nFrames == nFrames)
        return;

    this->m_nFrames = nFrames;
    emit this->nFramesChanged(nFrames);
}

void FrameOverlapElement::setStride(int stride)
{
    if (this->m_stride == stride)
        return;

    this->m_stride = stride;
    emit this->strideChanged(stride);
}

void FrameOverlapElement::resetNFrames()
{
    this->setNFrames(16);
}

void FrameOverlapElement::resetStride()
{
    this->setStride(4);
}

AkPacket FrameOverlapElement::iStream(const AkPacket &packet)
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

    this->m_frames << src.copy();
    int diff = this->m_frames.size() - this->m_nFrames;

    for (int i = 0; i < diff; i++)
        this->m_frames.removeFirst();

    int stride = this->m_stride > 0? this->m_stride: 1;

    for (int y = 0; y < oFrame.height(); y++) {
        QRgb *dstBits = reinterpret_cast<QRgb *>(oFrame.scanLine(y));

        for (int x = 0; x < oFrame.width(); x++) {
            int r = 0;
            int g = 0;
            int b = 0;
            int a = 0;
            int n = 0;

            for (int frame = this->m_frames.size() - 1;
                 frame >= 0;
                 frame -= stride) {
                QRgb pixel = this->m_frames[frame].pixel(x, y);

                r += qRed(pixel);
                g += qGreen(pixel);
                b += qBlue(pixel);
                a += qAlpha(pixel);
                n++;
            }

            if (n > 0) {
                r /= n;
                g /= n;
                b /= n;
                a /= n;

                dstBits[x] = qRgba(r, g, b, a);
            } else {
                dstBits[x] = qRgba(0, 0, 0, 0);
            }
        }
    }

    AkPacket oPacket = AkUtils::imageToPacket(oFrame, packet);
    akSend(oPacket)
}
