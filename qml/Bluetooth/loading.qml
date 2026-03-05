import QtQuick 2.15
import QtQuick.Controls 2.15


// 加载指示器
Rectangle {
    id: loader
    anchors.centerIn: parent
    width: 75
    height: 75
    // radius: 16
    color: "#d6e6ff"

    BusyIndicator {
        anchors.centerIn: parent
        width: 50
        height: 50
        running: loader.visible
    }

}
