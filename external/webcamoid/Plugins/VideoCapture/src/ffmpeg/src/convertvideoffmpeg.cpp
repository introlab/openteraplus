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

#include <QMetaEnum>

#include "convertvideoffmpeg.h"

#define THREAD_WAIT_LIMIT 500

// no AV sync correction is done if below the minimum AV sync threshold
#define AV_SYNC_THRESHOLD_MIN 0.01

// AV sync correction is done if above the maximum AV sync threshold
#define AV_SYNC_THRESHOLD_MAX 0.1

// If a frame duration is longer than this, it will not be duplicated to compensate AV sync
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1

// no AV correction is done if too big error
#define AV_NOSYNC_THRESHOLD 10.0

typedef QMap<QString, AVPixelFormat> V4l2PixFmtMap;

inline V4l2PixFmtMap initV4l2PixFmtMap()
{
    V4l2PixFmtMap rawToFF = {
        // RGB formats
        {"RGB1", AV_PIX_FMT_RGB8    },
        {"R444", AV_PIX_FMT_RGB444LE},
        {"RGBO", AV_PIX_FMT_RGB555LE},
        {"RGBP", AV_PIX_FMT_RGB565LE},
        {"RGBQ", AV_PIX_FMT_RGB555BE},
        {"RGBR", AV_PIX_FMT_RGB565BE},
        {"BGR3", AV_PIX_FMT_BGR24   },
        {"RGB3", AV_PIX_FMT_RGB24   },
#ifdef HAVE_EXTRAPIXFORMATS
        {"BGR4", AV_PIX_FMT_RGB0    },
        {"RGB4", AV_PIX_FMT_BGR0    },
#endif
        {"ARGB", AV_PIX_FMT_ARGB    },
        {"RGBA", AV_PIX_FMT_RGBA    },

        // Grey formats
        {"Y800", AV_PIX_FMT_GRAY8    },
        {"GREY", AV_PIX_FMT_GRAY8    },
        {"Y16 ", AV_PIX_FMT_GRAY16LE },
        {"B1W0", AV_PIX_FMT_MONOWHITE},
        {"B0W1", AV_PIX_FMT_MONOBLACK},

        // Palette formats
        {"PAL8", AV_PIX_FMT_PAL8},

        // Luminance+Chrominance formats
        {"YVU9", AV_PIX_FMT_YUV410P},
        {"YV12", AV_PIX_FMT_YUV420P},
        {"I420", AV_PIX_FMT_YUV420P},
        {"YUYV", AV_PIX_FMT_YUYV422},
        {"YYUV", AV_PIX_FMT_YUV422P},
        {"Y42B", AV_PIX_FMT_YUV422P},
        {"UYVY", AV_PIX_FMT_UYVY422},
        {"VYUY", AV_PIX_FMT_YUV422P},
        {"422P", AV_PIX_FMT_YUV422P},
        {"411P", AV_PIX_FMT_YUV411P},
        {"Y41P", AV_PIX_FMT_YUV411P},
        {"YUY2", AV_PIX_FMT_YUYV422},
        {"Y444", AV_PIX_FMT_YUV444P},
        {"444P", AV_PIX_FMT_YUV444P},
        {"YUV9", AV_PIX_FMT_YUV410P},
        {"YU12", AV_PIX_FMT_YUV420P},

        // two planes -- one Y, one Cr + Cb interleaved
        {"NV12", AV_PIX_FMT_NV12},
        {"NV21", AV_PIX_FMT_NV21},
#ifdef HAVE_EXTRAPIXFORMATS
        {"NV16", AV_PIX_FMT_NV16},

        // Bayer formats
        {"BA81", AV_PIX_FMT_BAYER_BGGR8},
        {"GBRG", AV_PIX_FMT_BAYER_GBRG8},
        {"GRBG", AV_PIX_FMT_BAYER_GRBG8},
        {"RGGB", AV_PIX_FMT_BAYER_RGGB8},

        // 10bit raw bayer, expanded to 16 bits
        {"BYR2", AV_PIX_FMT_BAYER_BGGR16LE}
#endif
    };

    return rawToFF;
}

