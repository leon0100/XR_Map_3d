import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15
import QmlCommon 1.0



Rectangle {
    id: root
    z: 990
    width: 450
    height: 140
    radius: 10
    color: "#d6e6ff"
    anchors.centerIn: parent
    visible: false


    Rectangle {
        id: flashBorder
        anchors.fill: parent
        radius: root.radius
        color: "transparent"
        border.width: 0
        border.color: "transparent"
        z: 1000
        visible: true
    }


    ColumnLayout {
        anchors.fill: parent

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
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
                    width: 20
                    height: 20
                    source: "qrc:/exe/icon-XR-map.png"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    text: qsTr("Hint")
                    font.pixelSize: 14
                    color: "black"
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }


        Rectangle {
            Layout.fillWidth: true
            color: "transparent"
            implicitHeight: 50

            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                Image {
                    source: "qrc:/sonar/warning.png"
                    Layout.preferredWidth: 27
                    Layout.preferredHeight: 27
                    fillMode: Image.PreserveAspectFit
                    Layout.alignment: Qt.AlignTop
                }

                Text {
                    id: messageText
                    Layout.fillWidth: true
                    font.bold: true
                    wrapMode: Text.WordWrap
                    font.pixelSize: 18
                    color: "black"
                }
            }
        }




            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 40

                Row {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 12
                    anchors.rightMargin: 20

                    Rectangle {
                        width: 70
                        height: 25
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
                                font.pixelSize: 16
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
                        width: 70
                        height: 25
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
                                font.pixelSize: 16
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
                to: 3
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


    function flash() {
        flashAnim.restart()
    }

    function show(msg) {
        messageText.text = msg
        visible = true
    }
}

