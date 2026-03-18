import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import KoggerCommon 1.0

Rectangle {
    id: progressDialog
    z: 9999
    visible: false
    anchors.fill: parent
    color: "#88000000"

    property string title: qsTr("Processing...")
    property string statusText: ""
    property real progress: 0.0
    property bool indeterminate: true
    property bool showProgressBar: true
    property bool showCancelButton: true
    property int dialogWidth: 400
    property int dialogHeight: 180

    signal cancelRequested()

    function open() {
        visible = true
    }

    function close() {
        visible = false
    }

    function setProgress(value) {
        progress = value
    }

    function setStatus(text) {
        statusText = text
    }

    function setIndeterminate(indet) {
        indeterminate = indet
    }

    function setTitle(text) {
        title = text
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
                color: theme.textColor
            }

            // Status Text
            KText {
                id: statusLabel
                text: statusText
                color: theme.textColor
                font.pixelSize: theme.iconSize
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            // Spacer
            Item { Layout.fillHeight: true }

            // Progress Bar
            ProgressBar {
                id: progressBar
                Layout.fillWidth: true
                value: progress
                indeterminate: indeterminate
                visible: showProgressBar
            }

            // Cancel Button
            CButton {
                id: cancelButton
                text: qsTr("Cancel")
                backColor: theme.controlBackColor
                visible: showCancelButton
                Layout.alignment: Qt.AlignRight
                onClicked: {
                    cancelRequested()
                    close()
                }
            }
        }
    }
}
