import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.2



Item
{
    id: root
    width: 500
    height: 450

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#f5f7fa" }
            GradientStop { position: 1.0; color: "#c3cfe2" }
        }
    }


    ColumnLayout {
        anchors {
           top: parent.top
           left: parent.left
           right: parent.right
           bottom: parent.bottom
           margins: 5
        }
        spacing: 8


        //1、串口设置
        GroupBox
        {
            font.bold: true
            font.pixelSize: 16
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15

                RowLayout {
                    id: rowSettings
                    Layout.fillWidth: true
                    spacing: 40
                    RowLayout {
                        id: rowPort
                        Layout.fillWidth: true
                        spacing: 15

                        Label {
                            text: qsTr("Port")
                            color: "black"
                            font.pixelSize: 15
                            Layout.alignment: Qt.AlignLeft
                            Layout.minimumWidth: 15
                        }
                        ComboBox {
                            id: portCombo
                            model: serialPort.availablePorts
                            font.pixelSize: 15
                        }
                    }

                    RowLayout {
                        id: rowBaud
                        Layout.fillWidth: true
                        spacing: 15

                        Label {
                            text: qsTr("Baud Rate")
                            color: "black"
                            font.pixelSize: 15
                            Layout.alignment: Qt.AlignLeft
                            Layout.minimumWidth: 40
                        }
                        ComboBox {
                            id: baudCombo
                            model: [ "19200", "921600", "57600", "115200", "230400"]
                            // currentIndex: 2
                            font.pixelSize: 15
                        }
                    }
                }


                RowLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 10
                    spacing:45

                    Item {
                        Layout.fillWidth: true
                    }
                    Button {
                       id: refreshBtn
                       text: qsTr("Refresh")
                       font.pixelSize: 15
                       Layout.preferredWidth: 90
                       Layout.preferredHeight: 30
                       hoverEnabled: true
                       onClicked: serialPort.scanPorts()
                       background: Rectangle {
                          color: refreshBtn.hovered ? "#6f8fc0" : "#b3c7e5"
                          radius: 4
                          MouseArea {
                              anchors.fill: parent
                              cursorShape: Qt.PointingHandCursor
                              onClicked: refreshBtn.clicked()
                          }
                       }
                    }

                    Button {
                        id: connectBtn
                        text: serialPort.connected ? qsTr("Disconnect") : qsTr("Connect")
                        font.pixelSize: 15
                        Layout.preferredWidth: 90
                        Layout.preferredHeight: 30
                        hoverEnabled: true
                        onClicked: serialPort.toggleConnection(portCombo.currentText, baudCombo.currentText)
                        background: Rectangle {
                            color: serialPort.connected ? "#00fa9a" : (connectBtn.hovered ? "#6f8fc0" : "#b3c7e5")
                            radius: 4
                            MouseArea {
                               anchors.fill: parent
                               cursorShape: Qt.PointingHandCursor
                               onClicked: connectBtn.clicked()
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                }
            }
        }



        // 2、接收设置区域
        GroupBox
        {
            font.bold: true
            font.pixelSize: 16
            Layout.fillWidth: true

            FileDialog {
                id: fileDialog
                title: qsTr("Select Save Path")
                selectExisting: false
                nameFilters: ["File (*.txt)", "All Files (*)"]
                onAccepted: {
                    filePathField.text = fileDialog.fileUrl.toString().replace("file:///", "")
                    serialPort.saveFilePath = filePathField.text
                }
            }

            RowLayout {
                anchors.fill: parent
                spacing: 8

                TextField {
                    id: filePathField
                    placeholderText: qsTr("Select Save Path......")
                    Layout.fillWidth: true
                    text: serialPort.saveFilePath
                    onTextChanged: serialPort.saveFilePath = text
                }

                Button {
                    id: openPath
                    text: qsTr("Browse...")
                    font.pixelSize: 16
                    width:70
                    hoverEnabled: true
                    background: Rectangle {
                        color: openPath.hovered ? "#6f8fc0" : "#b3c7e5"
                        radius: 5
                        MouseArea {
                           anchors.fill: parent
                           cursorShape: Qt.PointingHandCursor
                           onClicked: fileDialog.open()
                        }
                    }
                }

                Button {
                    id: exportBtn
                    text: "Save"
                    font.pixelSize: 16
                    width: 70
                    hoverEnabled: true
                    background: Rectangle {
                        color: exportBtn.hovered ? "#6f8fc0" : "#b3c7e5"
                        radius: 5
                        MouseArea {
                           anchors.fill: parent
                           cursorShape: Qt.PointingHandCursor
                           onClicked: serialPort.saveSerialPortData()
                        }
                    }
                }
            }


        }



        // 3、数据接收区域
        GroupBox {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 5
                anchors.margins: 8

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: qsTr("Recevied Data")
                        font.bold: true
                        font.pixelSize: 16
                        Layout.alignment: Qt.AlignVCenter
                    }

                    Item { Layout.fillWidth: true }

                    CheckBox {
                        text: "Hex Display"
                        font.pixelSize: 14
                        checked: false
                        contentItem: Text {
                            text: parent.text
                            color: "black"
                            font: parent.font
                            leftPadding: parent.indicator.width + parent.spacing
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    Button {
                        id: clearBtn
                        text: "Clear"
                        font.pixelSize: 14
                        Layout.preferredWidth: 60
                        hoverEnabled: true

                        background: Rectangle {
                           color: clearBtn.hovered ?  "#6f8fc0" : "#b3c7e5"
                           radius: 5
                           MouseArea {
                              anchors.fill: parent
                              cursorShape: Qt.PointingHandCursor
                              onClicked: {
                                receivedData.text = ""
                                serialPort.clearBuffer()
                              }
                           }
                        }
                    }     
                }


                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                    clip: true

                    TextArea {
                        id: receivedData
                        implicitWidth: parent.width
                        wrapMode: TextArea.Wrap
                        text: qsTr("Waiting to receive serial port data...\n")
                        font.pixelSize: 14
                        font.family: "Consolas, Monaco, Monospace"
                        color: "#e0e0e0"
                        readOnly: true
                        selectByMouse: true
                        background: Rectangle {
                           color: "#363636"
                           radius: 5
                        }
                    }
                }
            }

            Connections {
               target: serialPort
               function onDataReceived(data) {
                    receivedData.append(data)
                    receivedData.cursorPosition = receivedData.length
                    receivedData.moveCursorSelection(receivedData.length)
                }
            }

        }

    }

}
