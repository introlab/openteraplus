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

#ifndef AUDIOGENELEMENT_H
#define AUDIOGENELEMENT_H

#include <QThreadPool>
#include <QtConcurrent>
#include <ak.h>

class AudioGenElement: public AkElement
{
    Q_OBJECT
    Q_ENUMS(WaveType)
    Q_PROPERTY(QString caps
               READ caps
               WRITE setCaps
               RESET resetCaps
               NOTIFY capsChanged)
    Q_PROPERTY(QString waveType
               READ waveType
               WRITE setWaveType
               RESET resetWaveType
               NOTIFY waveTypeChanged)
    Q_PROPERTY(qreal frequency
               READ frequency
               WRITE setFrequency
               RESET resetFrequency
               NOTIFY frequencyChanged)
    Q_PROPERTY(qreal volume
               READ volume
               WRITE setVolume
               RESET resetVolume
               NOTIFY volumeChanged)
    Q_PROPERTY(qreal sampleDuration
               READ sampleDuration
               WRITE setSampleDuration
               RESET resetSampleDuration
               NOTIFY sampleDurationChanged)

    public:
        enum WaveType
        {
            WaveTypeSilence,
            WaveTypeSine,
            WaveTypeSquare,
            WaveTypeTriangle,
            WaveTypeSawtooth,
            WaveTypeWhiteNoise,
        };

        explicit AudioGenElement();

        Q_INVOKABLE QString caps() const;
        Q_INVOKABLE QString waveType() const;
        Q_INVOKABLE qreal frequency() const;
        Q_INVOKABLE qreal volume() const;
        Q_INVOKABLE qreal sampleDuration() const;

    private:
        AkCaps m_caps;
        WaveType m_waveType;
        qreal m_frequency;
        qreal m_volume;
        qreal m_sampleDuration;
        AkElementPtr m_audioConvert;
        QThreadPool m_threadPool;
        QFuture<void> m_readFramesLoopResult;
        QMutex m_mutex;
        bool m_readFramesLoop;
        bool m_pause;
        qint64 m_id;

        void readFramesLoop();

    signals:
        void capsChanged(const QString &caps);
        void waveTypeChanged(const QString &waveType);
        void frequencyChanged(qreal frequency);
        void volumeChanged(qreal volume);
        void sampleDurationChanged(qreal sampleDuration);

    public slots:
        void setCaps(const QString &caps);
        void setWaveType(const QString &waveType);
        void setFrequency(qreal frequency);
        void setVolume(qreal volume);
        void setSampleDuration(qreal sampleDuration);
        void resetCaps();
        void resetWaveType();
        void resetFrequency();
        void resetVolume();
        void resetSampleDuration();
        bool setState(AkElement::ElementState state);
};

#endif // AUDIOGENELEMENT_H
