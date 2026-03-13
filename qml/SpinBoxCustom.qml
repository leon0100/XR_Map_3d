import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

SpinBox {
    id: control
    value: 50
    from: 20
    to: 30000
    editable: true
    font.pixelSize: theme.iconSize
    implicitHeight: theme.menuWidth * 0.6
    implicitWidth: implicitHeight * 5

    // 统一的按钮外观组件
    component SpinnerButton: Rectangle {
        property bool isUp: true
        width: height
        color: parent.pressed ? theme.controlSolidBackColor : theme.controlBackColor
        border.color: theme.controlBorderColor

        Text {
            anchors.centerIn: parent
            text: isUp ? "+" : "-"
            font.pixelSize: parent.height * 0.5
            color: "black"
        }
    }

    // 使用简单的 Rectangle 替代 Canvas
    up.indicator: SpinnerButton { isUp: true }
    down.indicator: SpinnerButton { isUp: false }

    background: Rectangle {
        color: isValid ? theme.controlBackColor : "red"
        border.color: theme.controlBorderColor
    }

    contentItem: TextInput {
        text: control.textFromValue(control.value, control.locale)
        font.pixelSize: parent.height * 0.5
        color: "black"
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        readOnly: !control.editable
    }
}
