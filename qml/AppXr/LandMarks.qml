import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


Rectangle {
    id: root
    width:  landSize * 1.1
    height: landSize * 0.8
    radius: 10
    z: 10000
    visible: renderer.screetShot.landMarkMode


    property int landSize: theme.screenSize * 0.3
    property int iconSize: landSize * 0.05
    property int boxSize:  iconSize * 2


    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#f5f7fa" }
            GradientStop { position: 1.0; color: "#c3cfe2" }
        }
    }


    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        Rectangle {
            id: titleBar
            width: parent.width
            height: landSize * 0.1
            color: "#c3cfe2"

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr(" Spot Information")
                font.pixelSize: iconSize - 1
                font.bold: true
                color: "#4a4a4a"
            }
        }


        // Name
        RowLayout {
            // Layout.fillWidth: true
            spacing: 10
            Label {
                text: qsTr("Name")
                font.pixelSize: iconSize
                Layout.minimumWidth: boxSize * 3
            }
            TextField {
                id: nameField
                Layout.fillWidth: true
                implicitHeight: boxSize
                text: renderer.screetShot.spotName
                font.pixelSize: iconSize + 1
                onTextChanged: renderer.screetShot.setSpotName2(text)

                selectByMouse: true
                Keys.onPressed: function(event) {
                    if (event.key === Qt.Key_V && event.modifiers & Qt.ControlModifier) {
                        Qt.callLater(function() { renderer.screetShot.spotName = nameField.text })
                    }
                }
                Menu {
                    id: spotNameMenu
                    implicitWidth: boxSize * 2
                    implicitHeight: boxSize
                    MenuItem {
                        text: qsTr("Paste")
                        enabled: nameField.canPaste
                        onTriggered: {
                            nameField.paste()
                            renderer.screetShot.spotName = nameField.text
                        }
                    }
                }

                onPressed: function(event) {
                    if (event.button === Qt.RightButton) {
                        forceActiveFocus()
                        spotNameMenu.popup()
                        event.accepted = true
                    } else if (event.button === Qt.LeftButton) {
                        spotNameMenu.close()
                    }
                }
            }

        }

        // Latitude
        RowLayout {
            // Layout.fillWidth: true
            spacing: 10

            Label {
                text: qsTr("Latitude")
                font.pixelSize: iconSize
                Layout.minimumWidth: boxSize * 3
            }
            TextField {
                id: latiField
                Layout.fillWidth: true
                implicitHeight: boxSize
                font.pixelSize: iconSize
                text: renderer.screetShot.spotLatitude
                onTextChanged: renderer.screetShot.setSpotLatitude2(text)
            }
        }

        // Longitude
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Label {
                text: qsTr("Longitude")
                font.pixelSize: iconSize
                Layout.minimumWidth: boxSize * 3
            }
            TextField {
                id: longiField
                Layout.fillWidth: true
                implicitHeight: boxSize
                font.pixelSize: iconSize
                text: renderer.screetShot.spotLongitude
                onTextChanged: renderer.screetShot.setSpotLongitude2(text)
            }
        }


        RowLayout {
            Layout.fillWidth: true
            spacing: boxSize

            Item { Layout.fillWidth: true }

            Button {
                id: saveBtn
                text: qsTr("Save")
                Layout.preferredWidth: boxSize * 3
                font.pixelSize: iconSize

                background: Rectangle {
                    radius: 6
                    color: saveBtn.hovered ? "#d0e8ff" : "#d9edff"
                    border.color: "#9bbfe9"
                    border.width: 1
                }

                onClicked: {
                    renderer.screetShot.saveClicked()
                }
            }

            Button {
                id: cancelBtn
                text: qsTr("Cancel")
                Layout.preferredWidth: boxSize * 3
                font.pixelSize: iconSize

                background: Rectangle {
                    radius: 6
                    color: cancelBtn.hovered ? "#d0e8ff" : "#d9edff"
                    border.color: "#9bbfe9"
                    border.width: 1
                }

                onClicked: {
                    renderer.screetShot.cancelClicked()
                    renderer.screetShot.landMarkMode = false
                }
            }

            Item { Layout.fillWidth: true }
        }

    }


}

