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

#include "abstractstream.h"

template <typename T>
inline void waitLoop(const QFuture<T> &loop)
{
    while (!loop.isFinished()) {
        auto eventDispatcher = QThread::currentThread()->eventDispatcher();

        if (eventDispatcher)
            eventDispatcher->processEvents(QEventLoop::AllEvents);
    }
}

AbstractStream::AbstractStream(const AVFormatContext *formatContext,
                               uint index, qint64 id, Clock *globalClock,
                               bool noModify,
                               QObject *parent): QObject(parent)
{
    this->m_runPacketLoop = false;
    this->m_runDataLoop = false;
    this->m_paused = false;
    this->m_isValid = false;
    this->m_clockDiff = 0;
    this->m_maxData = 0;
    this->m_index = index;
    this->m_id = id;

    this->m_stream = (formatContext && index < formatContext->nb_streams)?
                         formatContext->streams[index]: nullptr;

    this->m_mediaType = this->m_stream?
#ifdef HAVE_CODECPAR
                            this->m_stream->codecpar->codec_type:
#else
                            this->m_stream->codec->codec_type:
#endif
                            AVMEDIA_TYPE_UNKNOWN;

    this->m_codecContext = nullptr;

    if (this->m_stream) {
        this->m_codecContext = avcodec_alloc_context3(nullptr);

#ifdef HAVE_CODECPAR
        if (avcodec_parameters_to_context(this->m_codecContext,
                                          this->m_stream->codecpar) < 0)
            avcodec_free_context(&this->m_codecContext);
#else
        if (avcodec_copy_context(this->m_codecContext,
                                 this->m_stream->codec) < 0) {
            avcodec_close(this->m_codecContext);
            av_free(this->m_codecContext);
        }
#endif
    }

    this->m_codec = this->m_codecContext?
                        avcodec_find_decoder(this->m_codecContext->codec_id):
                        nullptr;

    this->m_codecOptions = nullptr;
    this->m_packetQueueSize = 0;
    this->m_globalClock = globalClock;

    if (!this->m_codec)
        return;

    if (this->m_stream)
        this->m_timeBase = AkFrac(this->m_stream->time_base.num,
                                  this->m_stream->time_base.den);

    if (!noModify) {
        if (this->m_stream)
            this->m_stream->discard = AVDISCARD_DEFAULT;

        this->m_codecContext->workaround_bugs = 1;
        this->m_codecContext->idct_algo = FF_IDCT_AUTO;
        this->m_codecContext->error_concealment = FF_EC_GUESS_MVS | FF_EC_DEBLOCK;

        //if (this->m_codec->capabilities & AV_CODEC_CAP_DR1)
        //    this->m_codecContext->flags |= CODEC_FLAG_EMU_EDGE;

        av_dict_set(&this->m_codecOptions, "refcounted_frames", "0", 0);
    }

    this->m_isValid = true;

    if (this->m_threadPool.maxThreadCount() < 2)
        this->m_threadPool.setMaxThreadCount(2);
}

AbstractStream::~AbstractStream()
{
    if (this->m_codecContext) {
#ifdef HAVE_FREECONTEXT
        avcodec_free_context(&this->m_codecContext);
#else
        avcodec_close(this->m_codecContext);
        av_free(this->m_codecContext);
#endif
    }
}

bool AbstractStream::paused() const
{
    return this->m_paused;
}

bool AbstractStream::isValid() const
{
    return this->m_isValid;
}

uint AbstractStream::index() const
{
    return this->m_index;
}

qint64 AbstractStream::id() const
{
    return this->m_id;
}

AkFrac AbstractStream::timeBase() const
{
    return this->m_timeBase;
}

AVMediaType AbstractStream::mediaType() const
{
    return this->m_mediaType;
}

AVStream *AbstractStream::stream() const
{
    return this->m_stream;
}

AVCodecContext *AbstractStream::codecContext() const
{
    return this->m_codecContext;
}

