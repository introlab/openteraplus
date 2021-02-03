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

#include <QtMath>
#include <QColor>

#include "rippleelement.h"

typedef QMap<RippleElement::RippleMode, QString> RippleModeToStr;

inline RippleModeToStr initRippleModeToStr()
{
    RippleModeToStr rippleModeToStr {
        {RippleElement::RippleModeMotionDetect, "motionDetect"},
        {RippleElement::RippleModeRain        , "rain"        }
    };

    return rippleModeToStr;
}

Q_GLOBAL_STATIC_WITH_ARGS(RippleModeToStr, rippleModeToStr, (initRippleModeToStr()))

RippleElement::RippleElement(): AkElement()
{
    this->m_mode = RippleModeMotionDetect;
    this->m_amplitude = 256;
    this->m_decay = 8;
    this->m_threshold = 15;
    this->m_lumaThreshold = 15;
}

QString RippleElement::mode() const
{
    return rippleModeToStr->value(this->m_mode);
}

int RippleElement::amplitude() const
{
    return this->m_amplitude;
}

int RippleElement::decay() const
{
    return this->m_decay;
}

int RippleElement::threshold() const
{
    return this->m_threshold;
}

int RippleElement::lumaThreshold() const
{
    return this->m_lumaThreshold;
}

QImage RippleElement::imageDiff(const QImage &img1,
                                const QImage &img2,
                                int threshold,
                                int lumaThreshold,
                                int strength)
{
    int width = qMin(img1.width(), img2.width());
    int height = qMin(img1.height(), img2.height());
    QImage diff(width, height, QImage::Format_ARGB32);

    for (int y = 0; y < height; y++) {
        const QRgb *img1Line = reinterpret_cast<const QRgb *>(img1.constScanLine(y));
        const QRgb *img2Line = reinterpret_cast<const QRgb *>(img2.constScanLine(y));
        int *diffLine = reinterpret_cast<int *>(diff.scanLine(y));

        for (int x = 0; x < width; x++) {
            QRgb pixel1 = img1Line[x];
            int r1 = qRed(pixel1);
            int g1 = qGreen(pixel1);
            int b1 = qBlue(pixel1);

            QRgb pixel2 = img2Line[x];
            int r2 = qRed(pixel2);
            int g2 = qGreen(pixel2);
            int b2 = qBlue(pixel2);

            int dr = r1 - r2;
            int dg = g1 - g2;
            int db = b1 - b2;

            int s = dr * dr + dg * dg + db * db;
            s = int(sqrt(s / 3));
            s = s < threshold? 0: s;

            int gray = qGray(img2Line[x]);
            s = gray < lumaThreshold? 0: s;

            diffLine[x] = (strength * s) >> 8;
        }
    }

    return diff;
}

void RippleElement::addDrops(const QImage &buffer, const QImage &drops)
{
    for (int y = 0; y < buffer.height(); y++) {
        const int *dropsLine = reinterpret_cast<const int *>(drops.constScanLine(y));
        int *bufferLine = const_cast<int *>(reinterpret_cast<const int *>(buffer.scanLine(y)));

        for (int x = 0; x < buffer.width(); x++)
            bufferLine[x] += dropsLine[x];
    }
}

void RippleElement::ripple(const QImage &buffer1, const QImage &buffer2, int decay)
{
    QImage buffer3(buffer1.size(), buffer1.format());
    const int *buffer1Bits = reinterpret_cast<const int *>(buffer1.constBits());
    int *buffer2Bits = const_cast<int *>(reinterpret_cast<const int *>(buffer2.bits()));
    int *buffer3Bits = const_cast<int *>(reinterpret_cast<const int *>(buffer3.bits()));
    int widthM1 = buffer1.width() - 1;
    int widthP1 = buffer1.width() + 1;
    int height = buffer1.height() - 1;

    memset(buffer2Bits, 0, size_t(buffer1.bytesPerLine()));
    memset(buffer2Bits + height * buffer1.width(), 0, size_t(buffer1.bytesPerLine()));
    memset(buffer3Bits, 0, size_t(buffer1.bytesPerLine()));
    memset(buffer3Bits + height * buffer1.width(), 0, size_t(buffer1.bytesPerLine()));

    for (int y = 1; y < height; y++) {
        buffer2Bits[y * buffer1.width()] = 0;
        buffer2Bits[widthM1 + y * buffer1.width()] = 0;
        buffer3Bits[y * buffer1.width()] = 0;
        buffer3Bits[widthM1 + y * buffer1.width()] = 0;
    }

    // Wave simulation.
    for (int y = 1; y < height; y++) {
        int xOfftset = y * buffer1.width();

        for (int x = 1; x < widthM1; x++) {
            int xp = x + xOfftset;
            int h = 0;

            h += buffer1Bits[xp - widthP1];
            h += buffer1Bits[xp - buffer1.width()];
            h += buffer1Bits[xp - widthM1];
            h += buffer1Bits[xp - 1];
            h += buffer1Bits[xp + 1];
            h += buffer1Bits[xp + widthM1];
            h += buffer1Bits[xp + buffer1.width()];
            h += buffer1Bits[xp + widthP1];
            h -= 9 * buffer1Bits[xp];
            h >>= 3;

            int v = buffer1Bits[xp] - buffer2Bits[xp];
            v += h - (v >> decay);
            buffer3Bits[xp] = v + buffer1Bits[xp];
        }
    }

    // Low pass filter.
    for (int y = 1; y < height; y++) {
        int xOfftset = y * buffer1.width();

        for (int x = 1; x < widthM1; x++) {
            int xp = x + xOfftset;

            int h = 0;

            h += buffer3Bits[xp - 1];
            h += buffer3Bits[xp + 1];
            h += buffer3Bits[xp - buffer3.width()];
            h += buffer3Bits[xp + buffer3.width()];
            h += 60 * buffer3Bits[xp];

            buffer2Bits[xp] = h >> 6;
        }
    }
}

