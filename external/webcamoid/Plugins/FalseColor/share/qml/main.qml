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

ColumnLayout {
    function fromRgba(rgba)
    {
        var r = ((rgba >> 16) & 0xff)
        var g = ((rgba >> 8) & 0xff)
        var b = (rgba & 0xff)

        return [r, g, b]
    }

    function toRgba(color)
    {
        var a = 0xff << 24
        var r = color[0] << 16
        var g = color[1] << 8
        var b = color[2]

        return a | r | g | b
    }

    function tableFromStr(str)
    {
        if (str.length < 1)
            return []

        var colorTable = JSON.parse(str)
        var table = []

        for (var color in colorTable)
            table.push(toRgba(colorTable[color]))

        return table
    }

    function tableToStr(table)
    {
        var colorTable = []

        for (var color in table)
            colorTable.push(fromRgba(table[color]))

        return JSON.stringify(colorTable, null, 4)
    }

    SystemPalette {
        id: palette
    }

    // Color table.
    Label {
        text: qsTr("Color table")
    }
    Rectangle {
        height: 400
        Layout.fillWidth: true
        color: palette.base

        ScrollView {
            anchors.fill: parent

            TextEdit {
                text: tableToStr(FalseColor.table)
                cursorVisible: true
                wrapMode: TextEdit.Wrap
                color: palette.text

                onTextChanged: FalseColor.table = tableFromStr(text)
            }
        }
    }

    // Soft gradient.
    CheckBox {
        text: qsTr("Soft")
        checked: FalseColor.soft

        onCheckedChanged: FalseColor.soft = checked
    }
}
