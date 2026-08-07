import QtQuick 2.15

Rectangle {
    id: root

    y: - height
    width: 100
    height: expanded ? expandHeight : 0
    z: 10000
    color: "#dbe3f2"
    border.color: "white"
    border.width: 1
    radius: Math.min(width, height) * 0.03
    opacity: 1.0

    property bool draggable: true
    property bool dragging: false

    property Item dragArea: null  // 拖动区域限制

    property bool expanded: false
    property real expandHeight: 100

   // 从底部向上展开的缩放变换
   transform: Scale {
       origin.x: root.width / 2
       origin.y: root.height
       xScale: 1
       yScale: root.expanded ? 1 : 0

       Behavior on yScale {
           NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
       }
   }


    MouseArea {
        id: mouseArea

        anchors.fill: parent

        acceptedButtons: Qt.LeftButton

        hoverEnabled: true

        property real pressX
        property real pressY


        onPressed: {
            if (!root.draggable)
                return

            root.dragging = true


            pressX = mouse.x
            pressY = mouse.y
        }


        onPositionChanged: {
            if (!pressed & Qt.LeftButton)  {
                return
            }
            var dx = mouse.x - pressX
            var dy = mouse.y - pressY
            var newX = root.x + dx
            var newY = root.y + dy
            // 如果指定拖动区域
            // if (root.dragArea) {
                // newX = Math.max( 0, Math.min( newX, root.dragArea.width - root.width ) )
                // newY = Math.max( 0, Math.min( newY, root.dragArea.height - root.height ) )
            // }
            root.x = newX
            root.y = newY
        }

        onReleased: {
            root.dragging = false
        }

    }
}
