import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Templates 2.15 as T
import QtQuick.Layouts 1.15


T.SpinBox {
    id: control

    property bool spinner: true
    property bool isValid: true
    property int  devValue: -2147483648
    property bool isDriverChanged: true
    property string unitText: "(m)"

    value: 50
    from: 20
    to: 30000
    editable: true
    font.pixelSize: theme.iconSize
    padding: 2

    implicitHeight: theme.menuWidth * 0.5
    implicitWidth:  implicitHeight * 2

    valueFromText: function(text, locale) {
        return Number.fromLocaleString(locale, text);
    }


    // 1. 输入框内容层
    contentItem: TextInput {
        id: textInput
        anchors.left:  control.spinner ? downControl.right : parent.left
        anchors.right: control.spinner ? upControl.left    : parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        text: control.textFromValue(control.value, control.locale) + control.unitText
        font.pixelSize: theme.iconSize * 0.85
        color: "black"
        selectedTextColor: theme.textColor
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        selectByMouse: true
        autoScroll: false
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: Qt.ImhFormattedNumbersOnly

        onTextEdited: {
            control.value = control.valueFromText(textInput.text, control.locale)
        }
    }


    // 2. 加号按钮 (右侧)
    up.indicator: Rectangle {
        id: upControl
        visible: control.spinner
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: control.spinner ? (theme.menuWidth * 0.5) : 0
        implicitWidth: theme.menuWidth * 0.5
        color: control.up.pressed ? "#d0d0d0" : "#eee9e9"

        Connections {
            target: control.up
            function onPressedChanged() {
                if (typeof upCanvas !== "undefined" && upCanvas !== null) {
                    upCanvas.requestPaint()
                }
            }
        }

        Text {
            text: "+"
            anchors.centerIn: parent
            font.pixelSize: theme.iconSize
            font.bold: true
            color: "black"
        }
    }


    // 3. 减号按钮 (左侧)
    down.indicator: Rectangle {
        id: downControl
        visible: control.spinner
        anchors.left: parent.left
        anchors.top:  parent.top
        anchors.bottom: parent.bottom
        width: control.spinner ? (theme.menuWidth * 0.5) : 0
        implicitWidth: theme.menuWidth * 0.5
        color: control.down.pressed ? "#d0d0d0" : "#eee9e9"

        Connections {
            target: control.down
            function onPressedChanged() {
                if (typeof downCanvas !== "undefined" && downCanvas !== null) {
                    downCanvas.requestPaint()
                }
            }
        }

        Text {
            text: "-"
            anchors.centerIn: parent
            font.pixelSize: theme.iconSize
            font.bold: true
            color: "black"
        }
    }


    background: Rectangle {
        anchors.left:  control.spinner ? downControl.right : parent.left
        anchors.right: control.spinner ? upControl.left    : parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: isValid ? "#828282": "red"
    }

    onDevValueChanged: {
        if (value !== devValue) {
            isDriverChanged = true
            value = devValue
        }
        else {
            isDriverChanged = false
        }
    }

    Component.onCompleted: {
        isDriverChanged = false
    }
}
