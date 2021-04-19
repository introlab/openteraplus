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
    columns: 3

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

    // Marker type.
    Label {
        text: qsTr("Mode")
    }
    ComboBox {
        id: cbxMode
        currentIndex: modeIndex(Hypnotic.mode)

        model: ListModel {
            ListElement {
                text: qsTr("Spiral 1")
                mode: "spiral1"
            }
            ListElement {
                text: qsTr("Spiral 2")
                mode: "spiral2"
            }
            ListElement {
                text: qsTr("Parabola")
                mode: "parabola"
            }
            ListElement {
                text: qsTr("Horizontal stripe")
                mode: "horizontalStripe"
            }
        }

        onCurrentIndexChanged: Hypnotic.mode = cbxMode.model.get(currentIndex).mode
    }
    Label {
    }

    Label {
        text: qsTr("Speed increment")
    }
    TextField {
        text: Hypnotic.speedInc
        validator: RegExpValidator {
            regExp: /-?\d+/
        }

        onTextChanged: Hypnotic.speedInc = text
    }
    Label {
    }

    Label {
        text: qsTr("Threshold")
    }
    TextField {
        text: Hypnotic.threshold
        validator: RegExpValidator {
            regExp: /-?\d+/
        }

        onTextChanged: Hypnotic.threshold = text
    }
    Label {
    }

    Label {
        text: qsTr("Threshold")
    }
    Slider {
        id: sldThreshold
        value: Hypnotic.threshold
        stepSize: 1
        maximumValue: 255

        onValueChanged: Hypnotic.threshold = value
    }
    SpinBox {
        id: spbThreshold
        value: sldThreshold.value
        maximumValue: sldThreshold.maximumValue
        stepSize: sldThreshold.stepSize

        onValueChanged: sldThreshold.value = value
    }
}