Q_GLOBAL_STATIC_WITH_ARGS(V4l2PixFmtMap, rawToFF, (initV4l2PixFmtMap()))

typedef QMap<QString, AVCodecID> V4l2CodecMap;

inline V4l2CodecMap initCompressedMap()
{
    V4l2CodecMap compressedToFF = {
        // compressed formats
        {"MJPG", AV_CODEC_ID_MJPEG     },
        {"JPEG", AV_CODEC_ID_MJPEG     },
        {"dvsd", AV_CODEC_ID_DVVIDEO   },
        {"H264", AV_CODEC_ID_H264      },
        {"AVC1", AV_CODEC_ID_H264      },
        {"M264", AV_CODEC_ID_H264      },
        {"H263", AV_CODEC_ID_H263      },
        {"MPG1", AV_CODEC_ID_MPEG1VIDEO},
        {"MPG2", AV_CODEC_ID_MPEG2VIDEO},
        {"MPG4", AV_CODEC_ID_MPEG4     },
        {"XVID", AV_CODEC_ID_MPEG4     },
        {"VC1G", AV_CODEC_ID_VC1       },
        {"VC1L", AV_CODEC_ID_VC1       },
        {"VP80", AV_CODEC_ID_VP8       },

#ifdef HAVE_EXTRACODECFORMATS
        //  Vendor-specific formats
        {"CPIA", AV_CODEC_ID_CPIA}
#endif
    };

    return compressedToFF;
}

Q_GLOBAL_STATIC_WITH_ARGS(V4l2CodecMap, compressedToFF, (initCompressedMap()))

ConvertVideoFFmpeg::ConvertVideoFFmpeg(QObject *parent):
    ConvertVideo(parent)
{
    avcodec_register_all();

    this->m_scaleContext = nullptr;
    this->m_codecOptions = nullptr;
    this->m_codecContext = nullptr;
    this->m_packetQueueSize = 0;
    this->m_maxPacketQueueSize = 15 * 1024 * 1024;
    this->m_maxData = 3;
    this->m_id = -1;
    this->m_lastPts = 0;
    this->m_showLog = false;

#ifndef QT_DEBUG
    av_log_set_level(AV_LOG_QUIET);
#endif

    if (this->m_threadPool.maxThreadCount() < 2)
        this->m_threadPool.setMaxThreadCount(2);
}

ConvertVideoFFmpeg::~ConvertVideoFFmpeg()
{
    this->uninit();
}

qint64 ConvertVideoFFmpeg::maxPacketQueueSize() const
{
    return this->m_maxPacketQueueSize;
}

bool ConvertVideoFFmpeg::showLog() const
{
    return this->m_showLog;
}

void ConvertVideoFFmpeg::packetEnqueue(const AkPacket &packet)
{
    this->m_packetMutex.lock();

    if (this->m_packetQueueSize >= this->m_maxPacketQueueSize)
        this->m_packetQueueNotFull.wait(&this->m_packetMutex);

    this->m_packets.enqueue(packet);
    this->m_packetQueueSize += packet.buffer().size();
    this->m_packetQueueNotEmpty.wakeAll();
    this->m_packetMutex.unlock();
}

void ConvertVideoFFmpeg::dataEnqueue(AVFrame *frame)
{
    this->m_dataMutex.lock();

    if (this->m_frames.size() >= this->m_maxData)
        this->m_dataQueueNotFull.wait(&this->m_dataMutex);

    this->m_frames.enqueue(FramePtr(frame, this->deleteFrame));
    this->m_dataQueueNotEmpty.wakeAll();
    this->m_dataMutex.unlock();
}

