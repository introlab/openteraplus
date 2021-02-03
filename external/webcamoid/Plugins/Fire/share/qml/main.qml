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

    // Fire mode.
    Label {
        text: qsTr("Mode")
    }
    ComboBox {
        id: cbxMode
        currentIndex: modeIndex(Fire.mode)

        model: ListModel {
            ListElement {
                text: qsTr("Soft")
                mode: "soft"
            }
            ListElement {
                text: qsTr("Hard")
                mode: "hard"
            }
        }

        onCurrentIndexChanged: Fire.mode = cbxMode.model.get(currentIndex).mode
    }

    // Cooling factor.
    Label {
        text: qsTr("Cooling")
    }
    SpinBox {
        value: Fire.cool
        minimumValue: -255
        maximumValue: 255
        stepSize: 1

        onValueChanged: Fire.cool = value
    }

    // Disolving factor.
    Label {
        text: qsTr("Disolve")
    }
    TextField {
        text: Fire.disolve
        validator: RegExpValidator {
            regExp: /-?(\d+\.\d+|\d+\.|\.\d+|\d+)/
        }

        onTextChanged: Fire.disolve = text
    }

    // Blur.
    Label {
        text: qsTr("Blur")
    }
    TextField {
        text: Fire.blur
        validator: RegExpValidator {
            regExp: /\d+/
        }

        onTextChanged: Fire.blur = text
    }

    // Zoom.
    Label {
        text: qsTr("Zoom")
    }
    TextField {
        text: Fire.zoom
        validator: RegExpValidator {
            regExp: /-?(\d+\.\d+|\d+\.|\.\d+|\d+)/
        }

        onTextChanged: Fire.zoom = text
    }

    // Threshold.
    Label {
        text: qsTr("Threshold")
    }
    SpinBox {
        value: Fire.threshold
        maximumValue: 255
        stepSize: 1

        onValueChanged: Fire.threshold = value
    }

    // Luma threshold.
    Label {
        text: qsTr("Luma threshold")
    }
    SpinBox {
        value: Fire.lumaThreshold
        maximumValue: 255
        stepSize: 1

        onValueChanged: Fire.lumaThreshold = value
    }

    // Alpha diff.
    Label {
        text: qsTr("Alpha diff")
    }
    SpinBox {
        value: Fire.alphaDiff
        minimumValue: -255
        maximumValue: 255
        stepSize: 1

        onValueChanged: Fire.alphaDiff = value
    }

    // Alpha variation.
    Label {
        text: qsTr("Alpha variation")
    }
    SpinBox {
        value: Fire.alphaVariation
        maximumValue: 256
        stepSize: 1

        onValueChanged: Fire.alphaVariation = value
    }

    // N° of colors.
    Label {
        text: qsTr("N° of colors")
    }
    SpinBox {
        value: Fire.nColors
        maximumValue: 256
        stepSize: 1

        onValueChanged: Fire.nColors = value
    }
}
