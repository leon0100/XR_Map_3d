import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    width: 600
    height: 500

    signal signalCheckBoxToggle(int checkBoxId, bool checked)
    signal signalBleOnOff(bool on)

    function slotSetBleConnected(connected) {
        switchControl.isOn = connected
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
                        Layout.preferredHeight: 50
                        radius: 6
                        color: "#f9f9fb"
                        border.color: "#b0b3b8"
                        border.width: 1

                        CheckBox {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 6
                            text: qsTr("Direction Arrow")
                            checked: true
                            font.pixelSize: 20
                            onCheckedChanged: root.signalCheckBoxToggle(0,checked)

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
                        Layout.preferredHeight: 50
                        radius: 6
                        color: "#f9f9fb"
                        border.color: "#b0b3b8"
                        border.width: 1

                        CheckBox {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 20
                            text: qsTr("Keep Boat in View")
                            checked: true
                            font.pixelSize: 16
                            onCheckedChanged: root.signalCheckBoxToggle(1,checked)
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
                        Layout.preferredHeight: 50
                        radius: 6
                        color: "#f9f9fb"
                        border.color: "#b0b3b8"
                        border.width: 1


                        CheckBox {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 6
                            text: qsTr("Show Data Panel")
                            checked: true
                            font.pixelSize: 20
                            onCheckedChanged: root.signalCheckBoxToggle(2,checked)
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
                    width: 70
                    height: mainCol.implicitHeight
                    anchors.centerIn: parent


                    ColumnLayout {
                        id: mainCol
                        anchors.centerIn: parent
                        spacing: 2

                        Rectangle {
                            width: 100
                            height: 30
                            color: "transparent"
                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Bluetooth")
                                font.pixelSize: 20
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        Rectangle {
                            id: switchControl
                            width: 100
                            height: 50
                            radius: 10
                            color: hovered ? (switchControl.isOn ? "#4CD964" : "#D6E6FF")
                                           : (switchControl.isOn ? "#4CD964" : "#E9E9EA")
                            property bool isOn: false
                            property bool hovered: false

                            // 滑块
                            Rectangle {
                                id: slider
                                width: 64
                                height: 64
                                radius: 32
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
                                font.pixelSize: 20
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
                                font.pixelSize: 20
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
                                        root.signalBleOnOff(false);
                                    }
                                    else {
                                        root.signalBleOnOff(true);
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


        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 2; color: "#7f8c8d" }


        ColumnLayout
        {
            anchors.margins: 15
            spacing: 1

            Text {
                anchors.margins: 5
                font.pixelSize: 20
                text: qsTr("Toslon BLE Devices List:")
                verticalAlignment: Text.AlignVCenter
            }

            // 设备列表
            GroupBox {
                Layout.fillWidth: true
                Layout.preferredHeight: 150
                background: Rectangle {
                    color: "#F5F5F5"
                    radius: 8
                    border.color: "#bdc3c7"
                }

                ListView {
                    id: deviceList
                    anchors.fill: parent
                    anchors.margins: 5
                    model: BleManager ? BleManager.devices : ""
                    // model: typeof BleManager !== 'undefined' && BleManager ? BleManager.devices : []
                    clip: true

                    delegate: Rectangle
                    {
                        width: deviceList.width
                        height: 30
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
                                font.pixelSize: 20
                                verticalAlignment: Text.AlignVCenter
                            }
                            Text {
                                visible: !noDevices
                                text: switchControl.isOn ? "Connected" : "Disconnected"
                                color: switchControl.isOn ? "#27ae60" : "#7f8c8d"
                                font.pixelSize: 20
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
