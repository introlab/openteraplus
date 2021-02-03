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

#ifndef CONVOLVEELEMENT_H
#define CONVOLVEELEMENT_H

#include <QMutex>
#include <ak.h>
#include <akutils.h>

class ConvolveElement: public AkElement
{
    Q_OBJECT
    Q_PROPERTY(QVariantList kernel
               READ kernel
               WRITE setKernel
               RESET resetKernel
               NOTIFY kernelChanged)
    Q_PROPERTY(QSize kernelSize
               READ kernelSize
               WRITE setKernelSize
               RESET resetKernelSize
               NOTIFY kernelSizeChanged)
    Q_PROPERTY(AkFrac factor
               READ factor
               WRITE setFactor
               RESET resetFactor
               NOTIFY factorChanged)
    Q_PROPERTY(int bias
               READ bias
               WRITE setBias
               RESET resetBias
               NOTIFY biasChanged)

    public:
        explicit ConvolveElement();

        Q_INVOKABLE QVariantList kernel() const;
        Q_INVOKABLE QSize kernelSize() const;
        Q_INVOKABLE AkFrac factor() const;
        Q_INVOKABLE int bias() const;

    private:
        QVector<int> m_kernel;
        QSize m_kernelSize;
        AkFrac m_factor;
        int m_bias;

        QMutex m_mutex;

    protected:
        QString controlInterfaceProvide(const QString &controlId) const;
        void controlInterfaceConfigure(QQmlContext *context,
                                       const QString &controlId) const;

    signals:
        void kernelChanged(const QVariantList &kernel);
        void kernelSizeChanged(const QSize &kernelSize);
        void factorChanged(const AkFrac &factor);
        void biasChanged(int bias);

    public slots:
        void setKernel(const QVariantList &kernel);
        void setKernelSize(const QSize &kernelSize);
        void setFactor(const AkFrac &factor);
        void setBias(int bias);
        void resetKernel();
        void resetKernelSize();
        void resetFactor();
        void resetBias();
        AkPacket iStream(const AkPacket &packet);
};

#endif // CONVOLVEELEMENT_H
