import QtQuick 2.15
import QtQuick.Window 2.15


Item {
    id: root
    width:  boxSize
    height: boxSize
    x: renderer.screetShot.landMarkPtX - boxSize * 0.5
    y: renderer.screetShot.landMarkPtY - boxSize * 0.5
    z: 999
    visible: renderer.screetShot.landMarkMode

    property int landSize: Math.min(Screen.width, Screen.height) * 0.3
    property int iconSize: landSize * 0.1
    property int boxSize:  iconSize * 1.4

    property color lineColor: "#ffff00"
    property int lineLength: boxSize * 0.2
    property int lineWidth: 4

    Item {
        id: blinkGroup
        anchors.fill: parent
        opacity: 1.0

        SequentialAnimation on opacity {
            loops: Animation.Infinite

            NumberAnimation {
                from: 1.0
                to: 0.1
                duration: 800
                easing.type: Easing.InOutQuad
            }

            NumberAnimation {
                from: 0.1
                to: 1.0
                duration: 800
                easing.type: Easing.InOutQuad
            }
        }

        Rectangle {
            id: frame

            anchors.centerIn: parent
            width: boxSize
            height: boxSize

            color: "transparent"
            border.width: lineWidth
            border.color: lineColor
        }

        // 上边中点
        Rectangle {
            width: lineWidth - 1
            height: lineLength

            color: lineColor

            anchors.horizontalCenter: frame.horizontalCenter
            anchors.bottom: frame.top
        }

        // 下边中点
        Rectangle {
            width: lineWidth - 1
            height: lineLength

            color: lineColor

            anchors.horizontalCenter: frame.horizontalCenter
            anchors.top: frame.bottom
        }

        // 左边中点
        Rectangle {
            width: lineLength
            height: lineWidth - 1

            color: lineColor

            anchors.verticalCenter: frame.verticalCenter
            anchors.right: frame.left
        }

        // 右边中点
        Rectangle {
            width: lineLength
            height: lineWidth - 1

            color: lineColor

            anchors.verticalCenter: frame.verticalCenter
            anchors.left: frame.right
        }

    }

    Image {
        anchors.centerIn: parent
        width: iconSize
        height: iconSize
        source: "qrc:/XR/pushPin2.png"
        fillMode: Image.PreserveAspectFit
        smooth: true
    }
}