QImage RippleElement::applyWater(const QImage &src, const QImage &buffer)
{
    QImage dest(src.size(), src.format());
    const QRgb *srcBits = reinterpret_cast<const QRgb *>(src.constBits());
    const int *bufferBits = reinterpret_cast<const int *>(buffer.bits());
    QRgb *destBits = reinterpret_cast<QRgb *>(dest.bits());

    for (int y = 0; y < src.height(); y++) {
        int xOfftset = y * src.width();

        for (int x = 0; x < src.width(); x++) {
            int xp = x + xOfftset;

            int xOff = 0;

            if (x > 1
                && x < src.width() - 1) {
                xOff += bufferBits[xp - 1];
                xOff -= bufferBits[xp + 1];
            }

            int yOff = 0;

            if (y > 1
                && y < src.height() - 1) {
                yOff += bufferBits[xp - buffer.width()];
                yOff -= bufferBits[xp + buffer.width()];
            }

            QColor color;
            int xq = qBound(0, x + xOff, src.width() - 1);
            int yq = qBound(0, y + yOff, src.height() - 1);

            color.setRgba(srcBits[xq + yq * src.width()]);

            // Shading
            int lightness = color.lightness() + xOff;
            lightness = qBound(0, lightness, 255);
            color.setHsl(color.hue(), color.saturation(), lightness);

            destBits[xp] = color.rgb();
        }
    }

    return dest;
}

QImage RippleElement::rainDrop(int width, int height, int strength)
{
    if (this->m_period == 0) {
        if (this->m_rainStat == 0) {
            this->m_period = (qrand() >> 23) + 100;
            this->m_dropProb = 0;
            this->m_dropProbIncrement = 0x00ffffff / this->m_period;
            this->m_dropPower = qrand() % (strength << 1) - strength;
            this->m_dropsPerFrameMax = 2 << (qrand() >> 30); // 2,4,8 or 16
            this->m_rainStat = 1;
        } else if (this->m_rainStat == 1) {
            this->m_dropProb = 0x00ffffff;
            this->m_dropsPerFrame = 1;
            this->m_dropProbIncrement = 1;
            this->m_period = 16 * (this->m_dropsPerFrameMax - 1);
            this->m_rainStat = 2;
        } else if (this->m_rainStat == 2) {
            m_period = (qrand() >> 22) + 1000;
            m_dropProbIncrement = 0;
            m_rainStat = 3;
        } else if (this->m_rainStat == 3) {
            this->m_period = 16 * (this->m_dropsPerFrameMax - 1);
            this->m_dropProbIncrement = -1;
            this->m_rainStat = 4;
        } else if (this->m_rainStat == 4) {
            this->m_period = (qrand() >> 24) + 60;
            this->m_dropProbIncrement = -int(this->m_dropProb) / this->m_period;
            this->m_rainStat = 5;
        } else {
            this->m_period = (qrand() >> 23) + 500;
            this->m_dropProb = 0;
            this->m_rainStat = 0;
        }
    }

    QImage rain;

    if (this->m_rainStat == 1
        || this->m_rainStat == 5) {

        if ((qrand() >> 8) < int(this->m_dropProb))
            rain = this->drop(width, height, this->m_dropPower);

        this->m_dropProb += uint(this->m_dropProbIncrement);
    } else if (this->m_rainStat == 2
               || this->m_rainStat == 3
               || this->m_rainStat == 4) {
        for  (int i = this->m_dropsPerFrame / 16; i > 0; i--)
            rain = this->drop(width, height, this->m_dropPower);

        this->m_dropsPerFrame += this->m_dropProbIncrement;
    }

    this->m_period--;

    if (rain.isNull()) {
        rain = QImage(width, height, QImage::Format_ARGB32);
        rain.fill(qRgba(0, 0, 0, 0));
    }

    return rain;
}

