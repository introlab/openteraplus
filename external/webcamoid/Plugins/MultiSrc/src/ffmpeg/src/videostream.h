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

#ifndef VIDEOSTREAM_H
#define VIDEOSTREAM_H

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
}

#ifdef PixelFormat
#undef PixelFormat
#endif

#include "abstractstream.h"

class VideoStream: public AbstractStream
{
    Q_OBJECT

    public:
        explicit VideoStream(const AVFormatContext *formatContext=nullptr,
                             uint index=0, qint64 id=-1,
                             Clock *globalClock=nullptr,
                             bool noModify=false,
                             QObject *parent=nullptr);
        ~VideoStream();

        Q_INVOKABLE AkCaps caps() const;

    protected:
        void processPacket(AVPacket *packet);
        void processData(AVFrame *frame);

    private:
        SwsContext *m_scaleContext;

        // Sync properties.
        qreal m_lastPts;

        AkFrac fps() const;
        AkPacket convert(AVFrame *iFrame);
        int64_t bestEffortTimestamp(const AVFrame *frame) const;
        AVFrame *copyFrame(AVFrame *frame) const;
};

#endif // VIDEOSTREAM_H
