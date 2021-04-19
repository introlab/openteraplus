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

#include "hypnoticelement.h"

typedef QMap<HypnoticElement::OpticMode, QString> OpticModeMap;

inline OpticModeMap initOpticModeMap()
{
    OpticModeMap opticModeToStr {
        {HypnoticElement::OpticModeSpiral1         , "spiral1"         },
        {HypnoticElement::OpticModeSpiral2         , "spiral2"         },
        {HypnoticElement::OpticModeParabola        , "parabola"        },
        {HypnoticElement::OpticModeHorizontalStripe, "horizontalStripe"}
    };

    return opticModeToStr;
}

Q_GLOBAL_STATIC_WITH_ARGS(OpticModeMap, opticModeToStr, (initOpticModeMap()))

HypnoticElement::HypnoticElement(): AkElement()
{
    this->m_mode = OpticModeSpiral1;
    this->m_speedInc = 0;
    this->m_threshold = 127;

    this->m_palette = this->createPalette();
}

QString HypnoticElement::mode() const
{
    return opticModeToStr->value(this->m_mode);
}

int HypnoticElement::speedInc() const
{
    return this->m_speedInc;
}

int HypnoticElement::threshold() const
{
    return this->m_threshold;
}

QVector<QRgb> HypnoticElement::createPalette()
{
    QVector<QRgb> palette(256);

    for (int i = 0; i < 112; i++) {
        palette[i] = qRgb(0, 0, 0);
        palette[i + 128] = qRgb(255, 255, 255);
    }

    for (int i = 0; i < 16; i++) {
        QRgb color = QRgb(16 * (i + 1) - 1);
        palette[i + 112] = qRgb(qRed(color), qGreen(color), qBlue(color));
        color = 255 - color;
        palette[i + 240] = qRgb(qRed(color), qGreen(color), qBlue(color));
    }

    return palette;
}

HypnoticElement::OpticalMap HypnoticElement::createOpticalMap(const QSize &size)
{
    OpticalMap opticalMap;
    int sci = 640 / size.width();

    opticalMap[OpticModeSpiral1] = QImage(size, QImage::Format_Indexed8);
    opticalMap[OpticModeSpiral2] = QImage(size, QImage::Format_Indexed8);
    opticalMap[OpticModeParabola] = QImage(size, QImage::Format_Indexed8);
    opticalMap[OpticModeHorizontalStripe] = QImage(size, QImage::Format_Indexed8);

    for (int y = 0; y < size.height(); y++) {
        qreal yy = qreal(y - size.height() / 2) / size.width();

        quint8 *spiral1Line = opticalMap[OpticModeSpiral1].scanLine(y);
        quint8 *spiral2Line = opticalMap[OpticModeSpiral2].scanLine(y);
        quint8 *parabolaLine = opticalMap[OpticModeParabola].scanLine(y);
        quint8 *horizontalStripeLine = opticalMap[OpticModeHorizontalStripe].scanLine(y);

        for (int x = 0; x < size.width(); x++) {
            qreal xx = qreal(x) / size.width() - 0.5;

            qreal r = sqrt(xx * xx + yy * yy);
            qreal at = atan2(xx, yy);

            spiral1Line[x] = (uint((at / M_PI * 256) + (r * 4000))) & 255;

            int j = int(r * 300 / 32);
            qreal rr = r * 300 - j * 32;

            j *= 64;
            j += (rr > 28)? (rr - 28) * 16: 0;

            spiral2Line[x] = (uint((at / M_PI * 4096) + (r * 1600) - j)) & 255;
            parabolaLine[x] = (uint(yy / (xx * xx * 0.3 + 0.1) * 400)) & 255;
            horizontalStripeLine[x] = quint8(x * 8 * sci);
        }
    }

    return opticalMap;
}

QImage HypnoticElement::imageThreshold(const QImage &src, int threshold)
{
    QImage diff(src.size(), QImage::Format_Grayscale8);

    for (int y = 0; y < src.height(); y++) {
        const QRgb *srcBits = reinterpret_cast<const QRgb *>(src.constScanLine(y));
        quint8 *diffBits = diff.scanLine(y);

        for (int x = 0; x < src.width(); x++)
            diffBits[x] = qGray(srcBits[x]) >= threshold? 255: 0;
    }

    return diff;
}

QString HypnoticElement::controlInterfaceProvide(const QString &controlId) const
{
    Q_UNUSED(controlId)

    return QString("qrc:/Hypnotic/share/qml/main.qml");
}

void HypnoticElement::controlInterfaceConfigure(QQmlContext *context,
                                                const QString &controlId) const
{
    Q_UNUSED(controlId)

    context->setContextProperty("Hypnotic", const_cast<QObject *>(qobject_cast<const QObject *>(this)));
    context->setContextProperty("controlId", this->objectName());
}

void HypnoticElement::setMode(const QString &mode)
{
    OpticMode opticMode = opticModeToStr->key(mode, OpticModeSpiral1);

    if (this->m_mode == opticMode)
        return;

    this->m_mode = opticMode;
    emit this->modeChanged(mode);
}

void HypnoticElement::setSpeedInc(int speedInc)
{
    if (this->m_speedInc == speedInc)
        return;

    this->m_speedInc = speedInc;
    emit this->speedIncChanged(speedInc);
}

void HypnoticElement::setThreshold(int threshold)
{
    if (this->m_threshold == threshold)
        return;

    this->m_threshold = threshold;
    emit this->thresholdChanged(threshold);
}

void HypnoticElement::resetMode()
{
    this->setMode("spiral1");
}

void HypnoticElement::resetSpeedInc()
{
    this->setSpeedInc(0);
}

void HypnoticElement::resetThreshold()
{
    this->setThreshold(127);
}

AkPacket HypnoticElement::iStream(const AkPacket &packet)
{
    QImage src = AkUtils::packetToImage(packet);

    if (src.isNull())
        return AkPacket();

    src = src.convertToFormat(QImage::Format_ARGB32);
    QImage oFrame(src.size(), src.format());

    if (src.size() != this->m_frameSize) {
        this->m_speed = 16;
        this->m_phase = 0;
        this->m_opticalMap = this->createOpticalMap(src.size());
        this->m_frameSize = src.size();
    }

    QImage opticalMap = this->m_opticalMap.value(this->m_mode,
                                                 this->m_opticalMap[OpticModeSpiral1]);

    this->m_speed += this->m_speedInc;
    this->m_phase -= this->m_speed;

    QImage diff = this->imageThreshold(src, this->m_threshold);

    for (int i = 0, y = 0; y < src.height(); y++) {
        QRgb *oLine = reinterpret_cast<QRgb *>(oFrame.scanLine(y));
        const quint8 *optLine = opticalMap.constScanLine(y);
        const quint8 *diffLine = diff.constScanLine(y);

        for (int x = 0; x < src.width(); i++, x++)
            oLine[x] = this->m_palette[((char(optLine[x] + this->m_phase)) ^ diffLine[x]) & 255];
    }

    AkPacket oPacket = AkUtils::imageToPacket(oFrame, packet);
    akSend(oPacket)
}
