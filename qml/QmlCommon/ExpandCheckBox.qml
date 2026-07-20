import QtQuick 2.15
import QtQuick.Controls 2.15



Item {
    id: root
    property string text: qsTr("Check")
    property bool   checked:  false
    property bool   expanded: false

    property bool expandUp: false

    property int iconSize: theme.iconSize * 1.5

    property color panelColor: "#eeeeee"

    default property alias content: contentContainer.data

    width: theme.iconSize * 12
    height: checkbox.height + (expanded ? panel.height : 0)


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


        // 点击文字区域
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            onClicked: {
                if(mouseX > checkbox.indicator.width) {
                    root.expanded = !root.expanded
                    root.expandedChanged(root.expanded)
                }
                else {
                    checkbox.checked = !checkbox.checked
                }
            }
        }
    }


    // 抽屉面板
    Rectangle {
       id: panel
       width: parent.width

       color: panelColor
       clip: true

       // 根据方向决定位置
       anchors.left: parent.left
       anchors.right: parent.right
       anchors.top: root.expandUp ? undefined : checkbox.bottom
       anchors.bottom: root.expandUp ? checkbox.top : undefined

       height: root.expanded ? implicitHeight : 0
       implicitHeight: contentContainer.childrenRect.height +  contentContainer.anchors.margins * 2

       Item {
           id: contentContainer
           anchors.fill: parent
           anchors.margins: 10
       }

       Behavior on height {
           NumberAnimation {
               duration: 200
               easing.type: Easing.OutCubic
           }
       }
   }


}
