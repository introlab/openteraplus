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

#ifndef COLORFILTERELEMENT_H
#define COLORFILTERELEMENT_H

#include <ak.h>
#include <akutils.h>

class ColorFilterElement: public AkElement
{
    Q_OBJECT
    Q_PROPERTY(QRgb colorf
               READ color
               WRITE setColor
               RESET resetColor
               NOTIFY colorChanged)
    Q_PROPERTY(qreal radius
               READ radius
               WRITE setRadius
               RESET resetRadius
               NOTIFY radiusChanged)
    Q_PROPERTY(bool soft
               READ soft
               WRITE setSoft
               RESET resetSoft
               NOTIFY softChanged)
    Q_PROPERTY(bool disable
               READ disable
               WRITE setDisable
               RESET resetDisable
               NOTIFY disableChanged)

    public:
        explicit ColorFilterElement();

        Q_INVOKABLE QRgb color() const;
        Q_INVOKABLE qreal radius() const;
        Q_INVOKABLE bool soft() const;
        Q_INVOKABLE bool disable() const;

    private:
        QRgb m_color;
        qreal m_radius;
        bool m_soft;
        bool m_disable;

    protected:
        QString controlInterfaceProvide(const QString &controlId) const;
        void controlInterfaceConfigure(QQmlContext *context,
                                       const QString &controlId) const;

    signals:
        void colorChanged(QRgb color);
        void radiusChanged(qreal radius);
        void softChanged(bool soft);
        void disableChanged(bool disable);

    public slots:
        void setColor(QRgb color);
        void setRadius(qreal radius);
        void setSoft(bool soft);
        void setDisable(bool disable);
        void resetColor();
        void resetRadius();
        void resetSoft();
        void resetDisable();
        AkPacket iStream(const AkPacket &packet);
};

#endif // COLORFILTERELEMENT_H
