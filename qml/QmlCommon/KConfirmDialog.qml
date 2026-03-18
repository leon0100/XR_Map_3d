import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import KoggerCommon 1.0

Rectangle {
    id: confirmDialog
    z: 9999
    visible: false
    anchors.fill: parent
    color: "#88000000"

    property string title: qsTr("Confirm")
    property string message: ""
    property string confirmText: qsTr("Confirm")
    property string cancelText: qsTr("Cancel")
    property bool isDanger: false
    property int dialogWidth: 400
    property int dialogHeight: 180

    signal confirmed()
    signal canceled()

    function open(msg, dangerMode) {
        message = msg
        if (dangerMode !== undefined) isDanger = dangerMode
        visible = true
    }

    function close() {
        visible = false
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
            anchors.margins: 20
            spacing: 16

            // Title
            KText {
                text: title
                font.bold: true
                font.pixelSize: theme.iconSize + 4
                color: isDanger ? "#ff4444" : theme.textColor
            }

            // Message
            KText {
                text: message
                color: theme.textColor
                font.pixelSize: theme.iconSize
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            // Button Row
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
                height: 40
                spacing: 12

                CButton {
                    text: cancelText
                    backColor: theme.controlBackColor
                    onClicked: {
                        close()
                        canceled()
                    }
                }

                CButton {
                    text: confirmText
                    backColor: isDanger ? "#cc0000" : theme.controlSolidBackColor
                    onClicked: {
                        close()
                        confirmed()
                    }
                }
            }
        }
    }

    Keys.onEscapePressed: {
        close()
        canceled()
    }
}
