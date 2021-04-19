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

#ifndef AUDIODEVICEGLOBALS_H
#define AUDIODEVICEGLOBALS_H

#include <QObject>

class AudioDeviceGlobals: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString audioLib
               READ audioLib
               WRITE setAudioLib
               RESET resetAudioLib
               NOTIFY audioLibChanged)

    public:
        explicit AudioDeviceGlobals(QObject *parent=nullptr);

        Q_INVOKABLE QString audioLib() const;

    private:
        QString m_audioLib;
        QStringList m_preferredLibrary;

    signals:
        void audioLibChanged(const QString &audioLib);

    public slots:
        void setAudioLib(const QString &audioLib);
        void resetAudioLib();
};

#endif // AUDIODEVICEGLOBALS_H
