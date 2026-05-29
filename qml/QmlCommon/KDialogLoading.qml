import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15


// 加载指示器
Rectangle {
    id: loader
    anchors.centerIn: parent
    width: loadSize
    height: loadSize
    color: "#d6e6ff"
    visible: false

    property int loadSize: Math.min(Screen.width, Screen.height) * 0.075
    property int iconSize: loadSize * 0.3


    BusyIndicator {
        anchors.centerIn: parent
        width: loadSize * 0.85
        height: loadSize * 0.85
        running: loader.visible
    }


    function show(msg) {
        console.log("msg.....", msg)
        if(msg === "show") {
            loader.visible = true
        }
        else if(msg === "hide") {
            loader.visible = false
        }
    }

}
