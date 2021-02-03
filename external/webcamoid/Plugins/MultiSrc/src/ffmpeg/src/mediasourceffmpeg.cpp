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

#include <QApplication>
#include <QDesktopWidget>

#include "mediasourceffmpeg.h"
#include "videostream.h"
#include "audiostream.h"
#include "subtitlestream.h"

typedef QMap<AVMediaType, QString> AvMediaTypeStrMap;

inline AvMediaTypeStrMap initAvMediaTypeStrMap()
{
    AvMediaTypeStrMap mediaTypeToStr = {
        {AVMEDIA_TYPE_UNKNOWN   , "unknown/x-raw"   },
        {AVMEDIA_TYPE_VIDEO     , "video/x-raw"     },
        {AVMEDIA_TYPE_AUDIO     , "audio/x-raw"     },
        {AVMEDIA_TYPE_DATA      , "data/x-raw"      },
        {AVMEDIA_TYPE_SUBTITLE  , "text/x-raw"      },
        {AVMEDIA_TYPE_ATTACHMENT, "attachment/x-raw"},
        {AVMEDIA_TYPE_NB        , "nb/x-raw"        }
    };

    return mediaTypeToStr;
}

Q_GLOBAL_STATIC_WITH_ARGS(AvMediaTypeStrMap, mediaTypeToStr, (initAvMediaTypeStrMap()))

MediaSourceFFmpeg::MediaSourceFFmpeg(QObject *parent):
    MediaSource(parent)
{
    av_register_all();
    avformat_network_init();

    this->m_loop = false;
    this->m_run = false;
    this->m_maxPacketQueueSize = 15 * 1024 * 1024;
    this->m_showLog = false;
    this->m_curState = AkElement::ElementStateNull;
    this->m_curClockTime = 0.;

#ifndef QT_DEBUG
    av_log_set_level(AV_LOG_QUIET);
#endif

    if (this->m_threadPool.maxThreadCount() < 2)
        this->m_threadPool.setMaxThreadCount(2);

    // Reconnect timer for streams
    reconnectTimer.setInterval(100);
    reconnectTimer.setSingleShot(true);
    connect(&reconnectTimer, SIGNAL(timeout()), this, SLOT(reconnectStream()));
}

MediaSourceFFmpeg::~MediaSourceFFmpeg()
{
    this->setState(AkElement::ElementStateNull);
}

QStringList MediaSourceFFmpeg::medias() const
{
    QStringList medias;

    if (!this->m_media.isEmpty())
        medias << this->m_media;

    return medias;
}

QString MediaSourceFFmpeg::media() const
{
    return this->m_media;
}

QList<int> MediaSourceFFmpeg::streams() const
{
    return this->m_streams;
}

QList<int> MediaSourceFFmpeg::listTracks(const QString &mimeType)
{
    QList<int> tracks;
    bool clearContext = false;

    if (!this->m_inputContext) {
        if (!this->initContext())
            return tracks;

        clearContext = true;
    }

    for (uint stream = 0; stream < this->m_inputContext->nb_streams; stream++) {
#ifdef HAVE_CODECPAR
        auto type = this->m_inputContext->streams[stream]->codecpar->codec_type;
#else
        auto type = this->m_inputContext->streams[stream]->codec->codec_type;
#endif

        if (mimeType.isEmpty()
            || mediaTypeToStr->value(type) == mimeType)
            tracks << int(stream);
    }

    if (clearContext)
        this->m_inputContext.clear();

    return tracks;
}

QString MediaSourceFFmpeg::streamLanguage(int stream)
{
    bool clearContext = false;

    if (!this->m_inputContext) {
        if (!this->initContext())
            return QString();

        clearContext = true;
    }

    AVDictionary *metadata = this->m_inputContext->streams[stream]->metadata;
    AVDictionaryEntry *dicEntry = nullptr;
    QString language;

    while ((dicEntry = av_dict_get(metadata, "", dicEntry, AV_DICT_IGNORE_SUFFIX))) {
        QString key(dicEntry->key);
        QString value(dicEntry->value);

        if (key == "language") {
            language = value;

            break;
        }
    }

    if (clearContext)
        this->m_inputContext.clear();

    return language;
}

bool MediaSourceFFmpeg::loop() const
{
    return this->m_loop;
}

