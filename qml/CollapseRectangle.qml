import QtQuick 2.15

Rectangle {
    id: triggerBar

    width: isExpanded ? 300 : 500
    height: isExpanded ? smallCollapseHeight : bigCollapseHeight
    color: Qt.rgba(0.2, 0.2, 0.2, 0.6)
    anchors.horizontalCenter: parent.horizontalCenter
    z: 3
    radius: 4
    border.color: "#c0c0c0"
    anchors.topMargin: 3
    visible: true


    readonly property int bigCollapseHeight: 80
    readonly property int smallCollapseHeight: 60

    property bool isExpanded: true

    signal toggleClicked(bool state)


    // 动画配置：监听 width 和 height 的变化
    Behavior on width { NumberAnimation { duration: 400; easing.type: Easing.OutBack } }
    Behavior on height { NumberAnimation { duration: 400; easing.type: Easing.OutBack } }

    // 点击后的逻辑：由小变大 -> 保持 3 秒 -> 变小
    MouseArea {
        anchors.fill: parent
        onClicked: {


            // 触发外部逻辑（控制工具栏）
            isExpanded = !isExpanded
            toggleClicked(isExpanded)

            // 如果点击后变大了，启动 3 秒计时器回到小状态
            if (!isExpanded) {
                restoreTimer.restart()
            }
        }

        cursorShape: Qt.PointingHandCursor
        hoverEnabled: true
        onEntered: parent.color = "#d0d0d0"
        onExited: parent.color = "#e0e0e0"
    }

    // 计时器：3秒后自动恢复“小且细”的状态
    Timer {
        id: restoreTimer
        interval: 3000
        repeat: false
        onTriggered: {
            isExpanded = true
        }
    }
}
