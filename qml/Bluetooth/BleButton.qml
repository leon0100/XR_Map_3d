import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15



Rectangle {
    id: root
    width: 270
    height: 40
    z: 999
    color: "transparent"
    radius: 5
    border.width: 0

    signal signalClearReadData()
    signal signalHideRealTrack(bool hide)
    signal signalPauseDrawTrack(bool paused)

    RowLayout {
        anchors.fill: parent
        anchors.margins: 5
        Layout.fillWidth: true
        spacing: 3

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            radius: 5
            color: btnClear.hovered ? "#a0c4ff" : "#d6e6ff"

            Button {
                id: btnClear
                anchors.fill: parent
                background: null
                hoverEnabled: true

                text: qsTr("Clear")

                contentItem: Text {
                    text: btnClear.text
                    font.pixelSize: 18
                    color: "#2c3e50"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    bleManager.clearRealData()
                    root.signalClearReadData()
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            radius: 5
            color: btnHide.hovered ? "#a0c4ff" : "#d6e6ff"

            Button {
                id: btnHide
                anchors.fill: parent
                background: null
                hoverEnabled: true

                property bool hide: false
                text: hide ? qsTr("Show") : qsTr("Hide")

                contentItem: Text {
                    text: btnHide.text
                    font.pixelSize: 18
                    color: "#2c3e50"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    hide = !hide
                    text = hide ? qsTr("Show") : qsTr("Hide")
                    root.signalHideRealTrack(hide)
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            radius: 5
            color: btnPause.hovered ? "#a0c4ff" : "#d6e6ff"

            Button {
                id: btnPause
                anchors.fill: parent
                background: null
                hoverEnabled: true

                property bool paused: false
                text: paused ? qsTr("Read") : qsTr("Pause")

                contentItem: Text {
                    text: btnPause.text
                    font.pixelSize: 18
                    color: "#2c3e50"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    paused = !paused
                    text = paused ? qsTr("Read") : qsTr("Pause")
                    root.signalPauseDrawTrack(paused)
                }
            }
        }
    }


}



