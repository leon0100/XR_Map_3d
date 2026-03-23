import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Button {
    id: control
    checkable: true    
    property bool active: checked || down

    property int borderWidth: 1
    property color color: theme.textColor
    property color checkedColor: "black"
    property color checkedBackColor: "white"
    property color backColor: "transparent"
    property color checkedBorderColor: "transparent"
    property color borderColor: theme.controlSolidBorderColor
    property string iconSource: ""
    property real   iconScale: 0.80

    implicitHeight: theme.menuWidth

    icon.source: iconSource
    icon.width:  implicitHeight * control.iconScale
    icon.height: implicitHeight * control.iconScale

    hoverEnabled: true
    padding: 0
    rightPadding: text === "" ? 2 : 6
    leftPadding: icon.source === "" ? 6 : 2


    font: theme.textFont
    palette.buttonText: active ? checkedColor : color
    palette.brightText: active ? checkedColor : color

    icon.color: active ? checkedColor : color

    contentItem: RowLayout {
        Item {
            Layout.preferredWidth: control.height
            Layout.fillHeight: true

            Image {
                source: control.iconSource
                anchors.centerIn: parent
                width: icon.width
                height: icon.height
            }
        }

        Text {
            text: control.text
            font: control.font
            color: control.active ? control.checkedColor : control.color
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            Layout.fillWidth: true
        }
    }



    background: Rectangle {
        id: backRect
        anchors.fill: parent
        anchors.margins: 0
        radius: 2
        height: parent.height
        width: parent.width
        color: control.active ? control.checkedBackColor : control.backColor
        border.color: control.active ? control.checkedBorderColor : control.borderColor
        border.width: control.borderWidth
    }

    onCheckableChanged: {
        if (!checkable && checked) {
            checked = false
        }
    }

    scale: pressed ? 0.96 : 1
    Behavior on scale {
        NumberAnimation { duration: 100; easing.type: Easing.OutQuad }
    }
}
