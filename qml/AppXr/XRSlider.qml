import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

RowLayout {
    id: xrSlider

    property  alias  value: slider.value
    property  string unit: ""
    property  real   from: 0
    property  real   to: 100
    property  real   stepSize: 1
    property  string title: "Slider"
    property  color  inactiveColor: "#cdc9c9"
    property  int    fontSize: theme.iconSize * 1.5
    property  real   handleWidth: fontSize * 0.5
    property  int    sliderLen: fontSize * 10

    spacing: fontSize * 0.4

    //---------------- 标题 ----------------//
    Text {
        Layout.preferredWidth: implicitWidth
        text: title
        font.pixelSize: fontSize
        verticalAlignment: Text.AlignVCenter
    }


    Slider {
        id: slider
        Layout.preferredWidth: sliderLen
        Layout.preferredHeight: fontSize
        from: xrSlider.from
        to:   xrSlider.to
        stepSize: xrSlider.stepSize
        snapMode: Slider.SnapAlways
        live: true
        onValueChanged: xrSlider.valueChanged(value)

        background: Item {
            Rectangle {
                id: axisRec
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: fontSize * 0.2
                anchors.right: parent.right
                anchors.rightMargin: fontSize * 0.2
                height: 3
                radius: 1
                color: inactiveColor
            }

            // Repeater {
            //     id: tickRepeater
            //     model: Math.round((slider.to-slider.from) / slider.stepSize) + 1

            //     Rectangle {
            //         width:3
            //         height:fontSize * 0.3
            //         radius:1
            //         x:index*(parent.width-width)/(tickRepeater.count-1)
            //         y:(parent.height-height)/2
            //         color: inactiveColor
            //     }
            // }
        }

    }


    //---------------- 当前值 ----------------//
    Rectangle {
        Layout.preferredWidth: fontSize  * 2.1
        Layout.preferredHeight: fontSize * 1.2

        radius: height * 0.2
        color: "white"

        Text {
            anchors.centerIn: parent
            text: Number(slider.value).toFixed(0) + unit
            color: xrSlider.activeColor
            font.pixelSize: fontSize * 0.9
            font.bold: true
        }
    }
}