int MediaSourceFFmpeg::defaultStream(const QString &mimeType)
{
    int stream = -1;
    bool clearContext = false;

    if (!this->m_inputContext) {
        if (!this->initContext())
            return stream;

        clearContext = true;
    }

    for (uint i = 0; i < this->m_inputContext->nb_streams; i++) {
#ifdef HAVE_CODECPAR
        AVMediaType type = this->m_inputContext->streams[i]->codecpar->codec_type;
#else
        AVMediaType type = this->m_inputContext->streams[i]->codec->codec_type;
#endif

        if (mediaTypeToStr->value(type) == mimeType) {
            stream = int(i);

            break;
        }
    }

    if (clearContext)
        this->m_inputContext.clear();

    return stream;
}

QString MediaSourceFFmpeg::description(const QString &media) const
{
    if (this->m_media != media)
        return QString();

    return QFileInfo(media).baseName();
}

AkCaps MediaSourceFFmpeg::caps(int stream)
{
    bool clearContext = false;

    if (!this->m_inputContext) {
        if (!this->initContext())
            return AkCaps();

        if (avformat_find_stream_info(this->m_inputContext.data(), nullptr) < 0) {
            this->m_inputContext.clear();

            return AkCaps();
        }

        clearContext = true;
    }

    AkCaps caps;

    if (stream >= 0
        && stream < int(this->m_inputContext->nb_streams)) {
        AbstractStreamPtr streamPtr = this->createStream(stream, true);
        caps = streamPtr->caps();
    }

    if (clearContext)
        this->m_inputContext.clear();

    return caps;
}

qint64 MediaSourceFFmpeg::maxPacketQueueSize() const
{
    return this->m_maxPacketQueueSize;
}

bool MediaSourceFFmpeg::showLog() const
{
    return this->m_showLog;
}

qint64 MediaSourceFFmpeg::packetQueueSize()
{
    qint64 size = 0;

    for (const AbstractStreamPtr &stream: this->m_streamsMap.values())
        size += stream->queueSize();

    return size;
}

void MediaSourceFFmpeg::deleteFormatContext(AVFormatContext *context)
{
    avformat_close_input(&context);
}

AbstractStreamPtr MediaSourceFFmpeg::createStream(int index, bool noModify)
{
    AVMediaType type = AbstractStream::type(this->m_inputContext.data(), uint(index));
    AbstractStreamPtr stream;
    qint64 id = Ak::id();

    if (type == AVMEDIA_TYPE_VIDEO)
        stream = AbstractStreamPtr(new VideoStream(this->m_inputContext.data(),
                                                   uint(index), id,
                                                   &this->m_globalClock,
                                                   noModify));
    else if (type == AVMEDIA_TYPE_AUDIO)
        stream = AbstractStreamPtr(new AudioStream(this->m_inputContext.data(),
                                                   uint(index), id,
                                                   &this->m_globalClock,
                                                   noModify));
    else if (type == AVMEDIA_TYPE_SUBTITLE)
        stream = AbstractStreamPtr(new SubtitleStream(this->m_inputContext.data(),
                                                      uint(index), id,
                                                      &this->m_globalClock,
                                                      noModify));
    else
        stream = AbstractStreamPtr(new AbstractStream(this->m_inputContext.data(),
                                                      uint(index), id,
                                                      &this->m_globalClock,
                                                      noModify));

    return stream;
}

void MediaSourceFFmpeg::readPackets()
{
    while (this->m_run) {

        this->m_dataMutex.lock();

        if (this->packetQueueSize() >= this->m_maxPacketQueueSize)
            if (!this->m_packetQueueNotFull.wait(&this->m_dataMutex, THREAD_WAIT_LIMIT)) {
                this->m_dataMutex.unlock();

                continue;
            }

        AVPacket *packet = new AVPacket();
        av_init_packet(packet);
        bool notuse = true;
        int r = av_read_frame(this->m_inputContext.data(), packet);

        if (r >= 0) {
            if (this->m_streamsMap.contains(packet->stream_index)
                && (this->m_streams.isEmpty()
                    || this->m_streams.contains(packet->stream_index))) {
                this->m_streamsMap[packet->stream_index]->packetEnqueue(packet);
                notuse = false;
            }
        }

        if (notuse) {
#ifdef HAVE_PACKETREF
            av_packet_unref(packet);
#else
            av_destruct_packet(packet);
#endif
            delete packet;
        }

        if (r < 0) {
            //qDebug() << "No reply from video source...";
            if (this->loop()) {
                for (const AbstractStreamPtr &stream: this->m_streamsMap.values())
                    stream->packetEnqueue(nullptr);
            }

            this->m_run = false;
        }

        this->m_dataMutex.unlock();
    }
}