AVCodec *AbstractStream::codec() const
{
    return this->m_codec;
}

AVDictionary *AbstractStream::codecOptions() const
{
    return this->m_codecOptions;
}

AkCaps AbstractStream::caps() const
{
    return AkCaps();
}

void AbstractStream::packetEnqueue(AVPacket *packet)
{
    if (!this->m_runPacketLoop)
        return;

    this->m_packetMutex.lock();

    if (packet) {
        this->m_packets.enqueue(PacketPtr(packet, this->deletePacket));
        this->m_packetQueueSize += packet->size;
    } else
        this->m_packets.enqueue(PacketPtr());

    this->m_packetQueueNotEmpty.wakeAll();
    this->m_packetMutex.unlock();
}

void AbstractStream::dataEnqueue(AVFrame *frame)
{
    this->m_dataMutex.lock();

    if (this->m_frames.size() >= this->m_maxData)
        this->m_dataQueueNotFull.wait(&this->m_dataMutex);

    if (frame)
        this->m_frames.enqueue(FramePtr(frame, this->deleteFrame));
    else
        this->m_frames.enqueue(FramePtr());

    this->m_dataQueueNotEmpty.wakeAll();
    this->m_dataMutex.unlock();
}

void AbstractStream::subtitleEnqueue(AVSubtitle *subtitle)
{
    this->m_dataMutex.lock();

    if (this->m_subtitles.size() >= this->m_maxData)
        this->m_dataQueueNotFull.wait(&this->m_dataMutex);

    if (subtitle)
        this->m_subtitles.enqueue(SubtitlePtr(subtitle, this->deleteSubtitle));
    else
        this->m_subtitles.enqueue(SubtitlePtr());

    this->m_dataQueueNotEmpty.wakeAll();
    this->m_dataMutex.unlock();
}

qint64 AbstractStream::queueSize()
{
    return this->m_packetQueueSize;
}

Clock *AbstractStream::globalClock()
{
    return this->m_globalClock;
}

qreal AbstractStream::clockDiff() const
{
    return this->m_clockDiff;
}

AVMediaType AbstractStream::type(const AVFormatContext *formatContext,
                                 uint index)
{
    return index < formatContext->nb_streams?
#ifdef HAVE_CODECPAR
                formatContext->streams[index]->codecpar->codec_type:
#else
                formatContext->streams[index]->codec->codec_type:
#endif
                AVMEDIA_TYPE_UNKNOWN;
}

void AbstractStream::processPacket(AVPacket *packet)
{
    Q_UNUSED(packet)
}

void AbstractStream::processData(AVFrame *frame)
{
    Q_UNUSED(frame)
}

void AbstractStream::processData(AVSubtitle *subtitle)
{
    Q_UNUSED(subtitle)
}

void AbstractStream::packetLoop()
{
    while (this->m_runPacketLoop) {
        this->m_packetMutex.lock();
        bool gotPacket = true;

        if (this->m_packets.isEmpty())
            gotPacket = this->m_packetQueueNotEmpty.wait(&this->m_packetMutex,
                                                         THREAD_WAIT_LIMIT);

        PacketPtr packet;

        if (gotPacket) {
            packet = this->m_packets.dequeue();

            if (packet)
                this->m_packetQueueSize -= packet->size;
        }

        this->m_packetMutex.unlock();

        if (gotPacket) {
            this->processPacket(packet.data());
            emit this->notify();
        }

        if (!packet)
            this->m_runPacketLoop = false;
    }
}

