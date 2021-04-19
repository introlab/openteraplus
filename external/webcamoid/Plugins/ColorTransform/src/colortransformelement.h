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

#ifndef COLORTRANSFORMELEMENT_H
#define COLORTRANSFORMELEMENT_H

#include <ak.h>
#include <akutils.h>

class ColorTransformElement: public AkElement
{
    Q_OBJECT
    Q_PROPERTY(QVariantList kernel
               READ kernel
               WRITE setKernel
               RESET resetKernel
               NOTIFY kernelChanged)

    public:
        explicit ColorTransformElement();

        Q_INVOKABLE QVariantList kernel() const;

    private:
        QVector<qreal> m_kernel;

    protected:
        QString controlInterfaceProvide(const QString &controlId) const;
        void controlInterfaceConfigure(QQmlContext *context,
                                       const QString &controlId) const;

    signals:
        void kernelChanged(const QVariantList &kernel);

    public slots:
        void setKernel(const QVariantList &kernel);
        void resetKernel();
        AkPacket iStream(const AkPacket &packet);
};

#endif // COLORTRANSFORMELEMENT_H