void MediaSourceFFmpeg::unlockQueue()
{
    this->m_dataMutex.lock();

    if (this->packetQueueSize() < this->m_maxPacketQueueSize)
        this->m_packetQueueNotFull.wakeAll();

    if (this->packetQueueSize() < 1)
        this->m_packetQueueEmpty.wakeAll();

    this->m_dataMutex.unlock();
}

void MediaSourceFFmpeg::setMedia(const QString &media)
{
    if (media == this->m_media)
        return;

    bool isRunning = this->m_run;
    this->setState(AkElement::ElementStateNull);
    this->m_media = media;

    if (isRunning && !this->m_media.isEmpty())
        this->setState(AkElement::ElementStatePlaying);

    emit this->mediaChanged(media);
    emit this->mediasChanged(this->medias());
}

void MediaSourceFFmpeg::setStreams(const QList<int> &streams)
{
    if (this->m_streams == streams)
        return;

    this->m_streams = streams;
    emit this->streamsChanged(streams);
}

void MediaSourceFFmpeg::setMaxPacketQueueSize(qint64 maxPacketQueueSize)
{
    if (this->m_maxPacketQueueSize == maxPacketQueueSize)
        return;

    this->m_maxPacketQueueSize = maxPacketQueueSize;
    emit this->maxPacketQueueSizeChanged(maxPacketQueueSize);
}

void MediaSourceFFmpeg::setShowLog(bool showLog)
{
    if (this->m_showLog == showLog)
        return;

    this->m_showLog = showLog;
    emit this->showLogChanged(showLog);
}

void MediaSourceFFmpeg::setLoop(bool loop)
{
    if (this->m_loop == loop)
        return;

    this->m_loop = loop;
    emit this->loopChanged(loop);
}

void MediaSourceFFmpeg::resetMedia()
{
    this->setMedia("");
}

void MediaSourceFFmpeg::resetStreams()
{
    if  (this->m_streams.isEmpty())
        return;

    this->m_streams.clear();
    emit this->streamsChanged(this->m_streams);
}

void MediaSourceFFmpeg::resetMaxPacketQueueSize()
{
    this->setMaxPacketQueueSize(15 * 1024 * 1024);
}

void MediaSourceFFmpeg::resetShowLog()
{
    this->setShowLog(false);
}

void MediaSourceFFmpeg::resetLoop()
{
    this->setLoop(false);
}

