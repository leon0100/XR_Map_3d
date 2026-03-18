import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import KoggerCommon 1.0

Rectangle {
    id: toastDialog
    z: 9999
    visible: false
    anchors.fill: parent
    color: "transparent"

    property string message: ""
    property int displayDuration: 3000  // milliseconds
    property string toastType: "info"   // info, success, warning, error
    property int toastWidth: 300
    property int toastHeight: 60
    property bool showProgress: true

    function show(msg, type, duration) {
        message = msg
        if (type !== undefined) toastType = type
        if (duration !== undefined) displayDuration = duration

        visible = true
        toastAnimation.restart()

        if (displayDuration > 0 && showProgress) {
            progressBar.value = 0
            progressAnimation.start()
        }
    }

    function close() {
        visible = false
        progressAnimation.stop()
    }

    Rectangle {
        id: toastBox
        width: toastWidth
        height: toastHeight
        anchors.centerIn: parent
        anchors.verticalCenterOffset: parent.height * 0.4
        radius: 8
        color: {
            switch(toastType) {
                case "success": return "#2e7d32";
                case "warning": return "#f57c00";
                case "error": return "#c62828";
                default: return theme.controlBackColor;
            }
        }
        border.color: {
            switch(toastType) {
                case "success": return "#4caf50";
                case "warning": return "#ff9800";
                case "error": return "#f44336";
                default: return theme.controlBorderColor;
            }
        }
        border.width: 1

        property real targetY: parent.height * 0.4

        NumberAnimation on anchors.verticalCenterOffset {
            id: toastAnimation
            from: parent.height * 0.6
            to: toastBox.targetY
            duration: 300
            easing.type: Easing.OutCubic
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 4

            KText {
                text: message
                color: toastType === "info" ? theme.textColor : "white"
                font.pixelSize: theme.iconSize
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            ProgressBar {
                id: progressBar
                Layout.fillWidth: true
                value: 0
                visible: showProgress && displayDuration > 0
                background: Rectangle {
                    color: "transparent"
                }
                contentItem: Rectangle {
                    color: toastType === "info" ? theme.controlBorderColor : "white"
                }

                NumberAnimation on value {
                    id: progressAnimation
                    from: 0
                    to: 1
                    duration: displayDuration
                    onFinished: {
                        toastDialog.close()
                    }
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            close()
        }
    }
}