bool ConvertVideoFFmpeg::init(const AkCaps &caps)
{
    QString fourcc = caps.property("fourcc").toString();

    if (!rawToFF->contains(fourcc)
        && !compressedToFF->contains(fourcc))
        return false;

    AVCodec *codec = avcodec_find_decoder(compressedToFF->value(fourcc, AV_CODEC_ID_RAWVIDEO));

    if (!codec)
        return false;

    this->m_codecContext = avcodec_alloc_context3(codec);

    if (!this->m_codecContext)
        return false;

    if (codec->capabilities & CODEC_CAP_TRUNCATED)
        this->m_codecContext->flags |= CODEC_FLAG_TRUNCATED;

    if (codec->capabilities & CODEC_CAP_DR1)
        this->m_codecContext->flags |= CODEC_FLAG_EMU_EDGE;

    this->m_codecContext->pix_fmt = rawToFF->value(fourcc, AV_PIX_FMT_NONE);
    this->m_codecContext->width = caps.property("width").toInt();
    this->m_codecContext->height = caps.property("height").toInt();
    this->m_fps = caps.property("fps").toString();
#ifdef HAVE_CONTEXTFRAMERATE
    this->m_codecContext->framerate.num = int(this->m_fps.num());
    this->m_codecContext->framerate.den = int(this->m_fps.den());
#else
    this->m_codecContext->time_base.num = int(this->m_fps.den());
    this->m_codecContext->time_base.den = int(this->m_fps.num());
#endif
    this->m_codecContext->workaround_bugs = 1;
    this->m_codecContext->idct_algo = FF_IDCT_AUTO;
    this->m_codecContext->error_concealment = FF_EC_GUESS_MVS | FF_EC_DEBLOCK;

    this->m_codecOptions = nullptr;
    av_dict_set(&this->m_codecOptions, "refcounted_frames", "0", 0);

    if (avcodec_open2(this->m_codecContext, codec, &this->m_codecOptions) < 0) {
#ifdef HAVE_FREECONTEXT
        avcodec_free_context(&this->m_codecContext);
#else
        avcodec_close(this->m_codecContext);
        av_free(this->m_codecContext);
        this->m_codecContext = nullptr;
#endif

        return false;
    }

    this->m_packets.clear();
    this->m_frames.clear();
    this->m_lastPts = 0;
    this->m_id = Ak::id();
    this->m_packetQueueSize = 0;
    this->m_runPacketLoop = true;
    this->m_runDataLoop = true;
    this->m_globalClock.setClock(0.);
    this->m_packetLoopResult = QtConcurrent::run(&this->m_threadPool, this->packetLoop, this);
    this->m_dataLoopResult = QtConcurrent::run(&this->m_threadPool, this->dataLoop, this);

    return true;
}

void ConvertVideoFFmpeg::uninit()
{
    this->m_runPacketLoop = false;
    this->m_packetLoopResult.waitForFinished();

    this->m_runDataLoop = false;
    this->m_dataLoopResult.waitForFinished();

    this->m_packets.clear();
    this->m_frames.clear();

    if (this->m_scaleContext) {
        sws_freeContext(this->m_scaleContext);
        this->m_scaleContext = nullptr;
    }

    if (this->m_codecOptions)
        av_dict_free(&this->m_codecOptions);

    if (this->m_codecContext) {
#ifdef HAVE_FREECONTEXT
        avcodec_free_context(&this->m_codecContext);
#else
        avcodec_close(this->m_codecContext);
        av_free(this->m_codecContext);
        this->m_codecContext = nullptr;
#endif
    }
}

