import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

Item {
    id: root
    width: bleSize * 1.15
    height: bleSize * 0.8
    x: Screen.width - width - 5
    z: 99

    property int  bleSize:         Math.min(Screen.width, Screen.height) * 0.35
    property int  layoutHeight:    bleSize * 0.1
    property int  iconSize:        bleSize * 0.06
    property bool isShowDataPanel: false
    property bool isOn:            false

    onVisibleChanged: {
        BleManager.setBleLiveScanningVisible(visible)
    }

    // Connections {
    //     target: BleManager
    //     function onConnectedChanged(connected) {
    //         isOn = connected
    //         readControl.isReading  = connected
    //         root.isShowDataPanel = true
    //         // root.visible = false
    //     }
    // }

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

                    Rectangle {
                        id: keepBoatView
                        Layout.fillWidth: true
                        Layout.preferredHeight: layoutHeight
                        radius: layoutHeight * 0.2
                        color: "#f9f9fb"
                        border.color: "#b0b3b8"
                        border.width: 2

                        property bool checked: true

                        SequentialAnimation {
                           id: flashAnim1
                           running: false
                           loops: 1

                           ColorAnimation {
                               target: keepBoatView
                               property: "color"
                               to: "#9ecbff"
                               duration: 100
                           }
                           ColorAnimation {
                               target: keepBoatView
                               property: "color"
                               to: "#d6e6ff"
                               duration: 100
                           }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: iconSize * 0.5
                            anchors.rightMargin: iconSize * 0.5
                            spacing: iconSize * 0.5
                            anchors.verticalCenter: parent.verticalCenter

                            Rectangle {
                                width: iconSize * 1.1
                                height: iconSize * 1.1
                                radius: 5
                                border.color: "#b0b3b8"
                                border.width: 1
                                anchors.verticalCenter: parent.verticalCenter

                                Rectangle {
                                    anchors.centerIn: parent
                                    width: parent.width * 0.8
                                    height: parent.height * 0.8
                                    radius: parent.height * 0.4
                                    color: "#66E07A"
                                }
                            }

                            Text {
                                text: qsTr("Keep Boat in View")
                                font.pixelSize: iconSize
                                color: "black"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true

                            onClicked: {
                                flashAnim1.restart()

                                if(isOn) {
                                    core.location(1)
                                }
                            }

                            onEntered: parent.color = "#d6e6ff"
                            onExited:  parent.color = "#f9f9fb"
                        }
                    }


                    Rectangle {
                        id: showDataPanel
                        Layout.fillWidth: true
                        Layout.preferredHeight: layoutHeight
                        radius: layoutHeight * 0.2
                        color: "#f9f9fb"
                        border.color: "#b0b3b8"
                        border.width: 2

                        property bool checked: true

                        SequentialAnimation {
                           id: flashAnim2
                           running: false
                           loops: 1

                           ColorAnimation {
                               target: showDataPanel
                               property: "color"
                               to: "#9ecbff"
                               duration: 100
                           }
                           ColorAnimation {
                               target: showDataPanel
                               property: "color"
                               to: "#d6e6ff"
                               duration: 100
                           }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: iconSize * 0.5
                            anchors.rightMargin: iconSize * 0.5
                            spacing: iconSize * 0.6
                            anchors.verticalCenter: parent.verticalCenter

                            Rectangle {
                                width: iconSize * 1.1
                                height: iconSize * 1.1
                                radius: 5
                                border.color: "#b0b3b8"
                                border.width: 1
                                anchors.verticalCenter: parent.verticalCenter

                                Rectangle {
                                    anchors.centerIn: parent
                                    width: parent.width * 0.8
                                    height: parent.height * 0.8
                                    radius: parent.height * 0.4
                                    color: "#66E07A"
                                    visible: isShowDataPanel
                                }
                            }

                            Text {
                                text: qsTr("Show Data Panel")
                                font.pixelSize: iconSize
                                color: "black"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true

                            onClicked: {
                                flashAnim2.restart()
                                isShowDataPanel = !isShowDataPanel
                            }

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
                        spacing: 20

                        Rectangle {
                            id: switchControl
                            width:  layoutHeight * 2.2
                            height: layoutHeight
                            radius: layoutHeight * 0.3
                            color:  hovered ? (isOn ? "#36D85A" : "#D6E6FF") : (isOn?  "#66E07A" : "#D0D0D2")

                            property bool hovered: false

                            // 滑块
                            Rectangle {
                                id: slider
                                width:  layoutHeight * 0.9
                                height: layoutHeight * 0.9
                                radius: layoutHeight * 0.45
                                anchors.verticalCenter: parent.verticalCenter
                                x: isOn ? parent.width-width-2 : 2
                                color: "#FAFAFA"
                                scale: mouse1Area.pressed ? 0.9 : 1.0

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
                                font.pixelSize: iconSize * 0.8
                                font.bold: true
                                visible: isOn
                                Behavior on opacity { NumberAnimation { duration: 150 } }
                            }

                            Text {
                                anchors {
                                    right: parent.right
                                    rightMargin: 10
                                    verticalCenter: parent.verticalCenter
                                }
                                text: qsTr("OFF")
                                font.pixelSize: iconSize * 0.8
                                font.bold: true
                                visible: !isOn
                                Behavior on opacity { NumberAnimation { duration: 150 } }
                            }

                            MouseArea {
                                id: mouse1Area
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    if(isOn) {
                                        isOn = false;
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


                        Rectangle {
                            id: readControl
                            width:  layoutHeight * 2.2
                            height: layoutHeight
                            radius: layoutHeight * 0.3
                            color: hovered ? (readControl.isReading ? "#36D85A" : "#D6E6FF")
                                           : (readControl.isReading?  "#66E07A" : "#D0D0D2")
                            property bool isReading: false
                            property bool hovered: false

                            // 滑块
                            Rectangle {
                                id: readPause
                                width:  layoutHeight * 0.9
                                height: layoutHeight * 0.9
                                radius: layoutHeight * 0.45
                                anchors.verticalCenter: parent.verticalCenter
                                x: readControl.isReading ? parent.width-width-2 : 2
                                color: "#FAFAFA"
                                scale: mouse2Area.pressed ? 0.9 : 1.0

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
                                    leftMargin: 5
                                    verticalCenter: parent.verticalCenter
                                }
                                text: qsTr("Read")
                                font.pixelSize: iconSize * 0.8
                                font.bold: true
                                visible: readControl.isReading
                                Behavior on opacity { NumberAnimation { duration: 150 } }
                            }

                            Text {
                                anchors {
                                    right: parent.right
                                    rightMargin: 5
                                    verticalCenter: parent.verticalCenter
                                }
                                text: qsTr("Pause")
                                font.pixelSize: iconSize * 0.8
                                font.bold: true
                                visible: !readControl.isReading
                                Behavior on opacity { NumberAnimation { duration: 150 } }
                            }

                            MouseArea {
                                id: mouse2Area
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    readControl.isReading = !readControl.isReading
                                    BleManager.setDataReading(readControl.isReading)
                                }
                                onEntered: readControl.hovered = true
                                onExited:  readControl.hovered = false
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

                        readonly property bool noDevices: (modelData === qsTr("No Devices Found"))

                        border.width: noDevices ? 1 : (isOn ? 2 : 1)
                        border.color: noDevices ? "#ecf0f1" : (isOn ? "#3498db" : "#ecf0f1")
                        color: noDevices ? "white" : (isOn ? "#d6eaf8" : (mouseArea2.containsMouse ? "#d6e6ff" : "white"))

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
                                text: isOn ? qsTr("Connected") : qsTr("Disconnected")
                                color: isOn ? "#36D85A" : "#7f8c8d"
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
