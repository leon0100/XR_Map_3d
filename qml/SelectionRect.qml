import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: selectionRect
    visible: renderer.isSelectionRectVisible
    
    x: renderer.selectionRect.x
    y: renderer.selectionRect.y
    width: renderer.selectionRect.width
    height: renderer.selectionRect.height
    color: Qt.rgba(0, 245, 255, 0.5)
    border.width: 2
    border.color: Qt.rgba(0, 245, 255, 1.0)
    z: 1000
    
    Behavior on x {
        NumberAnimation { duration: 0 }
    }
    
    Behavior on y {
        NumberAnimation { duration: 0 }
    }
    
    Behavior on width {
        NumberAnimation { duration: 0 }
    }
    
    Behavior on height {
        NumberAnimation { duration: 0 }
    }
    
    Component.onCompleted: {
        console.log("SelectionRect initial state:")
        console.log("  x:", x)
        console.log("  y:", y)
        console.log("  width:", width)
        console.log("  height:", height)
        console.log("  visible:", visible)
        console.log("  z:", z)
    }
    
    onXChanged: console.log("SelectionRect x changed:", x)
    onYChanged: console.log("SelectionRect y changed:", y)
    onWidthChanged: console.log("SelectionRect width changed:", width)
    onHeightChanged: console.log("SelectionRect height changed:", height)
    onVisibleChanged: console.log("SelectionRect visible changed:", visible)
}
