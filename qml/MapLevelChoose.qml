import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15


Rectangle {
    id: root
    visible: renderer.screetShot.isMapLevelChooseVisible

    width:  mapLevelSize * 1.25
    height: mapLevelSize
    x: Screen.width * 0.5  - width * 0.5
    y: Screen.height * 0.5 - height * 0.8
    z: 9999
    radius: 5


    property int  mapLevelSize: theme.screenSize * 0.45
    property int  iconSize:     theme.iconSize * 0.9
    property int  currentLevel: -1


    signal chooseMapLevel(int level)

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
        anchors.margins: 12
        spacing: 10

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
            height: 40
            color: "#eaeaea"
            radius: 4

            RowLayout {
                anchors.fill: parent

                Label {
                    Layout.preferredWidth: 140
                    text: qsTr("Map Level")
                    font.bold: true
                    font.pixelSize: iconSize
                    horizontalAlignment: Text.AlignHCenter
                }

                Label {
                    Layout.preferredWidth: 150
                    text: qsTr("Size")
                    font.bold: true
                    font.pixelSize: iconSize
                    horizontalAlignment: Text.AlignHCenter
                }

                Label {
                    Layout.preferredWidth: 150
                    text: qsTr("Theoretical Size")
                    font.bold: true
                    font.pixelSize: iconSize
                    horizontalAlignment: Text.AlignHCenter
                }

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Estimated Time")
                    font.bold: true
                    font.pixelSize: iconSize
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
                    height: 42

                    color: enabled ? "white" : "#d0d0d0"

                    border.width: 1
                    border.color: "#eeeeee"

                    required property int index
                    required property int level
                    required property string size
                    required property string theoreticalSize
                    required property string downloadTime
                    required property bool enabled

                    RowLayout {
                        anchors.fill: parent

                        Item {
                            Layout.preferredWidth: 140
                            Layout.fillHeight: true

                            RadioButton {
                                anchors.centerIn: parent
                                enabled: parent.parent.enabled
                                checked: root.currentLevel === level
                                text: "Level " + level
                                font.pixelSize: iconSize

                                onClicked: {
                                    root.currentLevel = level
                                }
                            }
                        }

                        Label {
                            Layout.preferredWidth: 150
                            text: size
                            font.pixelSize: iconSize
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        Label {
                            Layout.preferredWidth: 150
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

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 10

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("OK")
                width: 100
                height: 36
                onClicked: {
                    renderer.screetShot.setTargetMapLevel(root.currentLevel)
                    renderer.screetShot.isMapLevelChooseVisible = false
                    initData()
                }
            }

            Item {
                width: 60
            }

            Button {
                text: qsTr("Cancel")
                width: 100
                height: 36

                onClicked: {
                    renderer.screetShot.isMapLevelChooseVisible = false
                    initData()
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

}
