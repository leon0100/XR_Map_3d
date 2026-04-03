import QtQuick 2.15
import QtPositioning 5.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

Rectangle {
    id: polygonTool
    width: 800
    height: 600

    z: 9999


    property bool isDrawing: false
    property bool isEnabled: false
    property var map: null
    property var targetPlot: null

    property var polygonPoints: []
    property var screenPoints: []

    signal polygonCompleted(var points)
    signal polygonCleared()

    // 启动绘制模式
    function startDrawing() {
        polygonPoints = [];
        screenPoints = [];
        isDrawing = true;
        polygonCanvas.requestPaint();
    }

    // 完成绘制
    function finishDrawing() {
        var points = renderer.getPolygonPoints();
        polygonCompleted(points);
        polygonPoints = [];
        screenPoints = [];
        isDrawing = false;
        polygonCanvas.requestPaint();
    }

    // 清除多边形
    function clearPolygon() {
        polygonPoints = [];
        screenPoints = [];
        polygonCleared();
        polygonCanvas.requestPaint();
    }


    // Canvas 覆盖层：绘制多边形
    Canvas {
        id: polygonCanvas
        anchors.fill: parent
        visible: isDrawing

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            if (screenPoints.length < 1) return;

            // 绘制多边形边
            ctx.strokeStyle = "#00BFFF";
            ctx.lineWidth = 3;
            ctx.setLineDash([8, 4]);
            ctx.beginPath();

            for (var i = 0; i < screenPoints.length; i++) {
                var pt = screenPoints[i];
                if (i === 0) {
                    ctx.moveTo(pt.x, pt.y);
                } else {
                    ctx.lineTo(pt.x, pt.y);
                }
            }

            // 闭合多边形
            if (screenPoints.length >= 3) {
                ctx.lineTo(screenPoints[0].x, screenPoints[0].y);
                ctx.closePath();
                ctx.stroke();

                // 填充
                ctx.fillStyle = "rgba(0, 191, 255, 0.2)";
                ctx.fill();
            }

            ctx.stroke();

            // 绘制顶点
            ctx.setLineDash([]);
            ctx.fillStyle = "#FF4500";
            ctx.strokeStyle = "#FFFFFF";
            ctx.lineWidth = 2;

            for (var j = 0; j < screenPoints.length; j++) {
                var vpt = screenPoints[j];
                ctx.beginPath();
                ctx.arc(vpt.x, vpt.y, 8, 0, Math.PI * 2);
                ctx.fill();
                ctx.stroke();
            }
        }
    }

    // 控制面板（悬浮在底部）
    Rectangle {
        id: controlPanel
        width: 280
        height: 60

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        color: "#E0E0E0"
        radius: 8
        opacity: 0.95

        RowLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 10

            Button {
                text: isDrawing ? "完成" : "绘制多边形"
                font.pixelSize: 14
                implicitWidth: 100
                implicitHeight: 36
                background: Rectangle {
                    color: parent.down ? "#A0A0A0" : (isDrawing ? "#4CAF50" : "#2196F3")
                    radius: 4
                }
                onClicked: {
                    if (isDrawing) {
                        finishDrawing();
                    } else {
                        startDrawing();
                    }
                }
            }

            Button {
                text: "清除"
                font.pixelSize: 14
                implicitWidth: 80
                implicitHeight: 36
                enabled: isDrawing
                background: Rectangle {
                    color: parent.down ? "#A0A0A0" : "#f44336"
                    radius: 4
                }
                onClicked: {
                    clearPolygon();
                }
            }

            Text {
                text: "点数: " + (screenPoints.length || 0)
                font.pixelSize: 14
                color: "#333333"
            }
        }
    }
}






