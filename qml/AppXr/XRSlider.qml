import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

RowLayout {

    id: root

    //---------------- 对外属性 ----------------//

    property alias value: slider.value

    property real from: 0
    property real to: 100
    property real stepSize: 1

    property string title: "Slider"

    property color activeColor: "#2EA8FF"
    property color inactiveColor: "#666666"

    property int fontSize: theme.iconSize * 1.5

    // signal valueChanged(real value)

    spacing: fontSize * 0.8
    Layout.fillWidth: true

    //---------------- 标题 ----------------//

    Text {

        Layout.preferredWidth: fontSize * 6

        text: title

        color: "white"

        font.pixelSize: fontSize

        verticalAlignment: Text.AlignVCenter
    }

    //---------------- Slider ----------------//

    Slider {

        id: slider

        Layout.fillWidth: true
        Layout.preferredWidth: fontSize * 12

        from: root.from
        to: root.to
        stepSize: root.stepSize

        snapMode: Slider.SnapAlways

        live: true

        // onValueChanged: root.valueChanged(value)

        background: Item {

            implicitHeight: 36

            readonly property real ratio:
                (slider.value-slider.from)/(slider.to-slider.from)

            //---------------- 灰色轨道 ----------------//

            Rectangle {

                anchors.verticalCenter: parent.verticalCenter

                anchors.left: parent.left
                anchors.right: parent.right

                height: 2

                radius: 1

                color: root.inactiveColor
            }

            //---------------- 蓝色轨道 ----------------//

            Rectangle {

                anchors.left: parent.left

                anchors.verticalCenter: parent.verticalCenter

                width: parent.width*parent.ratio

                height: 3

                radius: 2

                gradient: Gradient {

                    GradientStop {
                        position: 0
                        color: "#0A7DFF"
                    }

                    GradientStop {
                        position: 1
                        color: "#45D1FF"
                    }
                }
            }

            //---------------- 刻度 ----------------//

            Repeater {

                model: Math.round((slider.to-slider.from)/slider.stepSize)+1

                Rectangle {

                    property bool active:
                        index<=Math.round((slider.value-slider.from)/slider.stepSize)

                    width: 2

                    height:
                        index===0 || index===model-1 ? 18 :
                        index%2===0 ? 16 : 12

                    radius: 1

                    x: index*(parent.width-width)/(model-1)

                    y: parent.height/2-height/2

                    color: active ?
                           root.activeColor :
                           "#777777"

                    Behavior on color {

                        ColorAnimation {

                            duration: 120
                        }
                    }
                }
            }
        }

        //---------------- 滑块 ----------------//

        handle: Item {

            implicitWidth: 26
            implicitHeight: 26

            // 外发光
            Rectangle {

                anchors.centerIn: parent

                width: 26
                height: 26

                radius: 13

                color: root.activeColor

                opacity: slider.pressed ? 0.35 : 0.18

                Behavior on opacity {

                    NumberAnimation {
                        duration: 120
                    }
                }
            }

            // 中层
            Rectangle {

                anchors.centerIn: parent

                width: 20
                height: 20

                radius: 10

                color: root.activeColor

                opacity: slider.pressed ? 0.7 : 0.45
            }

            // 主圆
            Rectangle {

                anchors.centerIn: parent

                width: 16
                height: 16

                radius: 8

                color: root.activeColor

                border.width: 2
                border.color: "white"
            }

            // 高光
            Rectangle {

                anchors.centerIn: parent

                width: 6
                height: 6

                radius: 3

                color: "white"

                opacity: 0.9
            }

            scale: slider.pressed ? 1.15 : 1.0

            Behavior on scale {

                NumberAnimation {

                    duration: 100
                }
            }
        }
    }

    //---------------- 当前值 ----------------//

    Rectangle {

        Layout.preferredWidth: fontSize*2.4
        Layout.preferredHeight: fontSize*1.4

        radius: height/2

        color: "#202020"

        border.color: root.activeColor

        border.width: 1

        Text {

            anchors.centerIn: parent

            text: Number(slider.value).toFixed(0)

            color: root.activeColor

            font.pixelSize: fontSize*0.9

            font.bold: true
        }
    }
}
