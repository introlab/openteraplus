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

import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

GridLayout {
    columns: 2

    function modeIndex(mode)
    {
        var index = -1

        for (var i = 0; i < cbxMode.model.count; i++)
            if (cbxMode.model.get(i).mode === mode) {
                index = i
                break
            }

        return index
    }

    Label {
        text: qsTr("Mode")
    }
    ComboBox {
        id: cbxMode
        currentIndex: modeIndex(Ripple.mode)

        model: ListModel {
            ListElement {
                text: qsTr("Motion detect")
                mode: "motionDetect"
            }
            ListElement {
                text: qsTr("Rain")
                mode: "rain"
            }
        }

        onCurrentIndexChanged: Ripple.mode = cbxMode.model.get(currentIndex).mode
    }

    Label {
        text: qsTr("Amplitude")
    }
    TextField {
        text: Ripple.amplitude
        validator: RegExpValidator {
            regExp: /\d+/
        }

        onTextChanged: Ripple.amplitude = text
    }

    Label {
        text: qsTr("Decay")
    }
    TextField {
        text: Ripple.decay
        validator: RegExpValidator {
            regExp: /\d+/
        }

        onTextChanged: Ripple.decay = text
    }

    Label {
        text: qsTr("Threshold")
    }
    TextField {
        text: Ripple.threshold
        validator: RegExpValidator {
            regExp: /\d+/
        }

        onTextChanged: Ripple.threshold = text
    }

    Label {
        text: qsTr("Luma threshold")
    }
    TextField {
        text: Ripple.lumaThreshold
        validator: RegExpValidator {
            regExp: /\d+/
        }

        onTextChanged: Ripple.lumaThreshold = text
    }
}
