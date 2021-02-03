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

#include "distortelement.h"

DistortElement::DistortElement(): AkElement()
{
    this->m_amplitude = 1.0;
    this->m_frequency = 1.0;
    this->m_gridSizeLog = 1;
}

qreal DistortElement::amplitude() const
{
    return this->m_amplitude;
}

qreal DistortElement::frequency() const
{
    return this->m_frequency;
}

int DistortElement::gridSizeLog() const
{
    return this->m_gridSizeLog;
}

QVector<QPoint> DistortElement::createGrid(int width, int height,
                                           int gridSize, qreal time)
{
    QVector<QPoint> grid;

    for (int y = 0; y <= height; y += gridSize)
        for (int x = 0; x <= width; x += gridSize)
            grid << this->plasmaFunction(QPoint(x, y), QSize(width, height),
                                         this->m_amplitude, this->m_frequency,
                                         time);

    return grid;
}

QString DistortElement::controlInterfaceProvide(const QString &controlId) const
{
    Q_UNUSED(controlId)

    return QString("qrc:/Distort/share/qml/main.qml");
}

void DistortElement::controlInterfaceConfigure(QQmlContext *context,
                                               const QString &controlId) const
{
    Q_UNUSED(controlId)

    context->setContextProperty("Distort", const_cast<QObject *>(qobject_cast<const QObject *>(this)));
    context->setContextProperty("controlId", this->objectName());
}

void DistortElement::setAmplitude(qreal amplitude)
{
    if (qFuzzyCompare(this->m_amplitude, amplitude))
        return;

    this->m_amplitude = amplitude;
    emit this->amplitudeChanged(amplitude);
}

void DistortElement::setFrequency(qreal frequency)
{
    if (qFuzzyCompare(this->m_frequency, frequency))
        return;

    this->m_frequency = frequency;
    emit this->frequencyChanged(frequency);
}

void DistortElement::setGridSizeLog(int gridSizeLog)
{
    if (this->m_gridSizeLog == gridSizeLog)
        return;

    this->m_gridSizeLog = gridSizeLog;
    emit this->gridSizeLogChanged(gridSizeLog);
}

void DistortElement::resetAmplitude()
{
    this->setAmplitude(1.0);
}

void DistortElement::resetFrequency()
{
    this->setFrequency(1.0);
}

void DistortElement::resetGridSizeLog()
{
    this->setGridSizeLog(1);
}

AkPacket DistortElement::iStream(const AkPacket &packet)
{
    QImage src = AkUtils::packetToImage(packet);

    if (src.isNull())
        return AkPacket();

    src = src.convertToFormat(QImage::Format_ARGB32);
    QImage oFrame = QImage(src.size(), src.format());

    const QRgb *srcBits = reinterpret_cast<const QRgb *>(src.constBits());
    QRgb *destBits = reinterpret_cast<QRgb *>(oFrame.bits());

    int gridSizeLog = this->m_gridSizeLog > 0? this->m_gridSizeLog: 1;
    int gridSize = 1 << gridSizeLog;
    qreal time = packet.pts() * packet.timeBase().value();
    QVector<QPoint> grid = this->createGrid(src.width(), src.height(), gridSize, time);

    int gridX = src.width() / gridSize;
    int gridY = src.height() / gridSize;

    for (int y = 0; y < gridY; y++)
        for (int x = 0; x < gridX; x++) {
            int offset = x + y * (gridX + 1);

            QPoint upperLeft  = grid[offset];
            QPoint lowerLeft  = grid[offset + gridX + 1];
            QPoint upperRight = grid[offset + 1];
            QPoint lowerRight = grid[offset + gridX + 2];

            int startColXX = upperLeft.x();
            int startColYY = upperLeft.y();
            int endColXX = upperRight.x();
            int endColYY = upperRight.y();

            int stepStartColX = (lowerLeft.x() - upperLeft.x())
                                >> gridSizeLog;

            int stepStartColY = (lowerLeft.y() - upperLeft.y())
                                >> gridSizeLog;

            int stepEndColX = (lowerRight.x() - upperRight.x())
                              >> gridSizeLog;

            int stepEndColY = (lowerRight.y() - upperRight.y())
                              >> gridSizeLog;

            int pos = (y << gridSizeLog) * src.width() + (x << gridSizeLog);

            for (int blockY = 0; blockY < gridSize; blockY++) {
                int xLineIndex = startColXX;
                int yLineIndex = startColYY;

                int stepLineX = (endColXX - startColXX) >> gridSizeLog;
                int stepLineY = (endColYY - startColYY) >> gridSizeLog;

                for (int i = 0, blockX = 0; blockX < gridSize; i++, blockX++) {
                    int xx = qBound(0, xLineIndex, src.width() - 1);
                    int yy = qBound(0, yLineIndex, src.height() - 1);

                    xLineIndex += stepLineX;
                    yLineIndex += stepLineY;

                    destBits[pos + i] = srcBits[xx + yy * src.width()];
                }

                startColXX += stepStartColX;
                endColXX   += stepEndColX;
                startColYY += stepStartColY;
                endColYY   += stepEndColY;

                pos += src.width() - gridSize + gridSize;
            }
        }

    AkPacket oPacket = AkUtils::imageToPacket(oFrame, packet);
    akSend(oPacket)
}
