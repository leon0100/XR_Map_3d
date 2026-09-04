import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts  1.12
import QtQuick.Dialogs  1.3
import Qt.labs.settings 1.1

import "../"
import AppXr 1.0


Item {
    id: isobathsSet

    width: toggleButton.width
    height: toggleButton.height
    z: 9999

    property bool isobathsDrawOpen:  false
    property bool bluetoothDrawOpen: false
    property int  isobathSize:  theme.screenSize * 0.35
    property int  layoutHeight: isobathSize * 0.1
    property var  targetPlot:   null
    property int  iconSize:     isobathSize * 0.05

    property int  currentCommPage: 0

    Connections {
        target: UdpManager
        function onSignalCancelUdpOn(isOn) {
            onOffControl.isOn = isOn
        }
    }

    // ------------------ 侧边按钮 -------------------
    ColumnLayout {
        id: toggleButton
        width: iconSize * 2
        spacing: 2

        Rectangle {
            id: isobathsToggleBtn
            width: iconSize * 1.6
            height: iconSize * 6.4
            color: "#879fc6"
            opacity: 0.75

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    isobathsDrawOpen = !isobathsDrawOpen
                    if(isobathsDrawOpen) {
                        bluetoothDrawOpen = false
                        bluetoothToggleBtn.color = "#879fc6"
                        parent.color = "#4a5f82"
                        isobathsToggleBtn.opacity = 0.95
                    }
                    else {
                        parent.color = "#879fc6"
                        isobathsToggleBtn.opacity = 0.75
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 2

                Image {
                    source: "qrc:/XR/contour.png"
                    Layout.preferredWidth: iconSize * 1.1
                    Layout.preferredHeight: iconSize * 1.1
                    Layout.alignment: Qt.AlignHCenter
                    fillMode: Image.PreserveAspectFit
                }

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: iconSize * 3.2
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("Isobaths")
                        color: "white"
                        font.pixelSize: iconSize
                        rotation: 90
                        transformOrigin: Item.Center
                    }
                }
            }
        }

        Rectangle {
            id: bluetoothToggleBtn
            width: iconSize * 1.6
            height: iconSize * 7.2
            color: "#879fc6"
            opacity: 0.75

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    bluetoothDrawOpen = !bluetoothDrawOpen
                    if(bluetoothDrawOpen) {
                        isobathsDrawOpen = false
                        isobathsToggleBtn.color = "#879fc6"
                        parent.color = "#4a5f82"
                        bluetoothToggleBtn.opacity = 0.95
                    }
                    else {
                        parent.color = "#879fc6"
                        bluetoothToggleBtn.opacity = 0.75
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 2

                Image {
                    source: "qrc:/icons/ui/file_settings.svg"
                    Layout.preferredWidth: iconSize * 1.1
                    Layout.preferredHeight: iconSize * 1.1
                    Layout.alignment: Qt.AlignHCenter
                    fillMode: Image.PreserveAspectFit
                }

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: iconSize * 4
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("Bathymetry")
                        color: "white"
                        font.pixelSize: iconSize
                        rotation: 90
                        transformOrigin: Item.Center
                    }
                }
            }
        }

    }


}
