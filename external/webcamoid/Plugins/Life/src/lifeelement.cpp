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
#include <QPainter>

#include "lifeelement.h"

LifeElement::LifeElement(): AkElement()
{
    this->m_lifeColor = qRgb(255, 255, 255);
    this->m_threshold = 15;
    this->m_lumaThreshold = 15;
}

QRgb LifeElement::lifeColor() const
{
    return this->m_lifeColor;
}

int LifeElement::threshold() const
{
    return this->m_threshold;
}

int LifeElement::lumaThreshold() const
{
    return this->m_lumaThreshold;
}

QImage LifeElement::imageDiff(const QImage &img1,
                              const QImage &img2,
                              int threshold,
                              int lumaThreshold)
{
    int width = qMin(img1.width(), img2.width());
    int height = qMin(img1.height(), img2.height());
    QImage diff(width, height, QImage::Format_Indexed8);

    for (int y = 0; y < height; y++) {
        const QRgb *line1 = reinterpret_cast<const QRgb *>(img1.constScanLine(y));
        const QRgb *line2 = reinterpret_cast<const QRgb *>(img2.constScanLine(y));
        quint8 *lineDiff = diff.scanLine(y);

        for (int x = 0; x < width; x++) {
            int r1 = qRed(line1[x]);
            int g1 = qGreen(line1[x]);
            int b1 = qBlue(line1[x]);

            int r2 = qRed(line2[x]);
            int g2 = qGreen(line2[x]);
            int b2 = qBlue(line2[x]);

            int dr = r1 - r2;
            int dg = g1 - g2;
            int db = b1 - b2;

            int colorDiff = dr * dr + dg * dg + db * db;

            lineDiff[x] = sqrt(colorDiff / 3) >= threshold
                          && qGray(line2[x]) >= lumaThreshold? 1: 0;
        }
    }

    return diff;
}

void LifeElement::updateLife()
{
    QImage lifeBuffer(this->m_lifeBuffer.size(), this->m_lifeBuffer.format());
    lifeBuffer.fill(0);

    for (int y = 1; y < lifeBuffer.height() - 1; y++) {
        const quint8 *iLine = this->m_lifeBuffer.constScanLine(y);
        quint8 *oLine = lifeBuffer.scanLine(y);

        for (int x = 1; x < lifeBuffer.width() - 1; x++) {
            int count = 0;

            for (int j = -1; j < 2; j++) {
                const quint8 *line = this->m_lifeBuffer.constScanLine(y + j);

                for (int i = -1; i < 2; i++)
                    count += line[x + i];
            }

            count -= iLine[x];

            if ((iLine[x] && count == 2) || count == 3)
                oLine[x] = 1;
        }
    }

    memcpy(this->m_lifeBuffer.bits(),
           lifeBuffer.constBits(),
           size_t(lifeBuffer.byteCount()));
}

QString LifeElement::controlInterfaceProvide(const QString &controlId) const
{
    Q_UNUSED(controlId)

    return QString("qrc:/Life/share/qml/main.qml");
}

void LifeElement::controlInterfaceConfigure(QQmlContext *context,
                                            const QString &controlId) const
{
    Q_UNUSED(controlId)

    context->setContextProperty("Life", const_cast<QObject *>(qobject_cast<const QObject *>(this)));
    context->setContextProperty("controlId", this->objectName());
}

void LifeElement::setLifeColor(QRgb lifeColor)
{
    if (this->m_lifeColor == lifeColor)
        return;

    this->m_lifeColor = lifeColor;
    emit this->lifeColorChanged(lifeColor);
}

void LifeElement::setThreshold(int threshold)
{
    if (this->m_threshold == threshold)
        return;

    this->m_threshold = threshold;
    emit this->thresholdChanged(threshold);
}

void LifeElement::setLumaThreshold(int lumaThreshold)
{
    if (this->m_lumaThreshold == lumaThreshold)
        return;

    this->m_lumaThreshold = lumaThreshold;
    emit this->lumaThresholdChanged(lumaThreshold);
}

void LifeElement::resetLifeColor()
{
    this->setLifeColor(qRgb(255, 255, 255));
}

void LifeElement::resetThreshold()
{
    this->setThreshold(15);
}

void LifeElement::resetLumaThreshold()
{
    this->setLumaThreshold(15);
}

AkPacket LifeElement::iStream(const AkPacket &packet)
{
    QImage src = AkUtils::packetToImage(packet);

    if (src.isNull())
        return AkPacket();

    src = src.convertToFormat(QImage::Format_ARGB32);
    QImage oFrame = src;

    if (src.size() != this->m_frameSize) {
        this->m_lifeBuffer = QImage();
        this->m_prevFrame = QImage();

        this->m_frameSize = src.size();
    }

    if (this->m_prevFrame.isNull()) {
        this->m_lifeBuffer = QImage(src.size(), QImage::Format_Indexed8);
        this->m_lifeBuffer.setColor(0, 0);
        this->m_lifeBuffer.setColor(1, this->m_lifeColor);
        this->m_lifeBuffer.fill(0);
    }
    else {
        // Compute the difference between previous and current frame,
        // and save it to the buffer.
        QImage diff = this->imageDiff(this->m_prevFrame,
                                      src,
                                      this->m_threshold,
                                      this->m_lumaThreshold);

        this->m_lifeBuffer.setColor(1, this->m_lifeColor);

        for (int y = 0; y < this->m_lifeBuffer.height(); y++) {
            const quint8 *diffLine = diff.constScanLine(y);
            quint8 *lifeBufferLine = this->m_lifeBuffer.scanLine(y);

            for (int x = 0; x < this->m_lifeBuffer.width(); x++)
                lifeBufferLine[x] |= diffLine[x];
        }

        this->updateLife();

        QPainter painter;
        painter.begin(&oFrame);
        painter.drawImage(0, 0, this->m_lifeBuffer);
        painter.end();
    }

    this->m_prevFrame = src.copy();

    AkPacket oPacket = AkUtils::imageToPacket(oFrame, packet);
    akSend(oPacket)
}
