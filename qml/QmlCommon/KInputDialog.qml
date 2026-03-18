import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import KoggerCommon 1.0

Rectangle {
    id: inputDialog
    z: 9999
    visible: false
    anchors.fill: parent
    color: "#88000000"

    property string dialogTitle: qsTr("Input")
    property string message: ""
    property string defaultValue: ""
    property string placeholderText: ""
    property bool echoMode: false  // false: normal, true: password
    property int dialogWidth: 400
    property int dialogHeight: 200
    property string inputText: ""

    signal accepted(string text)
    signal rejected()

    function open() {
        inputText = defaultValue
        visible = true
    }

    function close() {
        visible = false
    }

    function getText() {
        return inputText
    }

    Rectangle {
        id: dialogBox
        width: dialogWidth
        height: dialogHeight
        anchors.centerIn: parent
        radius: 8
        color: theme.menuBackColor
        border.color: theme.controlBorderColor
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12

            // Title Bar
            RowLayout {
                Layout.fillWidth: true
                height: 32

                KText {
                    text: dialogTitle
                    font.bold: true
                    font.pixelSize: theme.iconSize + 2
                    color: theme.textColor
                }

                Item { Layout.fillWidth: true }

                CButton {
                    width: 32
                    height: 32
                    text: "×"
                    backColor: "transparent"
                    borderRadius: 4
                    onClicked: {
                        close()
                        rejected()
                    }
                }
            }

            // Divider
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: theme.controlBorderColor
            }

            // Message
            KText {
                text: message
                color: theme.textColor
                font.pixelSize: theme.iconSize
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                visible: message !== ""
            }

            // Input Field
            CTextField {
                id: inputField
                text: defaultValue
                placeholderText: placeholderText
                echoMode: inputDialog.echoMode ? TextInput.Password : TextInput.Normal
                Layout.fillWidth: true
                onTextChanged: {
                    inputText = text
                }
            }

            // Spacer
            Item { Layout.fillHeight: true }

            // Button Row
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
                height: 40
                spacing: 12

                CButton {
                    text: qsTr("Cancel")
                    backColor: theme.controlBackColor
                    onClicked: {
                        close()
                        rejected()
                    }
                }

                CButton {
                    text: qsTr("OK")
                    onClicked: {
                        inputText = inputField.text
                        close()
                        accepted(inputField.text)
                    }
                }
            }
        }
    }

    // Keyboard handling
    Keys.onEscapePressed: {
        close()
        rejected()
    }

    Keys.onEnterPressed: {
        inputText = inputField.text
        close()
        accepted(inputField.text)
    }
}