void ConvertVideoFFmpeg::packetLoop(ConvertVideoFFmpeg *stream)
{
    while (stream->m_runPacketLoop) {
        stream->m_packetMutex.lock();

        if (stream->m_packets.isEmpty())
            stream->m_packetQueueNotEmpty.wait(&stream->m_packetMutex,
                                               THREAD_WAIT_LIMIT);

        if (!stream->m_packets.isEmpty()) {
            AkPacket packet = stream->m_packets.dequeue();

            AVPacket videoPacket;
            av_init_packet(&videoPacket);
            videoPacket.data = reinterpret_cast<uint8_t *>(packet.buffer().data());
            videoPacket.size = packet.buffer().size();
            videoPacket.pts = packet.pts();

#ifdef HAVE_SENDRECV
            if (avcodec_send_packet(stream->m_codecContext, &videoPacket) >= 0)
                forever {
    #ifdef HAVE_FRAMEALLOC
                    auto iFrame = av_frame_alloc();
    #else
                    auto iFrame = avcodec_alloc_frame();
    #endif
                    int r = avcodec_receive_frame(stream->m_codecContext, iFrame);

                    if (r >= 0) {
                        iFrame->pts = stream->bestEffortTimestamp(iFrame);
                        stream->dataEnqueue(stream->copyFrame(iFrame));
                    }
    #ifdef HAVE_FRAMEALLOC
                    av_frame_free(&iFrame);
    #else
                    avcodec_free_frame(&iFrame);
    #endif

                    if (r < 0)
                        break;
                }
#else
    #ifdef HAVE_FRAMEALLOC
                auto iFrame = av_frame_alloc();
    #else
                auto iFrame = avcodec_alloc_frame();
    #endif
            int gotFrame;
            avcodec_decode_video2(stream->m_codecContext, iFrame, &gotFrame, &videoPacket);

            if (gotFrame) {
                iFrame->pts = stream->bestEffortTimestamp(iFrame);
                stream->dataEnqueue(stream->copyFrame(iFrame));
            }
    #ifdef HAVE_FRAMEALLOC
            av_frame_free(&iFrame);
    #else
            avcodec_free_frame(&iFrame);
    #endif
#endif

            stream->m_packetQueueSize -= packet.buffer().size();

            if (stream->m_packetQueueSize < stream->m_maxPacketQueueSize)
                stream->m_packetQueueNotFull.wakeAll();
        }

        stream->m_packetMutex.unlock();
    }
}

void ConvertVideoFFmpeg::dataLoop(ConvertVideoFFmpeg *stream)
{
    while (stream->m_runDataLoop) {
        stream->m_dataMutex.lock();

        if (stream->m_frames.isEmpty())
            stream->m_dataQueueNotEmpty.wait(&stream->m_dataMutex,
                                             THREAD_WAIT_LIMIT);

        if (!stream->m_frames.isEmpty()) {
            FramePtr frame = stream->m_frames.dequeue();
            stream->processData(frame);

            if (stream->m_frames.size() < stream->m_maxData)
                stream->m_dataQueueNotFull.wakeAll();
        }

        stream->m_dataMutex.unlock();
    }
}

void ConvertVideoFFmpeg::deleteFrame(AVFrame *frame)
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

void ConvertVideoFFmpeg::processData(const FramePtr &frame)
{
    forever {
        AkFrac timeBase = this->m_fps.invert();
        qreal pts = frame->pts * timeBase.value();
        qreal diff = pts - this->m_globalClock.clock();
        qreal delay = pts - this->m_lastPts;

        // skip or repeat frame. We take into account the
        // delay to compute the threshold. I still don't know
        // if it is the best guess
        qreal syncThreshold = qBound(AV_SYNC_THRESHOLD_MIN,
                                     delay,
                                     AV_SYNC_THRESHOLD_MAX);

        if (!qIsNaN(diff)
            && qAbs(diff) < AV_NOSYNC_THRESHOLD
            && delay < AV_SYNC_FRAMEDUP_THRESHOLD) {
            // video is backward the external clock.
            if (diff <= -syncThreshold) {
                this->m_lastPts = pts;

                break;
            } else if (diff > syncThreshold) {
                // video is ahead the external clock.
                QThread::usleep(ulong(1e6 * (diff - syncThreshold)));

                continue;
            }
        } else
            this->m_globalClock.setClock(pts);

        this->convert(frame);
        this->log(diff);
        this->m_lastPts = pts;

        break;
    }
}