void AbstractStream::dataLoop()
{
    switch (this->mediaType()) {
    case AVMEDIA_TYPE_VIDEO:
    case AVMEDIA_TYPE_AUDIO:
        while (this->m_runDataLoop) {
            this->m_dataMutex.lock();
            bool gotFrame = true;

            if (this->m_frames.isEmpty())
                gotFrame = this->m_dataQueueNotEmpty.wait(&this->m_dataMutex,
                                                          THREAD_WAIT_LIMIT);

            FramePtr frame;

            if (gotFrame) {
                frame = this->m_frames.dequeue();

                if (this->m_frames.size() < this->m_maxData)
                    this->m_dataQueueNotFull.wakeAll();
            }

            this->m_dataMutex.unlock();

            if (gotFrame) {
                if (frame)
                    this->processData(frame.data());
                else {
                    emit this->eof();
                    this->m_runDataLoop = false;
                }
            }else{
                //qDebug() << "NO FRAME!";
                emit this->eof();
                m_runDataLoop=false;
            }
        }

        break;
    case AVMEDIA_TYPE_SUBTITLE:
        while (this->m_runDataLoop) {
            this->m_dataMutex.lock();
            bool gotSubtitle = true;

            if (this->m_subtitles.isEmpty())
                gotSubtitle = this->m_dataQueueNotEmpty.wait(&this->m_dataMutex,
                                                             THREAD_WAIT_LIMIT);

            SubtitlePtr subtitle;

            if (gotSubtitle) {
                subtitle = this->m_subtitles.dequeue();

                if (this->m_subtitles.size() < this->m_maxData)
                    this->m_dataQueueNotFull.wakeAll();
            }

            this->m_dataMutex.unlock();

            if (gotSubtitle) {
                if (subtitle)
                    this->processData(subtitle.data());
                else {
                    emit this->eof();
                    this->m_runDataLoop = false;
                }
            }
        }

        break;
    default:
        break;
    }
}

void AbstractStream::deletePacket(AVPacket *packet)
{
#ifdef HAVE_PACKETREF
    av_packet_unref(packet);
#else
    av_destruct_packet(packet);
#endif
    delete packet;
}

void AbstractStream::deleteFrame(AVFrame *frame)
{
    av_freep(&frame->data[0]);
    frame->data[0] = nullptr;

#ifdef HAVE_FRAMEALLOC
    av_frame_unref(frame);
    av_frame_free(&frame);
#else
    avcodec_free_frame(&frame);
#endif
}

void AbstractStream::deleteSubtitle(AVSubtitle *subtitle)
{
    avsubtitle_free(subtitle);
    delete subtitle;
}

void AbstractStream::setPaused(bool paused)
{
    if (this->m_paused == paused)
        return;

    this->m_runDataLoop = !paused;

    if (paused)
        this->m_dataLoopResult.waitForFinished();
    else
        this->m_dataLoopResult =
            QtConcurrent::run(&this->m_threadPool, [=]{AbstractStream::dataLoop();});

    this->m_paused = paused;
    emit this->pausedChanged(paused);
}

void AbstractStream::resetPaused()
{
    this->setPaused(false);
}

bool AbstractStream::init()
{
    if (!this->m_codecContext
        || !this->m_codec)
        return false;

    if (avcodec_open2(this->m_codecContext,
                      this->m_codec,
                      &this->m_codecOptions) < 0)
        return false;

    this->m_clockDiff = 0;
    this->m_runPacketLoop = true;
    this->m_runDataLoop = true;
    this->m_packetLoopResult =
        QtConcurrent::run(&m_threadPool,[=]{AbstractStream::packetLoop();});
    this->m_dataLoopResult =
            QtConcurrent::run(&m_threadPool, [=]{AbstractStream::dataLoop();});

    return true;
}

void AbstractStream::uninit()
{
    this->m_runPacketLoop = false;
    waitLoop(this->m_packetLoopResult);

    this->m_runDataLoop = false;
    waitLoop(this->m_dataLoopResult);

    if (this->m_codecOptions)
        av_dict_free(&this->m_codecOptions);

    if (this->m_codecContext) {
        avcodec_close(this->m_codecContext);
        this->m_codecContext = nullptr;
    }

    this->m_packets.clear();
    this->m_frames.clear();
    this->m_subtitles.clear();
}
