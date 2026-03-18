import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: progressDialog

    width:  dialogWidth
    height: dialogHeight
    x: dialogHeight
    y: dialogWidth
    z: 9999
    visible: true
    color: "#880000"

    property string  title: qsTr("Opening Files...")
    property string  statusText: core.fileProgressStatus
    property real    progress: core.fileProgress
    property bool    indeterminate: true
    property bool    showProgressBar: true
    property bool    showCancelButton: true
    property int     dialogWidth: theme.screenSize * 0.5
    property int     dialogHeight: theme.screenSize * 0.2


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
        console.log("QML setStatus called with:", text)
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
        anchors.fill: parent
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
                text: progressDialog.statusText
                color: "red"
                font.pixelSize: theme.iconSize
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            // Progress Bar
            // ProgressBar {
            //     id: progressBar
            //     Layout.fillWidth: true
            //     value: progressDialog.progress
            //     indeterminate: indeterminate
            //     visible: true
            // }

            Text {
                id: progressBar
                Layout.fillWidth: true
                text: String(progressDialog.progress)
                visible: true
                color: "red"
                font.bold: true
            }

            // Cancel Button
            Button {
                id: cancelButton
                text: qsTr("Cancel")
                // backColor: theme.controlBackColor
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


