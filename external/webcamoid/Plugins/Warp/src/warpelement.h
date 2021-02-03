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

#ifndef WARPELEMENT_H
#define WARPELEMENT_H

#include <ak.h>
#include <akutils.h>

class WarpElement: public AkElement
{
    Q_OBJECT
    Q_PROPERTY(qreal ripples
               READ ripples
               WRITE setRipples
               RESET resetRipples
               NOTIFY ripplesChanged)

    public:
        explicit WarpElement();

        qreal ripples() const;

    private:
        qreal m_ripples;

        QSize m_frameSize;
        QVector<qreal> m_phiTable;

    protected:
        QString controlInterfaceProvide(const QString &controlId) const;
        void controlInterfaceConfigure(QQmlContext *context,
                                       const QString &controlId) const;

    signals:
        void ripplesChanged(qreal ripples);
        void frameSizeChanged(const QSize &frameSize);

    public slots:
        void setRipples(qreal ripples);
        void resetRipples();

        AkPacket iStream(const AkPacket &packet);
};

#endif // WARPELEMENT_H