bool MediaSourceFFmpeg::setState(AkElement::ElementState state)
{
    switch (this->m_curState) {
    case AkElement::ElementStateNull:
    case AkElement::ElementStateReconnecting: {
        if (state == AkElement::ElementStatePaused
            || state == AkElement::ElementStatePlaying) {
            if (!this->initContext()){
                if (!reconnectTimer.isActive()){
                    m_curState = AkElement::ElementStateReconnecting;
                    reconnectTimer.start();
                }
                return false;
            }

            if (avformat_find_stream_info(this->m_inputContext.data(),
                                          nullptr) < 0) {
                this->m_inputContext.clear();

                return false;
            }

            QString uri = this->m_media;
            av_dump_format(this->m_inputContext.data(),
                           0,
                           uri.toStdString().c_str(),
                           false);

            QList<int> filterStreams;

            if (this->m_streams.isEmpty())
                filterStreams << this->defaultStream("audio/x-raw")
                              << this->defaultStream("video/x-raw");
            else
                filterStreams = this->m_streams;

            for (const int &i: filterStreams) {
                AbstractStreamPtr stream = this->createStream(i);

                if (stream) {
                    this->m_streamsMap[i] = stream;

                    QObject::connect(stream.data(),
                                     SIGNAL(oStream(AkPacket)),
                                     this,
                                     SIGNAL(oStream(AkPacket)),
                                     Qt::DirectConnection);

                    QObject::connect(stream.data(),
                                     SIGNAL(notify()),
                                     this,
                                     SLOT(packetConsumed()));

                    QObject::connect(stream.data(),
                                     SIGNAL(frameSent()),
                                     this,
                                     SLOT(log()));

                    QObject::connect(stream.data(),
                                     SIGNAL(eof()),
                                     this,
                                     SLOT(doLoop()));

                    stream->init();

                    if (state == AkElement::ElementStatePaused)
                        stream->setPaused(true);
                }
            }

            if (state == AkElement::ElementStatePaused)
                this->m_curClockTime = 0.;

            this->m_globalClock.setClock(0.);
            this->m_run = true;
            this->m_readPacketsLoopResult =
                    QtConcurrent::run(&this->m_threadPool,
                                      this,
                                      &MediaSourceFFmpeg::readPackets);
            this->m_curState = state;

            return true;
        }

        break;
    }
    case AkElement::ElementStatePaused: {
        switch (state) {
        case AkElement::ElementStateNull: {
            this->m_globalClock.setClock(this->m_curClockTime);

            for (const AbstractStreamPtr &stream: this->m_streamsMap)
                stream->setPaused(false);

            this->m_run = false;
            this->m_threadPool.waitForDone();

            this->m_dataMutex.lock();
            this->m_packetQueueNotFull.wakeAll();
            this->m_packetQueueEmpty.wakeAll();
            this->m_dataMutex.unlock();

            for (const AbstractStreamPtr &stream: this->m_streamsMap)
                stream->uninit();

            this->m_streamsMap.clear();
            this->m_inputContext.clear();
            this->m_curState = state;

            return true;
        }
        case AkElement::ElementStatePlaying: {
            this->m_globalClock.setClock(this->m_curClockTime);

            for (const AbstractStreamPtr &stream: this->m_streamsMap)
                stream->setPaused(false);

            this->m_curState = state;

            return true;
        }
        case AkElement::ElementStatePaused:
            break;
        }

        break;
    }
    case AkElement::ElementStatePlaying: {
        switch (state) {
        case AkElement::ElementStateNull: {
            this->m_run = false;
            this->m_readPacketsLoopResult.waitForFinished();

            this->m_dataMutex.lock();
            this->m_packetQueueNotFull.wakeAll();
            this->m_packetQueueEmpty.wakeAll();
            this->m_dataMutex.unlock();

            for (const AbstractStreamPtr &stream: this->m_streamsMap)
                stream->uninit();

            this->m_streamsMap.clear();
            this->m_inputContext.clear();
            this->m_curState = state;

            return true;
        }
        case AkElement::ElementStatePaused: {
            this->m_curClockTime = this->m_globalClock.clock();

            for (const AbstractStreamPtr &stream: this->m_streamsMap)
                stream->setPaused(true);

            this->m_curState = state;

            break;
        }
        case AkElement::ElementStatePlaying:
            break;
        }

        break;
    }
    }

    return false;
}

void MediaSourceFFmpeg::doLoop()
{
    //qDebug() << "doLoop()";
    if (m_run && m_curState != AkElement::ElementStateReconnecting){
        this->setState(AkElement::ElementStateNull);
        m_curState = AkElement::ElementStateReconnecting;
        reconnectStream();
    }
}

void MediaSourceFFmpeg::reconnectStream(){
    bool rval = false;

    if (m_curState != AkElement::ElementStateReconnecting){
        qDebug() << "reconnectStream() - bad state.";
        return;
    }

    qDebug() << "reconnectStream()";
    rval=this->setState(AkElement::ElementStatePlaying);

    if (!rval){
        // Prime timer to reconnect
        reconnectTimer.start();
    }
}

void MediaSourceFFmpeg::packetConsumed()
{
    QtConcurrent::run(&this->m_threadPool,
                      this,
                      &MediaSourceFFmpeg::unlockQueue);
}

