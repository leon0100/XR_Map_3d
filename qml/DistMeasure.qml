import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Shapes 1.15



Item {
    id: root
    visible: renderer.screetShot.isP1Visible
    anchors.fill: parent
    z: 999

    property int  distSize: Math.min(Screen.width, Screen.height) * 0.5
    property int  iconSize: distSize * 0.03
    property int  boxSize:  iconSize * 2

    property var  startPoint: renderer.screetShot.distLineP1  // 起点坐标
    property var  endPoint:   renderer.screetShot.distLineP2  // 终点坐标
    property string  distance: "0.0m"

    Connections {
        target: renderer.screetShot

        function onSignalStartToEndDist(dist) {
            if (dist >= 1000) {
                distance = (dist / 1000).toFixed(2) + " km"
            }
            else {
                distance = dist.toFixed(1) + " m"
            }
        }
    }


    // Canvas {
    //     id: lineCanvas
    //     anchors.fill: parent

    //     onPaint: {
    //         var ctx = getContext("2d")
    //         ctx.clearRect(0, 0, width, height)
    //         ctx.lineWidth = 3
    //         ctx.strokeStyle = "#0000ff"

    //         ctx.beginPath()
    //         ctx.moveTo(root.startPoint.x, root.startPoint.y)
    //         ctx.lineTo(root.endPoint.x, root.endPoint.y)
    //         ctx.stroke()
    //     }

    //     Connections {
    //         target: renderer.screetShot
    //         function onDistLineChanged() {
    //             lineCanvas.requestPaint()
    //         }
    //     }
    // }

    Shape {
        anchors.fill: parent

        ShapePath {
            strokeWidth: iconSize * 0.2
            strokeColor: "#00FFFF"
            fillColor:   "#00FFFF"
            startX: root.startPoint.x
            startY: root.startPoint.y

            PathLine {
                x: root.endPoint.x
                y: root.endPoint.y
            }
        }
    }


    // 起点图标
    Image {
        id: startIcon
        x: startPoint.x - boxSize * 0.5
        y: startPoint.y - boxSize
        width:  boxSize
        height: boxSize
        source: "qrc:/icons/ui/pin.svg"
        fillMode: Image.PreserveAspectFit
        visible: renderer.screetShot.isP1Visible
    }


    // 终点图标
    Image {
        id: endIcon
        x: endPoint.x
        y: endPoint.y - boxSize
        width: boxSize
        height: boxSize
        source: "qrc:/XR/target.png"
        fillMode: Image.PreserveAspectFit
        visible: renderer.screetShot.isP2Visible
    }


    // 距离文本
    Text {
        id: distanceText
        x: endPoint.x + height * 0.2
        y: endPoint.y - boxSize * 2.4
        text: distance
        color: "#ffff00"
        minimumPixelSize: iconSize
        font.bold: true
        padding: 5

        Rectangle {
            anchors.fill: parent
            color: "#333333"
            opacity: 0.85
            radius: 4
            z: -1
        }
    }
}
