import QtQuick 2.15
import QtQuick.Window 2.15

Rectangle {
    id: root
    visible: renderer.screetShot.isDistMeasureVisible
    z: 999

    property int  distSize: Math.min(Screen.width, Screen.height) * 0.5
    property int  iconSize: distSize * 0.03
    property int  boxSize:  iconSize * 1.2

    property var  startPoint: Qt.point(0, 0)  // 起点坐标
    property var  endPoint:   Qt.point(0, 0)  // 终点坐标
    property bool measuring:  false           // 是否正在测量
    property real distance:    0              // 距离值（米）


    // 更新距离
    function updateDistance(start, end) {
        var dx = end.x - start.x
        var dy = end.y - start.y
        distance = Math.sqrt(dx * dx + dy * dy)
        startPoint = start
        endPoint = end
        measuring = true
        visible = true
    }

    // 清除测量
    function clearMeasure() {
        measuring = false
        visible = false
        distance = 0
    }

    // 计算线段中点
    function getMidPoint(start, end) {
        return Qt.point((start.x + end.x) / 2, (start.y + end.y) / 2)
    }

    // 计算线段角度（用于文本旋转）
    function getLineAngle(start, end) {
        return Math.atan2(end.y - start.y, end.x - start.x) * 180 / Math.PI
    }

    // 格式化距离显示
    function formatDistance(meters) {
        if (meters >= 1000) {
            return (meters / 1000).toFixed(2) + " km"
        } else {
            return meters.toFixed(1) + " m"
        }
    }


    // 连接线
    Canvas {
        id: lineCanvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            if (!root.measuring)
                return

            ctx.beginPath()
            ctx.moveTo(root.startPoint.x, root.startPoint.y)
            ctx.lineTo(root.endPoint.x, root.endPoint.y)

            ctx.lineWidth = 3
            ctx.strokeStyle = "#FF6B6B"
            ctx.stroke()
        }

        Connections {
            target: root

            function onStartPointChanged() {
                lineCanvas.requestPaint()
            }

            function onEndPointChanged() {
                lineCanvas.requestPaint()
            }

            function onMeasuringChanged() {
                lineCanvas.requestPaint()
            }
        }
    }

    // 起点图标
    Image {
        id: startIcon
        x: renderer.screetShot.distLine.x1
        y: renderer.screetShot.distLine.y1
        width:  boxSize
        height: boxSize
        source: "qrc:/XR/pin.png"
        onStatusChanged: {
            console.log("pin status =", status)
        }
        fillMode: Image.PreserveAspectFit
        visible: true
    }


    // 终点图标
    Image {
        id: endIcon
        x: renderer.screetShot.distLine.x2
        y: renderer.screetShot.distLine.y2
        width: boxSize
        height: boxSize
        source: "qrc:/XR/target.png"
        fillMode: Image.PreserveAspectFit
        visible: true
    }


    // 距离文本
    Text {
        id: distanceText
        x: getMidPoint(startPoint, endPoint).x - width / 2
        y: getMidPoint(startPoint, endPoint).y - height / 2
        text: formatDistance(distance)
        color: "#fff"
        minimumPixelSize: iconSize
        font.bold: true
        padding: 6
        opacity: 0.9
        rotation: getLineAngle(startPoint, endPoint)

        Rectangle {
            anchors.fill: parent
            color: "#333"
            opacity: 0.9
            radius: 4
            z: -1
        }
    }
}
