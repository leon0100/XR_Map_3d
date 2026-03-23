import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


Rectangle {
    id: progressDialog

    width:  dialogWidth
    height: dialogHeight
    x: theme.screenWidth*0.5  - dialogWidth*0.5
    y: theme.screenHeight*0.4 - dialogHeight*0.5
    z: 9999
    visible: false
    radius: 8
    color: "#E6F2FA"

    property string  title: qsTr("Open File")
    property string  statusText: qsTr("Openging")
    property real    progress: 338
    property bool    indeterminate: true
    property bool    showProgressBar: true
    property bool    showCancelButton: true
    property int     dialogWidth: theme.screenSize * 0.36
    property int     dialogHeight: theme.screenSize * 0.14
    property int     iconSize: theme.iconSize * 0.9



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


    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        // Title
        KText {
            text: title
            font.bold: true
            font.pixelSize: iconSize
            color: "black"
        }
        Rectangle {
            id: separator
            Layout.fillWidth: true
            height: 1
            color: "#dcdcdc"
            Layout.topMargin: 2
            Layout.bottomMargin: 5
        }

        // Status Text
        KText {
            id: statusLabel
            text: progressDialog.statusText
            font.pixelSize: iconSize
            color: "black"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        //Progress Bar
        ProgressBar {
            id: progressBar
            Layout.fillWidth: true
            value: progressDialog.progress
            indeterminate: indeterminate
            visible: true
        }

        // Cancel Button
        Button {
            id: cancelButton
            text: qsTr("Close")
            font.pixelSize: iconSize
            visible: showCancelButton
            Layout.alignment: Qt.AlignRight
            background: Rectangle {
               radius: 4
               color: cancelButton.hovered ? "#d0e8ff" : "#d9edff"
               border.color: "#9bbfe9"
               border.width: 1
            }
            onClicked: {
                close()
            }
        }

    }

}


