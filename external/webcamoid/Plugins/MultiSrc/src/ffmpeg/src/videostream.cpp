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

#include "videostream.h"

// no AV sync correction is done if below the minimum AV sync threshold
#define AV_SYNC_THRESHOLD_MIN 0.04

// AV sync correction is done if above the maximum AV sync threshold
#define AV_SYNC_THRESHOLD_MAX 0.1

// If a frame duration is longer than this, it will not be duplicated to compensate AV sync
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1

// no AV correction is done if too big error
#define AV_NOSYNC_THRESHOLD 10.0

VideoStream::VideoStream(const AVFormatContext *formatContext,
                         uint index, qint64 id, Clock *globalClock,
                         bool noModify, QObject *parent):
    AbstractStream(formatContext, index, id, globalClock, noModify, parent)
{
    this->m_maxData = 3;
    this->m_scaleContext = nullptr;
    this->m_lastPts = 0;
}

VideoStream::~VideoStream()
{
    if (this->m_scaleContext)
        sws_freeContext(this->m_scaleContext);
}

AkCaps VideoStream::caps() const
{
    AkVideoCaps caps;
    caps.isValid() = true;
    caps.format() = AkVideoCaps::Format_rgb24;
    caps.bpp() = AkVideoCaps::bitsPerPixel(caps.format());
    caps.width() = this->codecContext()->width;
    caps.height() = this->codecContext()->height;
    caps.fps() = this->fps();

    return caps.toCaps();
}

void VideoStream::processPacket(AVPacket *packet)
{
    if (!this->isValid())
        return;

    if (!packet) {
        this->dataEnqueue(nullptr);

        return;
    }

#ifdef HAVE_SENDRECV
    if (avcodec_send_packet(this->codecContext(), packet) >= 0)
        forever {
    #ifdef HAVE_FRAMEALLOC
            auto iFrame = av_frame_alloc();
    #else
            auto iFrame = avcodec_alloc_frame();
    #endif
            int r = avcodec_receive_frame(this->codecContext(), iFrame);

            if (r >= 0) {
                iFrame->pts = this->bestEffortTimestamp(iFrame);
                this->dataEnqueue(this->copyFrame(iFrame));
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
        avcodec_decode_video2(this->codecContext(), iFrame, &gotFrame, packet);

        if (gotFrame) {
            iFrame->pts = this->bestEffortTimestamp(iFrame);
            this->dataEnqueue(this->copyFrame(iFrame));
        }

    #ifdef HAVE_FRAMEALLOC
        av_frame_free(&iFrame);
    #else
        avcodec_free_frame(&iFrame);
    #endif
#endif
}

void VideoStream::processData(AVFrame *frame)
{
    forever {
        qreal pts = frame->pts * this->timeBase().value();
        qreal diff = pts - this->globalClock()->clock();
        qreal delay = pts - this->m_lastPts;

        // Skip or repeat frame. We take into account the
        // delay to compute the threshold. I still don't know
        // if it is the best guess.
        qreal syncThreshold = qBound(AV_SYNC_THRESHOLD_MIN,
                                     delay,
                                     AV_SYNC_THRESHOLD_MAX);

        //TODO - This is causing latency on RTSP Streams

        /*if (!qIsNaN(diff)
            && qAbs(diff) < AV_NOSYNC_THRESHOLD
            && delay < AV_SYNC_FRAMEDUP_THRESHOLD) {
            // Video is backward the external clock.
            if (diff <= -syncThreshold) {
                // Drop frame.
                this->m_lastPts = pts;

                break;
            } else if (diff > syncThreshold) {
                // Video is ahead the external clock.
                QThread::usleep(ulong(1e6 * (diff - syncThreshold)));

                continue;
            }
        } else*/
            this->globalClock()->setClock(pts);

        this->m_clockDiff = diff;
        AkPacket oPacket = this->convert(frame);
        emit this->oStream(oPacket);
        emit this->frameSent();

        this->m_lastPts = pts;

        break;
    }
}

AkFrac VideoStream::fps() const
{
    AkFrac fps;

    if (this->stream()->avg_frame_rate.num
        && this->stream()->avg_frame_rate.den)
        fps = AkFrac(this->stream()->avg_frame_rate.num,
                     this->stream()->avg_frame_rate.den);
    else
        fps = AkFrac(this->stream()->r_frame_rate.num,
                     this->stream()->r_frame_rate.den);

    return fps;
}

AkPacket VideoStream::convert(AVFrame *iFrame)
{
    AVPixelFormat outPixFormat = AV_PIX_FMT_RGB24;

    // Initialize rescaling context.
    this->m_scaleContext = sws_getCachedContext(this->m_scaleContext,
                                                  iFrame->width,
                                                  iFrame->height,
                                                  AVPixelFormat(iFrame->format),
                                                  iFrame->width,
                                                  iFrame->height,
                                                  outPixFormat,
                                                  SWS_FAST_BILINEAR,
                                                  nullptr,
                                                  nullptr,
                                                  nullptr);

    if (!this->m_scaleContext)
        return AkPacket();

    // Create oPicture
    AVFrame oFrame;
    memset(&oFrame, 0, sizeof(AVFrame));

    if (av_image_check_size(uint(iFrame->width),
                            uint(iFrame->height),
                            0,
                            nullptr) < 0)
        return AkPacket();

    if (av_image_fill_linesizes(oFrame.linesize,
                                outPixFormat,
                                iFrame->width) < 0)
        return AkPacket();

    uint8_t *data[4];
    memset(data, 0, 4 * sizeof(uint8_t *));
    int frameSize = av_image_fill_pointers(data,
                                           outPixFormat,
                                           iFrame->height,
                                           nullptr,
                                           oFrame.linesize);

    QByteArray oBuffer(frameSize, 0);

    if (av_image_fill_pointers(reinterpret_cast<uint8_t **>(oFrame.data),
                               outPixFormat,
                               iFrame->height,
                               reinterpret_cast<uint8_t *>(oBuffer.data()),
                               oFrame.linesize) < 0) {
        return AkPacket();
    }

    // Convert picture format
    sws_scale(this->m_scaleContext,
              iFrame->data,
              iFrame->linesize,
              0,
              iFrame->height,
              oFrame.data,
              oFrame.linesize);

    AkVideoCaps caps;
    caps.isValid() = true;
    caps.format() = AkVideoCaps::Format_rgb24;
    caps.bpp() = AkVideoCaps::bitsPerPixel(caps.format());
    caps.width() = iFrame->width;
    caps.height() = iFrame->height;
    caps.fps() = this->fps();

    // Create packet
    AkVideoPacket oPacket;
    oPacket.caps() = caps;
    oPacket.buffer() = oBuffer;
    oPacket.pts() = iFrame->pts;
    oPacket.timeBase() = this->timeBase();
    oPacket.index() = int(this->index());
    oPacket.id() = this->id();

    return oPacket.toPacket();
}

int64_t VideoStream::bestEffortTimestamp(const AVFrame *frame) const
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

AVFrame *VideoStream::copyFrame(AVFrame *frame) const
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
