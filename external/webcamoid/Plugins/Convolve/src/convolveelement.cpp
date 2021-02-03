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

#include "convolveelement.h"

ConvolveElement::ConvolveElement(): AkElement()
{
    this->m_kernel = {
        0, 0, 0,
        0, 1, 0,
        0, 0, 0
    };

    this->m_kernelSize = QSize(3, 3);
    this->m_factor = AkFrac(1, 1);
    this->m_bias = 0;
}

QVariantList ConvolveElement::kernel() const
{
    QVariantList kernel;

    for (const int &e: this->m_kernel)
        kernel << e;

    return kernel;
}

QSize ConvolveElement::kernelSize() const
{
    return this->m_kernelSize;
}

AkFrac ConvolveElement::factor() const
{
    return this->m_factor;
}

int ConvolveElement::bias() const
{
    return this->m_bias;
}

QString ConvolveElement::controlInterfaceProvide(const QString &controlId) const
{
    Q_UNUSED(controlId)

    return QString("qrc:/Convolve/share/qml/main.qml");
}

void ConvolveElement::controlInterfaceConfigure(QQmlContext *context,
                                                const QString &controlId) const
{
    Q_UNUSED(controlId)

    context->setContextProperty("Convolve", const_cast<QObject *>(qobject_cast<const QObject *>(this)));
    context->setContextProperty("controlId", this->objectName());
}

void ConvolveElement::setKernel(const QVariantList &kernel)
{
    QVector<int> k;

    for (const QVariant &e: kernel)
        k << e.toInt();

    if (this->m_kernel == k)
        return;

    QMutexLocker(&this->m_mutex);
    this->m_kernel = k;
    emit this->kernelChanged(kernel);
}

void ConvolveElement::setKernelSize(const QSize &kernelSize)
{
    if (this->m_kernelSize == kernelSize)
        return;

    QMutexLocker(&this->m_mutex);
    this->m_kernelSize = kernelSize;
    emit this->kernelSizeChanged(kernelSize);
}

void ConvolveElement::setFactor(const AkFrac &factor)
{
    if (this->m_factor == factor)
        return;

    QMutexLocker(&this->m_mutex);
    this->m_factor = factor;
    emit this->factorChanged(factor);
}

void ConvolveElement::setBias(int bias)
{
    if (this->m_bias == bias)
        return;

    QMutexLocker(&this->m_mutex);
    this->m_bias = bias;
    emit this->biasChanged(bias);
}

void ConvolveElement::resetKernel()
{
    static const QVariantList kernel = {
        0, 0, 0,
        0, 1, 0,
        0, 0, 0
    };

    this->setKernel(kernel);
}

void ConvolveElement::resetKernelSize()
{
    this->setKernelSize(QSize(3, 3));
}

void ConvolveElement::resetFactor()
{
    this->setFactor(AkFrac(1, 1));
}

void ConvolveElement::resetBias()
{
    this->setBias(0);
}

AkPacket ConvolveElement::iStream(const AkPacket &packet)
{
    QImage src = AkUtils::packetToImage(packet);

    if (src.isNull())
        return AkPacket();

    src = src.convertToFormat(QImage::Format_ARGB32);
    QImage oFrame(src.size(), src.format());

    this->m_mutex.lock();
    QVector<int> kernel = this->m_kernel;
    const int *kernelBits = kernel.constData();
    qint64 factorNum = this->m_factor.num();
    qint64 factorDen = this->m_factor.den();
    int kernelWidth = this->m_kernelSize.width();
    int kernelHeight = this->m_kernelSize.height();
    this->m_mutex.unlock();

    int minI = -(kernelWidth - 1) / 2;
    int maxI = (kernelWidth + 1) / 2;
    int minJ = -(kernelHeight - 1) / 2;
    int maxJ = (kernelHeight + 1) / 2;

    for (int y = 0; y < src.height(); y++) {
        const QRgb *iLine = reinterpret_cast<const QRgb *>(src.constScanLine(y));
        QRgb *oLine = reinterpret_cast<QRgb *>(oFrame.scanLine(y));

        for (int x = 0; x < src.width(); x++) {
            int r = 0;
            int g = 0;
            int b = 0;

            for (int j = minJ, k = 0; j < maxJ; j++) {
                int yp = qBound(0, y + j, src.height() - 1);
                const QRgb *iLine = reinterpret_cast<const QRgb *>(src.constScanLine(yp));

                for (int i = minI; i < maxI; i++, k++) {
                    int xp = qBound(0, x + i, src.width() - 1);

                    if (kernelBits[k]) {
                        r += kernelBits[k] * qRed(iLine[xp]);
                        g += kernelBits[k] * qGreen(iLine[xp]);
                        b += kernelBits[k] * qBlue(iLine[xp]);
                    }
                }
            }

            if (factorNum) {
                r = int(factorNum * r / factorDen + this->m_bias);
                g = int(factorNum * g / factorDen + this->m_bias);
                b = int(factorNum * b / factorDen + this->m_bias);

                r = qBound(0, r, 255);
                g = qBound(0, g, 255);
                b = qBound(0, b, 255);
            } else {
                r = 255;
                g = 255;
                b = 255;
            }

            oLine[x] = qRgba(r, g, b, qAlpha(iLine[x]));
        }
    }

    AkPacket oPacket = AkUtils::imageToPacket(oFrame, packet);
    akSend(oPacket)
}
