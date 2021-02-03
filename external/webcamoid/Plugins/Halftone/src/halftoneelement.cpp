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

#include <QColor>
#include <QStandardPaths>

#include "halftoneelement.h"

HalftoneElement::HalftoneElement(): AkElement()
{
    this->m_pattern = ":/Halftone/share/patterns/ditherCluster8Matrix.bmp";
    this->m_lightness = 0.5;
    this->m_slope = 1.0;
    this->m_intercept = 0.0;

    this->updatePattern();

    QObject::connect(this,
                     &HalftoneElement::patternChanged,
                     this,
                     &HalftoneElement::updatePattern);
    QObject::connect(this,
                     &HalftoneElement::patternSizeChanged,
                     this,
                     &HalftoneElement::updatePattern);
}

QString HalftoneElement::pattern() const
{
    return this->m_pattern;
}

QSize HalftoneElement::patternSize() const
{
    return this->m_patternSize;
}

qreal HalftoneElement::lightness() const
{
    return this->m_lightness;
}

qreal HalftoneElement::slope() const
{
    return this->m_slope;
}

qreal HalftoneElement::intercept() const
{
    return this->m_intercept;
}

void HalftoneElement::updatePattern()
{
    if (this->m_pattern.isEmpty()) {
        this->m_mutex.lock();
        this->m_patternImage = QImage();
        this->m_mutex.unlock();

        return;
    }

    QImage image(this->m_pattern);

    if (image.isNull()) {
        this->m_mutex.lock();
        this->m_patternImage = QImage();
        this->m_mutex.unlock();

        return;
    }

    QSize patternSize = this->m_patternSize.isEmpty()?
                            image.size(): this->m_patternSize;
    QImage pattern(patternSize, QImage::Format_Indexed8);

    for (int i = 0; i < 256; i++)
        pattern.setColor(i, qRgb(i, i, i));

    image = image.scaled(patternSize).convertToFormat(QImage::Format_RGB32);

    for (int y = 0; y < patternSize.height(); y++) {
        const QRgb *srcLine = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        quint8 *dstLine = pattern.scanLine(y);

        for (int x = 0; x < patternSize.width(); x++)
            dstLine[x] = quint8(qGray(srcLine[x]));
    }

    this->m_mutex.lock();
    this->m_patternImage = pattern;
    this->m_mutex.unlock();
}

QString HalftoneElement::controlInterfaceProvide(const QString &controlId) const
{
    Q_UNUSED(controlId)

    return QString("qrc:/Halftone/share/qml/main.qml");
}

void HalftoneElement::controlInterfaceConfigure(QQmlContext *context,
                                                const QString &controlId) const
{
    Q_UNUSED(controlId)

    context->setContextProperty("Halftone", const_cast<QObject *>(qobject_cast<const QObject *>(this)));
    context->setContextProperty("controlId", this->objectName());

    QStringList picturesPath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    context->setContextProperty("picturesPath", picturesPath[0]);
}

void HalftoneElement::setPattern(const QString &pattern)
{
    if (this->m_pattern == pattern)
        return;

    this->m_pattern = pattern;
    emit this->patternChanged(pattern);
}

void HalftoneElement::setPatternSize(const QSize &patternSize)
{
    if (this->m_patternSize == patternSize)
        return;

    this->m_patternSize = patternSize;
    emit this->patternSizeChanged(patternSize);
}

void HalftoneElement::setLightness(qreal lightness)
{
    if (qFuzzyCompare(this->m_lightness, lightness))
        return;

    this->m_lightness = lightness;
    emit this->lightnessChanged(lightness);
}

void HalftoneElement::setSlope(qreal slope)
{
    if (qFuzzyCompare(this->m_slope, slope))
        return;

    this->m_slope = slope;
    emit this->slopeChanged(slope);
}

void HalftoneElement::setIntercept(qreal intercept)
{
    if (qFuzzyCompare(this->m_intercept, intercept))
        return;

    this->m_intercept = intercept;
    emit this->interceptChanged(intercept);
}

void HalftoneElement::resetPattern()
{
    this->setPattern(":/Halftone/share/patterns/ditherCluster8Matrix.bmp");
}

void HalftoneElement::resetPatternSize()
{
    this->setPatternSize(QSize());
}

void HalftoneElement::resetLightness()
{
    this->setLightness(0.5);
}

void HalftoneElement::resetSlope()
{
    this->setSlope(1.0);
}

void HalftoneElement::resetIntercept()
{
    this->setIntercept(0.0);
}

AkPacket HalftoneElement::iStream(const AkPacket &packet)
{
    QImage src = AkUtils::packetToImage(packet);

    if (src.isNull())
        return AkPacket();

    src = src.convertToFormat(QImage::Format_ARGB32);
    QImage oFrame(src.size(), src.format());

    this->m_mutex.lock();

    if (this->m_patternImage.isNull()) {
        this->m_mutex.unlock();
        akSend(packet)
    }

    QImage patternImage = this->m_patternImage.copy();
    this->m_mutex.unlock();

    // filter image
    for (int y = 0; y < src.height(); y++) {
        const QRgb *iLine = reinterpret_cast<const QRgb *>(src.constScanLine(y));
        QRgb *oLine = reinterpret_cast<QRgb *>(oFrame.scanLine(y));

        for (int x = 0; x < src.width(); x++) {
            int col = x % patternImage.width();
            int row = y % patternImage.height();

            int gray = qGray(iLine[x]);
            const quint8 *pattern = reinterpret_cast<const quint8 *>(patternImage.constScanLine(row));
            int threshold = pattern[col];
            threshold = int(this->m_slope * threshold + this->m_intercept);
            threshold = qBound(0, threshold, 255);

            if (gray > threshold)
                oLine[x] = iLine[x];
            else {
                QColor color(iLine[x]);

                color.setHsl(color.hue(),
                             color.saturation(),
                             int(this->m_lightness * color.lightness()),
                             color.alpha());

                oLine[x] = color.rgba();
            }
        }
    }

    AkPacket oPacket = AkUtils::imageToPacket(oFrame, packet);
    akSend(oPacket)
}
