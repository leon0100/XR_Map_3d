import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15
import QmlCommon 1.0



Rectangle {
    id: root
    anchors.fill: parent
    color: "transparent"
    visible: false
    z: 9999

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        preventStealing: true   // 阻止事件传递到下层

        onPressed: {
            flashAnim.restart()
        }

        onWheel: {
            flashAnim.restart()
        }

        onPressedChanged: {
            if (pressed) {
                flashAnim.restart()
            }
        }
    }


    property int dialogWidth: theme.screenSize * 0.40
    property int dialogHeight: theme.screenSize * 0.16
    property int iconSize: theme.iconSize


    Rectangle {
        id: dialogRoot
        anchors.centerIn: parent
        width:  dialogWidth
        height: dialogHeight
        radius: 10
        color: "#d6e6ff"



        ColumnLayout {
            anchors.fill: parent

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: iconSize * 2
                Layout.alignment: Qt.AlignTop
                color: "#a0c4ff"
                radius: 10

                /* ===== 遮挡矩形（盖掉下半圆角）===== */
                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: parent.radius
                    color: parent.color
                }

                Row {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 2

                    Image {
                        width: iconSize * 1.2
                        height: iconSize * 1.2
                        source: "qrc:/XR/icon_XR_map.png"
                        fillMode: Image.PreserveAspectFit
                    }

                    Text {
                        text: qsTr("Hint")
                        font.pixelSize: iconSize
                        color: "black"
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }


            Rectangle {
                Layout.fillWidth: true
                color: "transparent"
                implicitHeight: iconSize * 3

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: iconSize
                    spacing: 10

                    Image {
                        source: "qrc:/XR/warning.png"
                        Layout.preferredWidth: iconSize * 2
                        Layout.preferredHeight: iconSize * 2
                        fillMode: Image.PreserveAspectFit
                        Layout.alignment: Qt.AlignTop
                    }

                    Text {
                        id: messageText
                        Layout.fillWidth: true
                        font.bold: true
                        wrapMode: Text.WordWrap
                        font.pixelSize: iconSize
                        color: "black"
                    }
                }
            }


            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: iconSize * 3

                Row {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: iconSize
                    anchors.rightMargin: iconSize

                    Rectangle {
                        width: dialogWidth * 0.2
                        height: dialogHeight * 0.2
                        radius: 10
                        border.color: "#6b8fd6"
                        border.width: 1
                        color: yesBtn.hovered ? "#a0c4ff" : "#d6e6ff"

                        Button {
                            id: yesBtn
                            anchors.fill: parent
                            background: null
                            hoverEnabled: true
                            contentItem: Text {
                                text: qsTr("Yes")
                                font.pixelSize: iconSize
                                color: "black"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                root.visible = false
                                GetInterface.dialogYesNoBtn(true)
                            }
                        }
                    }

                    Rectangle {
                        width: dialogWidth * 0.2
                        height: dialogHeight * 0.2
                        radius: 10
                        border.color: "#6b8fd6"
                        border.width: 1
                        color: noBtn.hovered ? "#a0c4ff" : "#d6e6ff"

                        Button {
                            id: noBtn
                            anchors.fill: parent
                            background: null
                            hoverEnabled: true
                            contentItem: Text {
                                text: qsTr("No")
                                font.pixelSize: iconSize
                                color: "black"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            onClicked: {
                                root.visible = false
                                GetInterface.dialogYesNoBtn(false)
                            }
                        }
                    }

                }
            }

        }


        Rectangle {
            id: flashBorder
            anchors.fill: parent
            radius: dialogRoot.radius
            color: "transparent"
            border.width: 0
            border.color: "transparent"
        }

        SequentialAnimation {
            id: flashAnim
            running: false
            loops: 6

            ParallelAnimation {
                ColorAnimation {
                    target: flashBorder
                    property: "border.color"
                    to: "#FFFFFF"
                    duration: 70
                }

                NumberAnimation {
                    target: flashBorder
                    property: "border.width"
                    to: iconSize * 0.25
                    duration: 70
                }
            }

            ParallelAnimation {
                ColorAnimation {
                    target: flashBorder
                    property: "border.color"
                    to: "transparent"
                    duration: 70
                }

                NumberAnimation {
                    target: flashBorder
                    property: "border.width"
                    to: 0
                    duration: 70
                }
            }
        }

    }


    function flash() {
        flashAnim.restart()
    }

    function show(msg) {
        messageText.text = msg
        root.visible = true
    }


}























