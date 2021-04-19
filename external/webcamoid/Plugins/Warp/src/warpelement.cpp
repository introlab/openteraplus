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

#include <QtMath>

#include "warpelement.h"

WarpElement::WarpElement(): AkElement()
{
    this->m_ripples = 4;
}

qreal WarpElement::ripples() const
{
    return this->m_ripples;
}

QString WarpElement::controlInterfaceProvide(const QString &controlId) const
{
    Q_UNUSED(controlId)

    return QString("qrc:/Warp/share/qml/main.qml");
}

void WarpElement::controlInterfaceConfigure(QQmlContext *context,
                                            const QString &controlId) const
{
    Q_UNUSED(controlId)

    context->setContextProperty("Warp", const_cast<QObject *>(qobject_cast<const QObject *>(this)));
    context->setContextProperty("controlId", this->objectName());
}

void WarpElement::setRipples(qreal ripples)
{
    if (qFuzzyCompare(this->m_ripples, ripples))
        return;

    this->m_ripples = ripples;
    emit this->ripplesChanged(ripples);
}

void WarpElement::resetRipples()
{
    this->setRipples(4);
}

AkPacket WarpElement::iStream(const AkPacket &packet)
{
    QImage src = AkUtils::packetToImage(packet);

    if (src.isNull())
        return AkPacket();

    src = src.convertToFormat(QImage::Format_ARGB32);
    QImage oFrame(src.size(), src.format());

    if (src.size() != this->m_frameSize) {
        int cx = src.width() >> 1;
        int cy = src.height() >> 1;

        qreal k = 2.0 * M_PI / sqrt(cx * cx + cy * cy);

        this->m_phiTable.clear();

        for (int y = -cy; y < cy; y++)
            for (int x = -cx; x < cx; x++)
                this->m_phiTable << k * sqrt(x * x + y * y);

        this->m_frameSize = src.size();
        emit this->frameSizeChanged(this->m_frameSize);
    }

    static int tval = 0;

    qreal dx = 30 * sin((tval + 100) * M_PI / 128)
               + 40 * sin((tval - 10) * M_PI / 512);

    qreal dy = -35 * sin(tval * M_PI / 256)
               + 40 * sin((tval + 30) * M_PI / 512);

    qreal ripples = this->m_ripples * sin((tval - 70) * M_PI / 64);

    tval = (tval + 1) & 511;
    qreal *phiTable = this->m_phiTable.data();

    for (int y = 0, i = 0; y < src.height(); y++) {
        auto oLine = reinterpret_cast<QRgb *>(oFrame.scanLine(y));

        for (int x = 0; x < src.width(); x++, i++) {
            qreal phi = ripples * phiTable[i];

            int xOrig = int(dx * cos(phi) + x);
            int yOrig = int(dy * sin(phi) + y);

            xOrig = qBound(0, xOrig, src.width());
            yOrig = qBound(0, yOrig, src.height());

            auto iLine = reinterpret_cast<const QRgb *>(src.constScanLine(yOrig));
            oLine[x] = iLine[xOrig];
        }
    }

    AkPacket oPacket = AkUtils::imageToPacket(oFrame, packet);
    akSend(oPacket)
}