void ConvertVideoFFmpeg::convert(const FramePtr &frame)
{
    AVPixelFormat outPixFormat = AV_PIX_FMT_RGB24;

    // Initialize rescaling context.
    this->m_scaleContext = sws_getCachedContext(this->m_scaleContext,
                                                frame->width,
                                                frame->height,
                                                AVPixelFormat(frame->format),
                                                frame->width,
                                                frame->height,
                                                outPixFormat,
                                                SWS_FAST_BILINEAR,
                                                nullptr,
                                                nullptr,
                                                nullptr);

    if (!this->m_scaleContext)
        return;

    // Create oPicture
    AVFrame oFrame;
    memset(&oFrame, 0, sizeof(AVFrame));

    if (av_image_check_size(uint(frame->width),
                            uint(frame->height),
                            0,
                            nullptr) < 0)
        return;

    if (av_image_fill_linesizes(oFrame.linesize,
                                outPixFormat,
                                frame->width) < 0)
        return;

    uint8_t *data[4];
    memset(data, 0, 4 * sizeof(uint8_t *));
    int frameSize = av_image_fill_pointers(data,
                                           outPixFormat,
                                           frame->height,
                                           nullptr,
                                           oFrame.linesize);

    QByteArray oBuffer(frameSize, 0);

    if (av_image_fill_pointers(reinterpret_cast<uint8_t **>(oFrame.data),
                               outPixFormat,
                               frame->height,
                               reinterpret_cast<uint8_t *>(oBuffer.data()),
                               oFrame.linesize) < 0) {
        return;
    }

    // Convert picture format
    sws_scale(this->m_scaleContext,
              frame->data,
              frame->linesize,
              0,
              frame->height,
              oFrame.data,
              oFrame.linesize);

    AkVideoCaps caps;
    caps.isValid() = true;
    caps.format() = AkVideoCaps::Format_rgb24;
    caps.bpp() = AkVideoCaps::bitsPerPixel(caps.format());
    caps.width() = frame->width;
    caps.height() = frame->height;
    caps.fps() = this->m_fps;

    // Create packet
    AkVideoPacket oPacket;
    oPacket.caps() = caps;
    oPacket.buffer() = oBuffer;
    oPacket.id() = this->m_id;
    oPacket.pts() = frame->pts;
    oPacket.timeBase() = caps.fps().invert();
    oPacket.index() = 0;

    emit this->frameReady(oPacket.toPacket());
}

void ConvertVideoFFmpeg::log(qreal diff)
{
    if (!this->m_showLog)
        return;

    QString logFmt("%1 %2: %3 vq=%5KB");

    QString log = logFmt.arg(this->m_globalClock.clock(), 7, 'f', 2)
                        .arg("M-V")
                        .arg(-diff, 7, 'f', 3)
                        .arg(this->m_packetQueueSize / 1024, 5);

    qDebug() << log.toStdString().c_str();
}

int64_t ConvertVideoFFmpeg::bestEffortTimestamp(const AVFrame *frame) const
{
#ifdef FF_API_PKT_PTS
    return av_frame_get_best_effort_timestamp(frame);
#else
    if (frame->pts != AV_NOPTS_VALUE)
        return frame->pts;
    else if (frame->pkt_pts != AV_NOPTS_VALUE)
        return frame->pkt_pts;

    return frame->pkt_dts;
#endif
}

AVFrame *ConvertVideoFFmpeg::copyFrame(AVFrame *frame) const
{
#ifdef HAVE_FRAMEALLOC
    auto oFrame = av_frame_alloc();
#else
    auto oFrame = avcodec_alloc_frame();
#endif
    oFrame->width = frame->width;
    oFrame->height = frame->height;
    oFrame->format = frame->format;
    oFrame->pts = frame->pts;

    av_image_alloc(oFrame->data,
                   oFrame->linesize,
                   oFrame->width,
                   oFrame->height,
                   AVPixelFormat(oFrame->format),
                   1);
    av_image_copy(oFrame->data,
                  oFrame->linesize,
                  const_cast<const uint8_t **>(frame->data),
                  frame->linesize,
                  AVPixelFormat(oFrame->format),
                  oFrame->width,
                  oFrame->height);

    return oFrame;
}

void ConvertVideoFFmpeg::setMaxPacketQueueSize(qint64 maxPacketQueueSize)
{
    if (this->m_maxPacketQueueSize == maxPacketQueueSize)
        return;

    this->m_maxPacketQueueSize = maxPacketQueueSize;
    emit this->maxPacketQueueSizeChanged(maxPacketQueueSize);
}

void ConvertVideoFFmpeg::setShowLog(bool showLog)
{
    if (this->m_showLog == showLog)
        return;

    this->m_showLog = showLog;
    emit this->showLogChanged(showLog);
}

void ConvertVideoFFmpeg::resetMaxPacketQueueSize()
{
    this->setMaxPacketQueueSize(15 * 1024 * 1024);
}

void ConvertVideoFFmpeg::resetShowLog()
{
    this->setShowLog(false);
}
