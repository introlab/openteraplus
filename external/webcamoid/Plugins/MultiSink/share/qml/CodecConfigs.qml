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
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

ApplicationWindow {
    id: recAbout
    title: qsTr("Configure %1").arg(codecName)
    color: palette.window
    flags: Qt.Dialog
    modality: Qt.ApplicationModal
    width: 400
    height: 500

    property int outputIndex: 0
    property bool isCodec: true
    property string codecName: ""

    signal formatControlsChanged(variant controlValues)
    signal codecControlsChanged(int streamIndex, variant controlValues)

    SystemPalette {
        id: palette
    }

    Component {
        id: classCodecControl

        CodecControl {
        }
    }

    function updateOptions()
    {
        // Remove old controls.
        for(var i = clyCodecOptions.children.length - 1; i >= 0 ; i--)
            clyCodecOptions.children[i].destroy();

        var options = isCodec?
                    MultiSink.codecOptions(outputIndex):
                    MultiSink.formatOptions();
        var minimumLeftWidth = 0;
        var minimumRightWidth = 0;

        for (var i in options) {
            var codecOptions = classCodecControl.createObject(clyCodecOptions);
            codecOptions.controlParams = options[i];
            codecOptions.onControlChanged.connect(updateValues);

            if (codecOptions.leftWidth > minimumLeftWidth)
                minimumLeftWidth = codecOptions.leftWidth

            if (codecOptions.rightWidth > minimumRightWidth)
                minimumRightWidth = codecOptions.rightWidth
        }

        for (var i in clyCodecOptions.children) {
            clyCodecOptions.children[i].minimumLeftWidth = minimumLeftWidth;
            clyCodecOptions.children[i].minimumRightWidth = minimumRightWidth;
        }
    }

    function updateValues(controlName, value) {
        btnOk.controlValues[controlName] = value;
    }

    Component.onCompleted: updateOptions()
    onCodecNameChanged: updateOptions()

    ColumnLayout {
        anchors.fill: parent

        ScrollView {
            id: scrollControls
            Layout.fillHeight: true
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                id: clyCodecOptions
                width: scrollControls.viewport.width
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight

            TextField {
                id: optionFilter
                placeholderText: qsTr("Search option")
                Layout.fillWidth: true

                onTextChanged: {
                    for (var i in clyCodecOptions.children) {
                        var opt = clyCodecOptions.children[i];
                        opt.visible =
                                MultiSinkUtils.matches(text,
                                                       [opt.controlName,
                                                        opt.controlDescription]);
                    }
                }
            }
            Button {
                text: qsTr("Reset")
                iconName: "reset"
                iconSource: "image://icons/reset"
                onClicked: {
                    btnOk.controlValues = {};

                    if (isCodec)
                        MultiSink.resetCodecOptions(outputIndex);
                    else
                        MultiSink.resetFormatOptions();

                    optionFilter.text = "";
                    updateOptions();
                }
            }
            Button {
                text: qsTr("Cancel")
                iconName: "cancel"
                iconSource: "image://icons/cancel"
                onClicked: {
                    optionFilter.text = ""
                    recAbout.close()
                    btnOk.controlValues = {};
                }
            }
            Button {
                id: btnOk
                text: qsTr("Ok")
                iconName: "ok"
                iconSource: "image://icons/ok"

                property variant controlValues: ({})

                onClicked: {
                    if (isCodec)
                        codecControlsChanged(outputIndex, controlValues);
                    else
                        formatControlsChanged(controlValues);

                    controlValues = {};
                    optionFilter.text = ""
                    recAbout.close()
                }
            }
        }
    }
}
