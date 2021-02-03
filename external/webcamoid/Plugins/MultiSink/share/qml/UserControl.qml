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
    id: grdCameraControl
    columns: 3
    state: controlParams.length > 1? controlParams[1]: ""

    property variant controlParams: []
    property int value: controlParams.length > 6? controlParams[6]: 0
    property int minimumLeftWidth: 0
    property int minimumRightWidth: 0
    readonly property alias leftWidth: lblRange.width
    readonly property alias rightWidth: spbRange.width

    signal controlChanged(string controlName, int value)

    onValueChanged: {
        sldRange.value = value
        spbRange.value = value
        chkBool.checked = value !== 0
        cbxMenu.currentIndex = value
    }

    Label {
        id: lblRange
        text: controlParams.length > 0? controlParams[0]: ""
        Layout.minimumWidth: minimumLeftWidth
    }

    Slider {
        id: sldRange
        minimumValue: controlParams.length > 2? controlParams[2]: 0
        maximumValue: controlParams.length > 3? controlParams[3]: 0
        stepSize: controlParams.length > 4? controlParams[4]: 0
        value: grdCameraControl.value
        Layout.fillWidth: true
        visible: false

        onValueChanged: {
            if (visible) {
                spbRange.value = value
                grdCameraControl.controlChanged(controlParams.length > 0? controlParams[0]: "", value)
            }
        }
    }
    SpinBox {
        id: spbRange
        minimumValue: controlParams.length > 2? controlParams[2]: 0
        maximumValue: controlParams.length > 3? controlParams[3]: 0
        stepSize: controlParams.length > 4? controlParams[4]: 0
        value: sldRange.value
        Layout.minimumWidth: minimumRightWidth
        visible: false

        onValueChanged: {
            if (visible)
                sldRange.value = value
        }
    }

    CheckBox {
        id: chkBool
        checked: grdCameraControl.value !== 0
        Layout.columnSpan: 2
        Layout.fillWidth: true
        visible: false

        onCheckedChanged: {
            if (visible)
                grdCameraControl.controlChanged(controlParams.length > 0? controlParams[0]: "", checked? 1: 0)
        }
    }

    ComboBox {
        id: cbxMenu
        model: controlParams.length > 7? controlParams[7]: []
        currentIndex: grdCameraControl.value
        Layout.fillWidth: true
        Layout.columnSpan: 2
        visible: false

        onCurrentIndexChanged: {
            if (visible)
                grdCameraControl.controlChanged(controlParams.length > 0? controlParams[0]: "", currentIndex)
        }
    }

    states: [
        State {
            name: "integer"

            PropertyChanges {
                target: sldRange
                visible: true
            }
            PropertyChanges {
                target: spbRange
                visible: true
            }
        },
        State {
            name: "integer64"

            PropertyChanges {
                target: sldRange
                visible: true
            }
            PropertyChanges {
                target: spbRange
                visible: true
            }
        },
        State {
            name: "boolean"

            PropertyChanges {
                target: chkBool
                visible: true
            }
        },
        State {
            name: "menu"

            PropertyChanges {
                target: cbxMenu
                visible: true
            }
        }
    ]
}
