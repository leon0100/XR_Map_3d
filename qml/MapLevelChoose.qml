import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15


Rectangle {
    id: root
    visible: renderer.screetShot.isMapLevelChooseVisible
    width:  mapLevelSize * 1.25
    height: mapLevelSize * 0.9
    x: Screen.width * 0.5  - width * 0.5
    y: Screen.height * 0.5 - height * 0.8
    z: 9999
    radius: 5

    property int  mapLevelSize: theme.screenSize * 0.45
    property int  iconSize:     theme.iconSize * 0.9
    property int  currentLevel: -1

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#f5f7fa" }
            GradientStop { position: 1.0; color: "#c3cfe2" }
        }
    }

    ListModel {
        id: tableModel
    }

    Component.onCompleted: {
        initData()
    }


    function initData() {
        tableModel.clear()
        for (var i = 0; i < 6; ++i) {
            tableModel.append({
                level: 13 + i,
                size: "0x0",
                theoreticalSize: "0MB",
                downloadTime: "0s",
                enabled: true
            })
        }

        currentLevel = -1;
    }
    Connections {
        target: renderer.screetShot

        function onUpdateTableRowData(row, size, theoreticalSize, downloadTime) {
            if (row < 0 || row >= tableModel.count) return

            tableModel.setProperty(row, "size", size)

            if (!tableModel.get(row).enabled) {
                tableModel.setProperty(row, "theoreticalSize", "0")
                tableModel.setProperty(row, "downloadTime", qsTr("Not Available"))
            }
            else {
                tableModel.setProperty(row, "theoreticalSize", theoreticalSize)
                tableModel.setProperty(row, "downloadTime", downloadTime)
            }
        }
    }


    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: iconSize

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: iconSize * 2.2
            color: "transparent"

            Text {
                anchors.centerIn: parent
                text: qsTr("Maplevel Choose Bar")
                font.bold: true
                font.pixelSize: iconSize * 1.1
                color: "#2b2b2b"
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: iconSize * 1.4
            color: "#eaeaea"

            RowLayout {
                anchors.fill: parent
                anchors.verticalCenter: parent.verticalCenter

                Label {
                    Layout.preferredWidth: mapLevelSize * 0.25
                    text: qsTr("Map Level")
                    font.bold: true
                    font.pixelSize: iconSize
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }

                Label {
                    Layout.preferredWidth: mapLevelSize * 0.3
                    text: qsTr("Size")
                    font.bold: true
                    font.pixelSize: iconSize
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }

                Label {
                    Layout.preferredWidth: mapLevelSize * 0.3
                    text: qsTr("Theoretical Size")
                    font.bold: true
                    font.pixelSize: iconSize
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Estimated Time")
                    font.bold: true
                    font.pixelSize: iconSize
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }


        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            border.color: "#d0d0d0"
            color: "white"

            ListView {
                id: listView
                anchors.fill: parent
                clip: true
                model: tableModel

                delegate: Rectangle {
                    width: listView.width
                    height: iconSize * 2.5
                    color: "transparent"

                    required property int index
                    required property int level
                    required property string size
                    required property string theoreticalSize
                    required property string downloadTime
                    required property bool enabled

                    RowLayout {
                        anchors.fill: parent

                        Item {
                            Layout.preferredWidth: mapLevelSize * 0.25
                            Layout.fillHeight: true

                            RadioButton {
                                anchors.centerIn: parent
                                enabled: parent.parent.enabled
                                checked: root.currentLevel === level

                                indicator: Rectangle {
                                    width: iconSize
                                    height: iconSize
                                    x: 0
                                    y: (parent.height - height) * 0.5
                                    radius: width * 0.5
                                    border.color: "#666666"
                                    border.width: 2
                                    color: "white"

                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: parent.width * 0.6
                                        height: width
                                        radius: width * 0.5
                                        color: "#66E07A"
                                        visible: parent.parent.checked
                                    }
                                }

                                contentItem: Text {
                                    text: "Level " + level
                                    font.pixelSize: iconSize
                                    color: "black"
                                    verticalAlignment: Text.AlignVCenter
                                    leftPadding: iconSize * 1.5
                                }

                                onClicked: root.currentLevel = level
                            }

                        }

                        Label {
                            Layout.preferredWidth: mapLevelSize * 0.3
                            text: size
                            font.pixelSize: iconSize
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        Label {
                            Layout.preferredWidth: mapLevelSize * 0.3
                            text: theoreticalSize
                            font.pixelSize: iconSize
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        Label {
                            Layout.fillWidth: true
                            text: downloadTime
                            font.pixelSize: iconSize
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }
        }


        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: iconSize * 2

            Row {
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter:  parent.horizontalCenter
                spacing: iconSize * 4

                Rectangle {
                    width: iconSize * 5
                    height: iconSize * 2
                    radius: 8
                    border.color: "#6b8fd6"
                    border.width: 1
                    color: okBtn.hovered ? "#a0c4ff" : "#d6e6ff"

                    Button {
                        id: okBtn
                        anchors.fill: parent
                        background: null
                        hoverEnabled: true
                        contentItem: Text {
                            text: qsTr("Confirm")
                            font.pixelSize: iconSize
                            color: "black"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            renderer.screetShot.setTargetMapLevel(root.currentLevel)
                            renderer.screetShot.isMapLevelChooseVisible = false
                            initData()
                        }
                    }
                }

                Rectangle {
                    width: iconSize * 5
                    height: iconSize * 2
                    radius: 8
                    border.color: "#6b8fd6"
                    border.width: 1
                    color: noBtn.hovered ? "#a0c4ff" : "#d6e6ff"

                    Button {
                        id: noBtn
                        anchors.fill: parent
                        background: null
                        hoverEnabled: true
                        contentItem: Text {
                            text: qsTr("Cancel")
                            font.pixelSize: iconSize
                            color: "black"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            renderer.screetShot.isMapLevelChooseVisible = false
                            initData()
                        }
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.maximumHeight: 1
        }



    }

}
