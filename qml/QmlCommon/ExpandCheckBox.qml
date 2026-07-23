import QtQuick 2.15
import QtQuick.Controls 2.15



Item {
    id: root
    width: theme.iconSize * 12
    height: checkbox.height


    property string text: qsTr("Check")
    property bool   checked:  false
    property int iconSize: theme.iconSize * 1.5


    CheckBox {
        id: checkbox

        width: parent.width
        height: iconSize * 1.2
        checked: root.checked

        indicator: Rectangle {
            id: backRect
            implicitHeight: root.iconSize
            implicitWidth: root.iconSize
            radius: 1
            x: checkbox.leftPadding
            y: checkbox.height * 0.5 - height * 0.5

            color: theme.controlBackColor
            border.color: theme.textColor
            border.width: 1

            Rectangle {
                width: backRect.width * 0.5
                height: backRect.height * 0.5
                x: backRect.width * 0.25
                y: backRect.height * 0.25
                radius: 1
                color: theme.textColor
                visible: checkbox.checked
            }
        }


        contentItem: Text {
            id: labelText
            text: root.text
            font.pixelSize: root.iconSize
            verticalAlignment: Text.AlignVCenter
            leftPadding: checkbox.indicator.width + checkbox.spacing
        }


        onCheckedChanged: {
            root.checked = checked
            root.checkedChanged(checked)
        }
    }


}
