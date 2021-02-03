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

#ifndef QUARKELEMENT_H
#define QUARKELEMENT_H

#include <ak.h>
#include <akutils.h>

class QuarkElement: public AkElement
{
    Q_OBJECT
    Q_PROPERTY(int nFrames
               READ nFrames
               WRITE setNFrames
               RESET resetNFrames
               NOTIFY nFramesChanged)

    public:
        explicit QuarkElement();

        Q_INVOKABLE int nFrames() const;

    private:
        int m_nFrames;

        QVector<QImage> m_frames;
        QSize m_frameSize;

    protected:
        QString controlInterfaceProvide(const QString &controlId) const;
        void controlInterfaceConfigure(QQmlContext *context,
                                       const QString &controlId) const;

    signals:
        void nFramesChanged(int nFrames);

    public slots:
        void setNFrames(int nFrames);
        void resetNFrames();

        AkPacket iStream(const AkPacket &packet);
};

#endif // QUARKELEMENT_H
