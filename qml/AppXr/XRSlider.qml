import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

RowLayout {
    id: xrSlider

    property alias value: slider.value
    property string unit: ""
    property real from: 0
    property real to: 100
    property real stepSize: 1
    property string title: "Slider"
    property int fontSize: theme.iconSize * 1.5

    property real handleWidth: fontSize * 0.8
    property int  sliderLen: fontSize * 12
    property real trackHeight: fontSize * 0.25
    property int  textLen: fontSize * 2.5

    spacing: fontSize * 0.5

    Text {
        Layout.preferredWidth: implicitWidth
        text: title
        font.pixelSize: fontSize
        verticalAlignment: Text.AlignVCenter
    }

    //---------------- Slider ----------------//
    Slider {
        id: slider
        Layout.preferredWidth: sliderLen
        Layout.preferredHeight: fontSize * 1.5

        from: xrSlider.from
        to: xrSlider.to
        stepSize: xrSlider.stepSize
        snapMode: Slider.SnapAlways
        live: true

        onValueChanged: xrSlider.valueChanged(value)

        background: Item {
            anchors.fill: parent

            Rectangle {
                id: axisRec
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: handleWidth / 2
                anchors.rightMargin: handleWidth / 2
                height: trackHeight
                radius: height / 2
                color: "#cdc9c9"
            }
        }

        handle: Rectangle {
            width: handleWidth
            height: handleWidth
            radius: width / 2
            x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
            y: slider.topPadding + (slider.availableHeight - height) / 2
            color: "#666666"
            border.width: 1
            border.color: "#444444"
        }
    }

    Rectangle {
        Layout.preferredWidth: xrSlider.textLen
        Layout.preferredHeight: fontSize * 1.3
        radius: height * 0.2
        color: "white"

        Text {
            anchors.centerIn: parent
            text: Number(slider.value).toFixed(0) + unit
            color: "black"
            font.pixelSize: fontSize * 0.9
            font.bold: true
        }
    }
}
