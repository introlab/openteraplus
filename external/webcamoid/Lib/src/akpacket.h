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

#ifndef AKPACKET_H
#define AKPACKET_H

#include <QSharedPointer>

#include "akcaps.h"
#include "akfrac.h"

class AkPacketPrivate;

template<typename T>
inline T AkNoPts()
{
    return T(0x1) << (sizeof(T) - 1);
}

class AKCOMMONS_EXPORT AkPacket: public QObject
{
    Q_OBJECT
    Q_PROPERTY(AkCaps caps
               READ caps
               WRITE setCaps
               RESET resetCaps
               NOTIFY capsChanged)
    Q_PROPERTY(QVariant data
               READ data
               WRITE setData
               RESET resetData
               NOTIFY dataChanged)
    Q_PROPERTY(QByteArray buffer
               READ buffer
               WRITE setBuffer
               RESET resetBuffer
               NOTIFY bufferChanged)
    Q_PROPERTY(qint64 id
               READ id
               WRITE setId
               RESET resetId
               NOTIFY idChanged)
    Q_PROPERTY(qint64 pts
               READ pts
               WRITE setPts
               RESET resetPts
               NOTIFY ptsChanged)
    Q_PROPERTY(AkFrac timeBase
               READ timeBase
               WRITE setTimeBase
               RESET resetTimeBase
               NOTIFY timeBaseChanged)
    Q_PROPERTY(int index
               READ index
               WRITE setIndex
               RESET resetIndex
               NOTIFY indexChanged)

    public:
        explicit AkPacket(QObject *parent=nullptr);
        AkPacket(const AkCaps &caps,
                 const QByteArray &buffer=QByteArray(),
                 qint64 pts=0,
                 const AkFrac &timeBase=AkFrac(),
                 int index=-1,
                 qint64 id=-1);
        AkPacket(const AkPacket &other);
        virtual ~AkPacket();
        AkPacket &operator =(const AkPacket &other);
        operator bool() const;

        Q_INVOKABLE QString toString() const;
        Q_INVOKABLE AkCaps caps() const;
        Q_INVOKABLE AkCaps &caps();
        Q_INVOKABLE QVariant data() const;
        Q_INVOKABLE QVariant &data();
        Q_INVOKABLE QByteArray buffer() const;
        Q_INVOKABLE QByteArray &buffer();
        Q_INVOKABLE qint64 id() const;
        Q_INVOKABLE qint64 &id();
        Q_INVOKABLE qint64 pts() const;
        Q_INVOKABLE qint64 &pts();
        Q_INVOKABLE AkFrac timeBase() const;
        Q_INVOKABLE AkFrac &timeBase();
        Q_INVOKABLE int index() const;
        Q_INVOKABLE int &index();

    private:
        AkPacketPrivate *d;

    Q_SIGNALS:
        void capsChanged(const AkCaps &caps);
        void dataChanged(const QVariant &data);
        void bufferChanged(const QByteArray &buffer);
        void idChanged(qint64 id);
        void ptsChanged(qint64 pts);
        void timeBaseChanged(const AkFrac &timeBase);
        void indexChanged(int index);

    public Q_SLOTS:
        void setCaps(const AkCaps &caps);
        void setData(const QVariant &data);
        void setBuffer(const QByteArray &buffer);
        void setId(qint64 id);
        void setPts(qint64 pts);
        void setTimeBase(const AkFrac &timeBase);
        void setIndex(int index);
        void resetCaps();
        void resetData();
        void resetBuffer();
        void resetId();
        void resetPts();
        void resetTimeBase();
        void resetIndex();

    friend QDebug operator <<(QDebug debug, const AkPacket &packet);
};

QDebug operator <<(QDebug debug, const AkPacket &packet);

Q_DECLARE_METATYPE(AkPacket)

#endif // AKPACKET_H
