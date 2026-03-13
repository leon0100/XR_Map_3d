import QtQuick 2.15
import QtQuick.Window 2.15


Rectangle {
    id: triggerBar

    width:  collapseWidth
    height: collapseHeight
    anchors.horizontalCenter: parent.horizontalCenter
    radius: collapseHeight * 0.5
    color: "#fffafa"
    visible: true
    z: 3
    opacity: 0.4

    readonly property int collapseWidth: Math.min(Screen.width, Screen.height) * 0.125
    readonly property int collapseHeight: collapseWidth * 0.125
    property bool isExpanded: true
    property bool opacityHold: false

    signal toggleClicked(bool state)


    Behavior on opacity { NumberAnimation { duration: 200 } }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            isExpanded = !isExpanded
            toggleClicked(isExpanded)

            opacityHold = true
            triggerBar.opacity = 0.85
            opacityHoldTimer.restart()
        }

        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true
        onEntered: {
            if (!opacityHold) {
                parent.opacity = 0.85
            }
        }
        onExited: {
            if (!opacityHold) {
                parent.opacity = 0.4
            }
        }
    }

    Timer {
        id: opacityHoldTimer
        interval: 2000
        repeat: false
        onTriggered: {
            opacityHold = false
            triggerBar.opacity = 0.4
        }
    }


}
