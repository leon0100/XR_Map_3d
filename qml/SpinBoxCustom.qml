import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


SpinBox {
    id: control

    property bool spinner: true
    property bool isValid: true
    property int devValue: Number.MIN_VALUE
    property bool isDriverChanged: true


    value: 50
    from: 20
    to: 30000
    editable: true
    font.pixelSize: 16
    padding: 2

    implicitHeight: theme.menuWidth * 0.75
    implicitWidth: implicitHeight * 4

    valueFromText: function(text, locale) {
        return Number.fromLocaleString(locale, text);
    }


    contentItem: TextInput {
        id:    textInput
        x: control.spinner ? down.indicator.width : 0
        text: control.textFromValue(control.value, control.locale)
        font.pixelSize: theme.iconSize
        width: control.spinner ? (control.width - down.indicator.width - up.indicator.width) : control.width
        color: "black"
        selectedTextColor:   theme.textColor
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment:   Qt.AlignVCenter
        selectByMouse: true

        onTextEdited: {
            control.value = control.valueFromText(textInput.text, control.locale)
        }

        autoScroll: false
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: Qt.ImhFormattedNumbersOnly
    }


    up.onPressedChanged: upCanvas.requestPaint()
    up.indicator: Rectangle {
        id: upControl
        visible: control.spinner
        x: parent.width - width
        height: theme.menuWidth * 0.6
        width: control.spinner ? (theme.menuWidth * 0.6) : 0

        Text {
            text: "+"
            anchors.centerIn: parent
            font.pixelSize: theme.iconSize
            font.bold: true
            color: "black"
        }
    }




    down.onPressedChanged: downCanvas.requestPaint()
    down.indicator: Rectangle {
        id: downControl
        visible: control.spinner
        x: 0
        height: theme.menuWidth * 0.6
        width: control.spinner ? (theme.menuWidth * 0.6) : 0

        Text {
            text: "-"
            anchors.centerIn: parent
            font.pixelSize: theme.iconSize
            font.bold: true
            color: "black"
        }
    }




    background: Rectangle {
        x: control.spinner ? down.indicator.width : 0
        y: 0
        width: control.spinner ? control.width - downCanvas.width - upCanvas.width : control.width
        height: control.height
        color: isValid ? theme.controlBackColor : "red"
        border.color: theme.controlBorderColor
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
