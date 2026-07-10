import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15



Button {
    property bool isKlfLogging: false
    property bool active: false
    property int  borderWidth: 0
    property real klfTint: 1.0

    id: control
    Layout.preferredHeight: theme.menuWidth
    padding: 6

    icon.color:"transparent"

    property color baseBackColor: "transparent"

    background: Rectangle {
        id: backRect
        radius: 2
        height: parent.height
        width: parent.width

        color: control.baseBackColor

        border.color: theme.controlBorderColor
        border.width: borderWidth
    }

}
