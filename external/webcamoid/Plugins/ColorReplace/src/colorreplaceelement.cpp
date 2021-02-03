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

#include "colorreplaceelement.h"

ColorReplaceElement::ColorReplaceElement(): AkElement()
{
    this->m_from = qRgb(0, 0, 0);
    this->m_to = qRgb(0, 0, 0);
    this->m_radius = 1.0;
    this->m_disable = false;
}

QRgb ColorReplaceElement::from() const
{
    return this->m_from;
}

QRgb ColorReplaceElement::to() const
{
    return this->m_to;
}

qreal ColorReplaceElement::radius() const
{
    return this->m_radius;
}

bool ColorReplaceElement::disable() const
{
    return this->m_disable;
}

QString ColorReplaceElement::controlInterfaceProvide(const QString &controlId) const
{
    Q_UNUSED(controlId)

    return QString("qrc:/ColorReplace/share/qml/main.qml");
}

void ColorReplaceElement::controlInterfaceConfigure(QQmlContext *context,
                                                    const QString &controlId) const
{
    Q_UNUSED(controlId)

    context->setContextProperty("ColorReplace", const_cast<QObject *>(qobject_cast<const QObject *>(this)));
    context->setContextProperty("controlId", this->objectName());
}

void ColorReplaceElement::setFrom(QRgb from)
{
    if (this->m_from == from)
        return;

    this->m_from = from;
    emit this->fromChanged(from);
}

void ColorReplaceElement::setTo(QRgb to)
{
    if (this->m_to == to)
        return;

    this->m_to = to;
    emit this->toChanged(to);
}

void ColorReplaceElement::setRadius(qreal radius)
{
    if (qFuzzyCompare(this->m_radius, radius))
        return;

    this->m_radius = radius;
    emit this->radiusChanged(radius);
}

void ColorReplaceElement::setDisable(bool disable)
{
    if (this->m_disable == disable)
        return;

    this->m_disable = disable;
    emit this->disableChanged(disable);
}

void ColorReplaceElement::resetFrom()
{
    this->setFrom(qRgb(0, 0, 0));
}

void ColorReplaceElement::resetTo()
{
    this->setTo(qRgb(0, 0, 0));
}

void ColorReplaceElement::resetRadius()
{
    this->setRadius(1.0);
}

void ColorReplaceElement::resetDisable()
{
    this->setDisable(false);
}

AkPacket ColorReplaceElement::iStream(const AkPacket &packet)
{
    if (this->m_disable)
        akSend(packet)

    QImage src = AkUtils::packetToImage(packet);

    if (src.isNull())
        return AkPacket();

    src = src.convertToFormat(QImage::Format_ARGB32);
    QImage oFrame(src.size(), src.format());

    for (int y = 0; y < src.height(); y++) {
        const QRgb *srcLine = reinterpret_cast<const QRgb *>(src.constScanLine(y));
        QRgb *dstLine = reinterpret_cast<QRgb *>(oFrame.scanLine(y));

        for (int x = 0; x < src.width(); x++) {
            int r = qRed(srcLine[x]);
            int g = qGreen(srcLine[x]);
            int b = qBlue(srcLine[x]);

            int rf = qRed(this->m_from);
            int gf = qGreen(this->m_from);
            int bf = qBlue(this->m_from);

            int rd = r - rf;
            int gd = g - gf;
            int bd = b - bf;

            qreal k = sqrt(rd * rd + gd * gd + bd * bd);

            if (k <= this->m_radius) {
                qreal p = k / this->m_radius;

                int rt = qRed(this->m_to);
                int gt = qGreen(this->m_to);
                int bt = qBlue(this->m_to);

                r = int(p * (r - rt) + rt);
                g = int(p * (g - gt) + gt);
                b = int(p * (b - bt) + bt);

                dstLine[x] = qRgba(r, g, b, qAlpha(srcLine[x]));
            } else
                dstLine[x] = srcLine[x];
        }
    }

    AkPacket oPacket = AkUtils::imageToPacket(oFrame, packet);
    akSend(oPacket)
}
