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

#ifndef CONVERTAUDIOGSTREAMER_H
#define CONVERTAUDIOGSTREAMER_H

#include <QtConcurrent>
#include <akaudiopacket.h>
#include <gst/audio/audio.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

#include "convertaudio.h"

class ConvertAudioGStreamer: public ConvertAudio
{
    Q_OBJECT

    public:
        explicit ConvertAudioGStreamer(QObject *parent=nullptr);
        ~ConvertAudioGStreamer();

        Q_INVOKABLE bool init(const AkAudioCaps &caps);
        Q_INVOKABLE AkPacket convert(const AkAudioPacket &packet);
        Q_INVOKABLE void uninit();

    private:
        AkAudioCaps m_caps;
        QThreadPool m_threadPool;
        GstElement *m_pipeline;
        GstElement *m_source;
        GstElement *m_sink;
        GMainLoop *m_mainLoop;
        guint m_busWatchId;
        QMutex m_mutex;

        void waitState(GstState state);
        static gboolean busCallback(GstBus *bus,
                                    GstMessage *message,
                                    gpointer userData);
};

#endif // CONVERTAUDIOGSTREAMER_H
