import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import KoggerCommon 1.0

Rectangle {
    id: dialogOverlay
    z: 9999
    visible: false
    anchors.fill: parent
    color: "#88000000"

    property bool modal: true
    property string dialogTitle: qsTr("Dialog")
    property string message: ""
    property string iconSource: ""
    property int dialogWidth: 400
    property int dialogHeight: 200
    property int buttonCount: 1  // 1: OK, 2: OK/Cancel, 3: Yes/No/Cancel, 4: Yes/No
    property string okText: qsTr("OK")
    property string cancelText: qsTr("Cancel")
    property string yesText: qsTr("Yes")
    property string noText: qsTr("No")
    property bool showIcon: false
    property bool showProgress: false
    property real progressValue: 0.0
    property bool progressIndeterminate: false
    property string progressText: ""

    signal accepted()
    signal rejected()
    signal yesClicked()
    signal noClicked()

    function open() {
        visible = true
    }

    function close() {
        visible = false
    }

    function show(title, msg, buttons, icon) {
        dialogTitle = title
        message = msg
        if (buttons !== undefined) buttonCount = buttons
        if (icon !== undefined) showIcon = true
        open()
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
                    id: closeButton
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

            // Content Area
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 16

                // Icon
                Image {
                    id: dialogIcon
                    width: 48
                    height: 48
                    source: iconSource
                    visible: showIcon && source !== ""
                    Layout.alignment: Qt.AlignTop
                }

                // Message
                KText {
                    id: messageText
                    text: message
                    color: theme.textColor
                    font.pixelSize: theme.iconSize
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }

            // Progress Bar (optional)
            ColumnLayout {
                Layout.fillWidth: true
                visible: showProgress

                KText {
                    text: progressText
                    color: theme.textColor
                    font.pixelSize: theme.iconSize - 2
                    visible: progressText !== ""
                }

                ProgressBar {
                    id: progressBar
                    Layout.fillWidth: true
                    value: progressValue
                    indeterminate: progressIndeterminate
                }
            }

            // Button Row
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignRight
                height: 40
                spacing: 12

                // Cancel button (for 2, 3 button dialogs)
                CButton {
                    id: cancelBtn
                    text: cancelText
                    visible: buttonCount === 2 || buttonCount === 3
                    backColor: theme.controlBackColor
                    onClicked: {
                        close()
                        rejected()
                    }
                }

                // No button (for 3, 4 button dialogs)
                CButton {
                    id: noBtn
                    text: noText
                    visible: buttonCount === 3 || buttonCount === 4
                    backColor: theme.controlBackColor
                    onClicked: {
                        close()
                        noClicked()
                    }
                }

                // Yes button (for 3, 4 button dialogs)
                CButton {
                    id: yesBtn
                    text: yesText
                    visible: buttonCount === 3 || buttonCount === 4
                    onClicked: {
                        close()
                        yesClicked()
                    }
                }

                // OK button (for 1, 2 button dialogs)
                CButton {
                    id: okBtn
                    text: okText
                    visible: buttonCount === 1 || buttonCount === 2
                    onClicked: {
                        close()
                        accepted()
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
        close()
        accepted()
    }
}
