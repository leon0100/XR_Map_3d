import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


Rectangle {
    id: progressDialog

    anchors.fill: parent
    z: 9999
    visible: false
    color: "#80000000"

    property  string  title:            qsTr("Open File")
    property  string  statusText:       qsTr("Openging")
    property  real    progress:         338
    property  bool    indeterminate:    true
    property  bool    showCancelButton: false
    property  int     dialogWidth:      theme.screenSize * 0.31
    property  int     dialogHeight:     theme.screenSize * 0.145
    property  int     iconSize:         theme.iconSize


    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        hoverEnabled: true
        preventStealing: true
        onWheel: wheel.accepted = true
    }

    function open() {
        visible = true
    }

    function close() {
        visible = false
        showCancelButton = false
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
        x: theme.screenWidth*0.5  - dialogWidth*0.5
        y: theme.screenHeight*0.4 - dialogHeight*0.5
        width:  dialogWidth
        height: dialogHeight
        radius: iconSize * 0.5
        color: "#E6F2FA"
    }


    ColumnLayout {
        anchors.fill: dialogBox
        anchors.margins: iconSize* 0.5
        spacing: iconSize * 0.4

        KText {
            text: title
            font.bold: true
            font.pixelSize: iconSize
            color: "black"
        }
        Rectangle {
            id: separator
            Layout.fillWidth: true
            height: 2
            color: "#c8c8c8"
            Layout.topMargin: 2
            Layout.bottomMargin: 5
        }

        KText {
            id: statusLabel
            text: progressDialog.statusText
            font.pixelSize: iconSize
            color: "black"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        ProgressBar {
            id: progressBar
            Layout.fillWidth: true
            Layout.preferredHeight: iconSize * 0.6
            value: progressDialog.progress
            indeterminate: indeterminate
            visible: true

            background: Rectangle {
                color: "#E0E0E0"
                radius: 3
            }
            contentItem: Rectangle {
                width: progressBar.visualPosition * progressBar.width
                color: "#00cd00"
                radius: 3
            }
        }

        Button {
            id: cancelButton
            text: qsTr("Close")
            font.pixelSize: iconSize
            visible: showCancelButton
            Layout.alignment: Qt.AlignRight
            Layout.preferredHeight: iconSize * 1.5
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
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: iconSize * 3
        }

    }

}

