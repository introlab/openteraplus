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

#ifndef DICEELEMENT_H
#define DICEELEMENT_H

#include <QMutex>
#include <ak.h>
#include <akutils.h>

class DiceElement: public AkElement
{
    Q_OBJECT
    Q_PROPERTY(int diceSize
               READ diceSize
               WRITE setDiceSize
               RESET resetDiceSize
               NOTIFY diceSizeChanged)

    public:
        explicit DiceElement();

        Q_INVOKABLE int diceSize() const;

    private:
        int m_diceSize;

        QMutex m_mutex;
        QImage m_diceMap;
        QSize m_frameSize;

    protected:
        QString controlInterfaceProvide(const QString &controlId) const;
        void controlInterfaceConfigure(QQmlContext *context,
                                       const QString &controlId) const;

    signals:
        void diceSizeChanged(int diceSize);
        void frameSizeChanged(const QSize &frameSize);

    public slots:
        void setDiceSize(int diceSize);
        void resetDiceSize();
        AkPacket iStream(const AkPacket &packet);

    private slots:
        void updateDiceMap();
};

#endif // DICEELEMENT_H
