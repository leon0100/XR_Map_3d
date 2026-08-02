import QtQuick 2.15
import QtQuick.Controls 2.15


ComboBox {
    id: control

    property var schemes: [
        // Blue
        [
            "#000080",
            "#0000ff",
            "#00ffff",
            "#00ff00",
            "#ffff00",
            "#ff0000"
        ],

        // Sepia
        [
            "#000000",
            "#553300",
            "#996600",
            "#cc9900",
            "#ffcc66",
            "#ffffff"
        ],

        // WRGBD
        [
            "#000080",
            "#0080ff",
            "#00ffff",
            "#00ff00",
            "#ffff00",
            "#ff0000"
        ],


        // WhiteBlack
        [
            "#ffffff",
            "#cccccc",
            "#888888",
            "#444444",
            "#000000"
        ],


        // BlackWhite
        [
            "#000000",
            "#444444",
            "#888888",
            "#cccccc",
            "#ffffff"
        ]
    ]


    property int schemeSize: theme.iconSize



    implicitWidth: schemeSize * 10
    implicitHeight: schemeSize



    // 背景边框
    background: Rectangle {
        radius: 3
        color: "transparent"
        border.width: 1
        border.color: "#666666"
    }


    // ===========================
    // 当前显示色条
    // ===========================
    contentItem: Row {
        clip: true
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.verticalCenter: parent.verticalCenter
        spacing: 1

        Repeater {
            model: control.schemes[control.currentIndex].length
            Rectangle {
                // width: (control.width - 35) /  control.schemes[control.currentIndex].length
                width:
                    (control.width - control.anchors.leftMargin - 10)
                    / control.schemes[control.currentIndex].length
                height: control.height - 2
                color:  control.schemes[control.currentIndex][index]
            }
        }
    }



    // 下拉箭头
    indicator: Canvas {
        width: schemeSize
        height: schemeSize
        x:  control.width - width
        y:  control.height / 2 - height / 2

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0,0,width,height)
            ctx.fillStyle = "#dddddd"
            ctx.beginPath()
            ctx.moveTo(0,0)
            ctx.lineTo(width,0)
            ctx.lineTo(width/2,height)
            ctx.closePath()
            ctx.fill()
        }
    }



    // 下拉列表项
    delegate: ItemDelegate {
        width:  control.width
        height: schemeSize
        background: Rectangle {
            color: highlighted ? "#444444" : "transparent"
        }

        Row {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 5
            anchors.rightMargin: 5
            anchors.verticalCenter: parent.verticalCenter

            spacing: 1

            Repeater {
                model:  control.schemes[index].length

                Rectangle {
                    width: (control.width - 12) / control.schemes[index].length
                    height: schemeSize
                    radius: 1
                    color:  control.schemes[index][modelData]
                }
            }
        }
    }
}