QString RippleElement::controlInterfaceProvide(const QString &controlId) const
{
    Q_UNUSED(controlId)

    return QString("qrc:/Ripple/share/qml/main.qml");
}

void RippleElement::controlInterfaceConfigure(QQmlContext *context,
                                              const QString &controlId) const
{
    Q_UNUSED(controlId)

    context->setContextProperty("Ripple", const_cast<QObject *>(qobject_cast<const QObject *>(this)));
    context->setContextProperty("controlId", this->objectName());
}

void RippleElement::setMode(const QString &mode)
{
    RippleMode modeEnum = rippleModeToStr->key(mode, RippleModeMotionDetect);

    if (this->m_mode == modeEnum)
        return;

    this->m_mode = modeEnum;
    emit this->modeChanged(mode);
}

void RippleElement::setAmplitude(int amplitude)
{
    if (this->m_amplitude == amplitude)
        return;

    this->m_amplitude = amplitude;
    emit this->amplitudeChanged(amplitude);
}

void RippleElement::setDecay(int decay)
{
    if (this->m_decay == decay)
        return;

    this->m_decay = decay;
    emit this->decayChanged(decay);
}

void RippleElement::setThreshold(int threshold)
{
    if (this->m_threshold == threshold)
        return;

    this->m_threshold = threshold;
    emit this->thresholdChanged(threshold);
}

void RippleElement::setLumaThreshold(int lumaThreshold)
{
    if (this->m_lumaThreshold == lumaThreshold)
        return;

    this->m_lumaThreshold = lumaThreshold;
    emit this->lumaThresholdChanged(lumaThreshold);
}

void RippleElement::resetMode()
{
    this->setMode("motionDetect");
}

void RippleElement::resetAmplitude()
{
    this->setAmplitude(256);
}

void RippleElement::resetDecay()
{
    this->setDecay(8);
}

void RippleElement::resetThreshold()
{
    this->setThreshold(15);
}

void RippleElement::resetLumaThreshold()
{
    this->setLumaThreshold(15);
}

AkPacket RippleElement::iStream(const AkPacket &packet)
{
    QImage src = AkUtils::packetToImage(packet);

    if (src.isNull())
        return AkPacket();

    src = src.convertToFormat(QImage::Format_ARGB32);
    QImage oFrame(src.size(), src.format());

    if (packet.caps() != this->m_caps) {
        this->m_prevFrame = QImage();

        this->m_period = 0;
        this->m_rainStat = 0;
        this->m_dropProb = 0;
        this->m_dropProbIncrement = 0;
        this->m_dropsPerFrameMax = 0;
        this->m_dropsPerFrame = 0;
        this->m_dropPower = 0;

        this->m_caps = packet.caps();
    }

    if (this->m_prevFrame.isNull()) {
        oFrame = src;
        this->m_rippleBuffer.clear();
        this->m_rippleBuffer << QImage(src.size(), src.format());
        this->m_rippleBuffer[0].fill(qRgba(0, 0, 0, 0));
        this->m_rippleBuffer << QImage(src.size(), src.format());
        this->m_rippleBuffer[1].fill(qRgba(0, 0, 0, 0));
        this->m_curRippleBuffer = 0;
    } else {
        QImage drops;

        if (this->m_mode == RippleModeMotionDetect)
            // Compute the difference between previous and current frame,
            // and save it to the buffer.
            drops = this->imageDiff(this->m_prevFrame,
                                    src,
                                    this->m_threshold,
                                    this->m_lumaThreshold,
                                    this->m_amplitude);
        else
            drops = this->rainDrop(src.width(), src.height(), this->m_amplitude);

        this->addDrops(this->m_rippleBuffer[this->m_curRippleBuffer], drops);
        this->addDrops(this->m_rippleBuffer[1 - this->m_curRippleBuffer], drops);

        this->ripple(this->m_rippleBuffer[this->m_curRippleBuffer],
                     this->m_rippleBuffer[1 - this->m_curRippleBuffer],
                     this->m_decay);

        // Apply buffer.
        oFrame = this->applyWater(src, this->m_rippleBuffer[this->m_curRippleBuffer]);
        this->m_curRippleBuffer = 1 - this->m_curRippleBuffer;
    }

    this->m_prevFrame = src.copy();

    AkPacket oPacket = AkUtils::imageToPacket(oFrame, packet);
    akSend(oPacket)
}