bool MediaSourceFFmpeg::initContext()
{
    QString uri = this->m_media;

    if (uri.isEmpty())
        return false;

    AVInputFormat *inputFormat = nullptr;
    AVDictionary *inputOptions = nullptr;

    if (QRegExp("/dev/video\\d*").exactMatch(uri))
        inputFormat = av_find_input_format("v4l2");
    else if (QRegExp(":\\d+\\.\\d+(?:\\+\\d+,\\d+)?").exactMatch(uri)) {
        inputFormat = av_find_input_format("x11grab");

        int width = this->roundDown(QApplication::desktop()->width(), 4);
        int height = this->roundDown(QApplication::desktop()->height(), 4);

        av_dict_set(&inputOptions,
                    "video_size",
                    QString("%1x%2").arg(width)
                                    .arg(height)
                                    .toStdString().c_str(),
                    0);

        // draw_mouse (int)
    }
    else if (uri == "pulse" ||
             QRegExp("hw:\\d+").exactMatch(uri))
        inputFormat = av_find_input_format("alsa");
    else if (uri == "/dev/dsp")
        inputFormat = av_find_input_format("oss");
    else if (uri.startsWith("RTSP",Qt::CaseInsensitive))
    {
        //qDebug() << "Setting RTSP options...";
        //av_dict_set(&inputOptions,"rtsp_transport", "tcp",0);
        av_dict_set(&inputOptions,"stimeout", "5000000",0);
        //av_dict_set(&inputOptions,"reorder_queue_size", "0", 0);
        //av_dict_set(&inputOptions,"max_delay", "200000",0);
        //av_dict_set(&inputOptions,"muxdelay", "0.5",0);
    }

    QStringList mmsSchemes;
    mmsSchemes << "mms://";// << "mmsh://" << "mmst://"; //SB: Why do we need all 3??

    AVFormatContext *inputContext = nullptr;

    for (const QString &scheme: mmsSchemes) {
        QString uriCopy = uri;

        for (const QString &schemer: mmsSchemes)
            uriCopy.replace(QRegExp(QString("^%1").arg(schemer)),
                            scheme);

        inputContext = nullptr;

        if (avformat_open_input(&inputContext,
                                uriCopy.toStdString().c_str(),
                                inputFormat,
                                &inputOptions) >= 0)
        {
            qDebug() <<"Done opening : " << uriCopy;
            break;
        }
        else
        {
            qDebug() << "Error opening : " << uriCopy;
        }
    }

    if (inputOptions)
        av_dict_free(&inputOptions);

    if (!inputContext) {
        emit this->error(QString("Can't open \"%1\" stream.").arg(uri));

        return false;
    }

    this->m_inputContext = FormatContextPtr(inputContext, this->deleteFormatContext);

    return true;
}

void MediaSourceFFmpeg::log()
{
    if (!this->m_showLog)
        return;

    AbstractStreamPtr audioStream;
    AbstractStreamPtr videoStream;

    for (const int &streamId: this->m_streamsMap.keys()) {
        AVMediaType mediaType = this->m_streamsMap[streamId]->mediaType();

        if (mediaType == AVMEDIA_TYPE_AUDIO && !audioStream)
            audioStream = this->m_streamsMap[streamId];

        if (mediaType == AVMEDIA_TYPE_VIDEO && !videoStream)
            videoStream = this->m_streamsMap[streamId];

        if (audioStream && videoStream)
            break;
    }

    QString diffType;
    qreal diff;
    qint64 audioQueueSize = 0;
    qint64 videoQueueSize = 0;

    if (audioStream && videoStream) {
        diffType = "A-V";
        diff = audioStream->clockDiff() - videoStream->clockDiff();
        audioQueueSize = audioStream->queueSize();
        videoQueueSize = videoStream->queueSize();
    } else if (audioStream) {
        diffType = "M-A";
        diff = -audioStream->clockDiff();
        audioQueueSize = audioStream->queueSize();
    } else if (videoStream) {
        diffType = "M-V";
        diff = -videoStream->clockDiff();
        videoQueueSize = videoStream->queueSize();
    } else
        return;

    /*QString logFmt("%1 %2: %3 aq=%4KB vq=%5KB");

    QString log = logFmt.arg(this->m_globalClock.clock(), 7, 'f', 2)
                        .arg(diffType)
                        .arg(diff, 7, 'f', 3)
                        .arg(audioQueueSize / 1024, 5)
                        .arg(videoQueueSize / 1024, 5);

    qDebug() << log.toStdString().c_str();*/
}
