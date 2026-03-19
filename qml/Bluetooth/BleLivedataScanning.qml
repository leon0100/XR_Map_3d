import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

Item {
    id: root
    width: bleSize * 1.1
    height: bleSize * 0.8
    z: 99

    property int bleSize: Math.min(Screen.width, Screen.height) * 0.35
    property int layoutHeight: bleSize * 0.1
    property int iconSize: bleSize * 0.06

    signal signalCheckBoxToggle(int checkBoxId, bool checked)

    onVisibleChanged: {
        BleManager.setBleLiveScanningVisible(visible)
    }

    Connections {
        target: BleManager
        function onConnectedChanged(connected) {
            switchControl.isOn = connected
        }
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#f5f7fa" }
            GradientStop { position: 1.0; color: "#c3cfe2" }
        }
    }

    ColumnLayout
    {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        RowLayout
        {
            anchors.margins: 10
            spacing: 10

            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredWidth: 2
                Layout.minimumWidth: 2

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 4

                    // 第一个复选框
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: layoutHeight
                        radius: layoutHeight * 0.2
                        color: "#f9f9fb"
                        border.color: "#b0b3b8"
                        border.width: 2

                        CheckBox {
                            id: check1
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: iconSize * 0.5
                            text: qsTr("Direction Arrow")
                            checked: true
                            font.pixelSize: iconSize
                            onCheckedChanged: root.signalCheckBoxToggle(0, checked)

                            indicator: Rectangle {
                                implicitWidth: iconSize * 1.1
                                implicitHeight: iconSize * 1.1
                                border.color: "#b0b3b8"
                                border.width: 1
                                radius: 5

                                Rectangle {
                                    anchors.centerIn: parent
                                    width: parent.width * 0.8
                                    height: parent.height * 0.8
                                    radius: parent.height * 0.4
                                    color: "#4CD964"
                                    visible: check1.checked
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.NoButton
                            onEntered: parent.color = "#d6e6ff"
                            onExited:  parent.color = "#f9f9fb"
                        }
                    }

                    // 第二个复选框
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: layoutHeight
                        radius: layoutHeight * 0.2
                        color: "#f9f9fb"
                        border.color: "#b0b3b8"
                        border.width: 2

                        CheckBox {
                            id: check2
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: iconSize * 0.5
                            text: qsTr("Keep Boat in View")
                            checked: true
                            font.pixelSize: iconSize
                            onCheckedChanged: root.signalCheckBoxToggle(0, checked)

                            indicator: Rectangle {
                                implicitWidth: iconSize * 1.1
                                implicitHeight: iconSize * 1.1
                                border.color: "#b0b3b8"
                                border.width: 1
                                radius: 5

                                Rectangle {
                                    anchors.centerIn: parent
                                    width: parent.width * 0.8
                                    height: parent.height * 0.8
                                    radius: parent.height * 0.4
                                    color: "#4CD964"
                                    visible: check2.checked
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.NoButton
                            onEntered: parent.color = "#d6e6ff"
                            onExited:  parent.color = "#f9f9fb"
                        }
                    }

                    // 第三个复选框
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: layoutHeight
                        radius: layoutHeight * 0.2
                        color: "#f9f9fb"
                        border.color: "#b0b3b8"
                        border.width: 2

                        CheckBox {
                            id: check3
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: iconSize * 0.5
                            text: qsTr("Show Data Panel")
                            checked: true
                            font.pixelSize: iconSize
                            onCheckedChanged: root.signalCheckBoxToggle(0, checked)

                            indicator: Rectangle {
                                implicitWidth: iconSize * 1.1
                                implicitHeight: iconSize * 1.1
                                border.color: "#b0b3b8"
                                border.width: 1
                                radius: 5

                                Rectangle {
                                    anchors.centerIn: parent
                                    width: parent.width * 0.8
                                    height: parent.height * 0.8
                                    radius: parent.height * 0.4
                                    color: "#4CD964"
                                    visible: check3.checked
                                }
                            }
                        }


                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.NoButton
                            onEntered: parent.color = "#d6e6ff"
                            onExited:  parent.color = "#f9f9fb"
                        }
                    }
                }
            }

            Item {
                id: rootItem
                Layout.fillWidth: true
                Layout.fillHeight: true

                Rectangle {
                    id: centerRect
                    color: "transparent"
                    width: layoutHeight * 2
                    anchors.centerIn: parent

                    ColumnLayout {
                        id: mainCol
                        anchors.centerIn: parent
                        spacing: 2

                        Rectangle {
                            width: layoutHeight * 2
                            height: iconSize * 1.1
                            color: "transparent"
                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Bluetooth")
                                font.pixelSize: iconSize * 0.9
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        Rectangle {
                            id: switchControl
                            width:  layoutHeight * 2.2
                            height: layoutHeight
                            radius: layoutHeight * 0.3
                            color: hovered ? (switchControl.isOn ? "#4CD964" : "#D6E6FF")
                                           : (switchControl.isOn? "#4CD964" : "#E9E9EA")
                            property bool isOn: false
                            property bool hovered: false

                            // 滑块
                            Rectangle {
                                id: slider
                                width: layoutHeight
                                height: layoutHeight
                                radius: layoutHeight * 0.5
                                anchors.verticalCenter: parent.verticalCenter
                                x: switchControl.isOn ? parent.width-width-2 : 2
                                color: "#FAFAFA"
                                scale: mouseArea.pressed ? 0.9 : 1.0

                                Behavior on x {
                                    NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                                }
                                Behavior on scale {
                                    NumberAnimation { duration: 100 }
                                }
                            }

                            Text {
                                anchors {
                                    left: parent.left
                                    leftMargin: 10
                                    verticalCenter: parent.verticalCenter
                                }
                                text: qsTr("ON")
                                font.pixelSize: iconSize
                                font.bold: true
                                opacity: switchControl.isOn ? 1 : 0
                                Behavior on opacity { NumberAnimation { duration: 150 } }
                            }

                            Text {
                                anchors {
                                    right: parent.right
                                    rightMargin: 10
                                    verticalCenter: parent.verticalCenter
                                }
                                text: qsTr("OFF")
                                font.pixelSize: iconSize
                                font.bold: true
                                opacity: switchControl.isOn ? 0 : 1
                                Behavior on opacity { NumberAnimation { duration: 150 } }
                            }

                            MouseArea {
                                id: mouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    if(switchControl.isOn) {
                                        switchControl.isOn = false;
                                        BleManager.operateBleOnOff(false)
                                    } else {
                                        BleManager.operateBleOnOff(true)
                                    }

                                }
                                onEntered: switchControl.hovered = true
                                onExited: switchControl.hovered = false
                            }


                            Behavior on color {
                                ColorAnimation { duration: 200 }
                            }
                        }
                    }
                }
            }

        }


        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 3; color: "#7f8c8d" }


        ColumnLayout
        {
            anchors.margins: 15
            spacing: 1

            Text {
                anchors.margins: 5
                font.pixelSize: iconSize * 0.9
                text: qsTr("Toslon BLE Devices List:")
                verticalAlignment: Text.AlignVCenter
            }

            // 设备列表
            GroupBox {
                Layout.fillWidth: true
                Layout.preferredHeight: layoutHeight * 3
                background: Rectangle {
                    color: "#F5F5F5"
                    radius: layoutHeight * 0.3
                    border.color: "#bdc3c7"
                }

                ListView {
                    id: deviceList
                    anchors.fill: parent
                    anchors.margins: 5
                    model: BleManager ? BleManager.devices : ""
                    clip: true

                    delegate: Rectangle
                    {
                        width: deviceList.width
                        height: layoutHeight * 0.8
                        radius: 4

                        readonly property bool noDevices: (modelData === "No Devices Found")

                        border.width: noDevices ? 1 : (switchControl.isOn ? 2 : 1)
                        border.color: noDevices ? "#ecf0f1" : (switchControl.isOn ? "#3498db" : "#ecf0f1")
                        color: noDevices ? "white" : (switchControl.isOn ? "#d6eaf8" : (mouseArea2.containsMouse ? "#d6e6ff" : "white"))

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 15
                            anchors.rightMargin: 15

                            Text {
                                Layout.fillWidth: true
                                text: modelData
                                color: "#2c3e50"
                                font.pixelSize: iconSize * 0.9
                                verticalAlignment: Text.AlignVCenter
                            }
                            Text {
                                visible: !noDevices
                                text: switchControl.isOn ? "Connected" : "Disconnected"
                                color: switchControl.isOn ? "#27ae60" : "#7f8c8d"
                                font.pixelSize: iconSize * 0.6
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        MouseArea {
                            id: mouseArea2
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked:{
                                if(!noDevices && BleManager){
                                    BleManager.connectToDevice(index)
                                }
                            }
                        }
                    }
                }
            }

        }

    }

}
