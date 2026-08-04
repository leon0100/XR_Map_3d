// import QtQuick 2.15
// import QtQuick.Controls 2.15
// import QtQuick.Window 2.15


/*----------------------------------------横着展开--------------------------------------*/
// Rectangle {
//     id: root
//     x: Screen.width - expandedWidth - 10
//     y: iconSize * 0.3
//     z: 999

//     // 让 root 的宽度始终等于展开后的总宽度，背景透明，靠内部组件来撑开视觉
//     width: expandedWidth
//     height: iconSize
//     color: "transparent"

//     property int  landSize: Math.min(Screen.width, Screen.height) * 0.3
//     property int  iconSize: landSize * 0.2
//     property bool expanded: false
//     property color backColor: "#d6e6ff"
//     property color pressColor: "#b0d4ff"

//     // 预先计算展开后的总宽度
//     property int expandedWidth: iconSize + row.implicitWidth + 10

//     property bool screenMode:    false
//     property bool locationMode:  false
//     property bool landMarkMode:  false
//     property bool contourMode:   false
//     property bool measureMode:   false
//     property bool bluetoothMode: false
//     property bool polygonMode:   false

//     // 胶囊状的半透明背景：它的宽度跟着 contentRect 动态变长
//     Rectangle {
//         id: bgRect
//         anchors.right: parent.right
//         width: handleBtn.width + contentRect.width
//         height: parent.height
//         radius: height / 2
//         color: "transparent"

//         // 增加平滑动画
//         Behavior on width {
//             NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
//         }
//     }

//     // 主按钮：固定在右侧 (通过 anchors 定位，绝不要用 Screen.width)
//     ToolButton {
//         id: handleBtn
//         anchors.right: parent.right
//         width:  iconSize * 1.1
//         height: iconSize * 1.1

//         // 修改背景使之透明，或配合整体
//         background: Rectangle { color: "white" }

//         contentItem: Image {
//             source: root.expanded ? "qrc/icons/ui/arrow_bar_to_down.svg" : "qrc:/XR/content.svg"
//             width:  iconSize
//             height: iconSize
//             fillMode: Image.PreserveAspectFit
//             anchors.centerIn: parent
//         }

//         onClicked: {
//             root.expanded = !root.expanded
//         }
//     }

//     // 展开区域：靠在主按钮的左边
//     Rectangle {
//         id: contentRect
//         anchors.right: handleBtn.left
//         anchors.verticalCenter: parent.verticalCenter
//         width: root.expanded ? row.implicitWidth + 10 : 0
//         height: parent.height
//         color: "transparent"
//         clip: true

//         Behavior on width {
//             NumberAnimation {
//                 duration: 250
//                 easing.type: Easing.OutCubic
//             }
//         }

//         Column {
//             id: row
//             spacing: 4
//             anchors.verticalCenter: parent.verticalCenter
//             anchors.right: parent.right
//             anchors.rightMargin: 5

//             MenuButton {  // 截图
//                 id: frameSlectBtn
//                 icon.source: "qrc:/XR/screet.png"
//                 icon.width:  root.iconSize
//                 icon.height: root.iconSize
//                 width:       root.iconSize
//                 height:      root.iconSize

//                 CMouseOpacityArea {
//                     toolTipText: qsTr("Screen Shot")
//                 }

//                 Rectangle {
//                     anchors.fill: parent
//                     radius: root.iconSize * 0.5
//                     color: screenMode ? pressColor :backColor
//                 }

//                 onPressed: {
//                     screenMode = !screenMode
//                     renderer.setScreenMode(screenMode)
//                 }

//                 Connections {
//                     target: renderer.screetShot
//                     function onCancelScreetShot() {
//                         screenMode = false
//                     }
//                 }

//             }

//             MenuButton { // 测距
//                 id: measureBtn
//                 icon.source: "qrc:/XR/measuredistance.png"
//                 icon.width:  root.iconSize
//                 icon.height: root.iconSize
//                 width:       root.iconSize
//                 height:      root.iconSize

//                 CMouseOpacityArea {
//                     toolTipText: qsTr("Measure")
//                 }

//                 Rectangle {
//                     anchors.fill: parent
//                     radius: root.iconSize * 0.5
//                     color: measureMode ? pressColor :backColor
//                 }

//                 onPressed: {
//                     measureMode = !measureMode
//                     renderer.setDistMeasureMode(measureMode)
//                 }
//             }

//             MenuButton { // 定位
//                 id: locationBtn
//                 icon.source: "qrc:/XR/location.png"
//                 icon.width:  root.iconSize
//                 icon.height: root.iconSize
//                 width:       root.iconSize
//                 height:      root.iconSize

//                 CMouseOpacityArea {
//                     toolTipText: qsTr("Location")
//                 }

//                 Rectangle {
//                     anchors.fill: parent
//                     radius:  root.iconSize * 0.5
//                     color: backColor
//                 }

//                 onPressed: {
//                     locationMode = !locationMode
//                     Locations.signalShowLocation(locationMode)
//                 }
//             }

//             MenuButton { //兴趣点
//                 id: landMarkBtn
//                 icon.source: "qrc:/XR/pushPin.png"
//                 icon.width:  root.iconSize
//                 icon.height: root.iconSize
//                 width:       root.iconSize
//                 height:      root.iconSize

//                 CMouseOpacityArea {
//                     toolTipText: qsTr("LandMark")
//                 }

//                 Rectangle {
//                     anchors.fill: parent
//                     radius: root.iconSize * 0.5
//                     color: backColor
//                 }

//                 onPressed: {
//                     landMarkMode = !landMarkMode
//                     renderer.setLandMarkMode(landMarkMode)
//                 }
//             }

//             // MenuButton { //等值线
//             //     id: contourBtn
//             //     icon.source: "qrc:/XR/contour.png"
//             //     icon.width:  root.iconSize
//             //     icon.height: root.iconSize
//             //     width:       root.iconSize
//             //     height:      root.iconSize

//             //     CMouseOpacityArea {
//             //         id: btn6
//             //         toolTipText: qsTr("Contours")
//             //     }

//             //     Rectangle {
//             //         anchors.fill: parent
//             //         radius: root.iconSize * 0.5
//             //         color: backColor
//             //     }

//             //     onPressed: {
//             //         contourMode = !contourMode
//             //         if(contourMode) {
//             //             bluetoothMode = false
//             //         }
//             //     }
//             // }

//             // MenuButton {
//             //     id: serialPortBtn
//             //     icon.source: "qrc:/XR/serialPort.png"
//             //     icon.width:  root.iconSize
//             //     icon.height: root.iconSize
//             //     width:       root.iconSize
//             //     height:      root.iconSize

//             //     CMouseOpacityArea {
//             //         toolTipText: qsTr("SerialPort")
//             //     }

//             //     property bool serialPortMode: false
//             //     Rectangle {
//             //         anchors.fill: parent
//             //         radius:  root.iconSize * 0.5
//             //         color: backColor
//             //     }

//             //     onPressed: {
//             //         serialPortMode = !serialPortMode
//             //     }
//             // }

//             // MenuButton {
//             //     id: blueToothBtn
//             //     icon.source: "qrc:/XR/bluetooth.png"
//             //     icon.width:  root.iconSize
//             //     icon.height: root.iconSize
//             //     width:       root.iconSize
//             //     height:      root.iconSize


//             //     CMouseOpacityArea {
//             //         toolTipText: qsTr("Bluetooth")
//             //     }

//             //     Rectangle {
//             //         anchors.fill: parent
//             //         radius:  root.iconSize * 0.5
//             //         color:  backColor
//             //     }

//             //     onPressed: {
//             //         bluetoothMode = !bluetoothMode
//             //         if(bluetoothMode) {
//             //             contourMode = false
//             //         }
//             //     }
//             // }


//             MenuButton {
//                 id: eraseRouteBtn
//                 icon.source: "qrc:/icons/ui/erase.svg"
//                 icon.width:  root.iconSize
//                 icon.height: root.iconSize
//                 width:       root.iconSize
//                 height:      root.iconSize

//                 CMouseOpacityArea {
//                     toolTipText: qsTr("Erase")
//                 }

//                 Rectangle {
//                     anchors.fill: parent
//                     radius:  root.iconSize * 0.5
//                     color:  backColor
//                 }

//                 onPressed: core.clearRouteData()
//             }
//         }
//     }

//     MouseArea {
//         id: hoverArea
//         anchors.fill: bgRect
//         hoverEnabled: true
//         acceptedButtons: Qt.NoButton

//         cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor

//         onEntered: {
//             root.expanded = true
//         }

//         onExited: {
//             root.expanded = false
//         }

//         onPressed: {
//             root.expanded = true
//         }
//     }
// }























/*---------------------------------------竖着展开-----------------------------------------*/
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15


Rectangle {
    id: root
    x: iconSize * 0.5
    y: iconSize * 0.5
    z: 9999

    // 让 root 的宽度始终等于展开后的总宽度，背景透明，靠内部组件来撑开视觉
    width: iconSize
    height: expandedHeight
    color: "transparent"

    property int   landSize: Math.min(Screen.width, Screen.height) * 0.3
    property int   iconSize: landSize * 0.2
    property bool  expanded: false
    property color backColor: "#d6e6ff"
    property color pressColor: "#b0d4ff"

    // 预先计算展开后的总宽度
    property int expandedHeight: iconSize + col.implicitHeight + 10

    property bool screenMode:    false
    property bool locationMode:  false
    property bool landMarkMode:  false
    property bool contourMode:   false
    property bool measureMode:   false
    property bool bluetoothMode: false
    property bool polygonMode:   false

    // 胶囊状的半透明背景：它的宽度跟着 contentRect 动态变长
    Rectangle {
        id: bgRect
        anchors.top: parent.top
        width: parent.width
        height: handleBtn.height + contentRect.height
        radius: width * 0.5
        color: "transparent"

        // 增加平滑动画
        Behavior on height {
            NumberAnimation { duration: 250; easing.type: Easing.OutCubic }
        }
    }


    Timer {
        id: collapseTimer
        interval: 5000
        repeat: false
        onTriggered: {
            if(root.expanded) {
                root.expanded = false
            }
        }
    }

    MenuButton {
        id: handleBtn
        icon.source: root.expanded ? "qrc:/icons/ui/arrow_bar_to_down.svg" : "qrc:/XR/content.svg"
        icon.width:  root.iconSize
        icon.height: root.iconSize
        width:       root.iconSize
        height:      root.iconSize

        Rectangle {
            anchors.fill: parent
            radius: root.iconSize * 0.55
            color: backColor
        }

        onPressed: {
            root.expanded = !root.expanded
            if(root.expanded) {
                collapseTimer.restart()
            }
            else {
                collapseTimer.stop()
            }
        }

    }



    // 展开区域：靠在主按钮的左边
    Rectangle {
        id: contentRect
        anchors.top: handleBtn.bottom
        anchors.topMargin: iconSize * 0.2
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width
        height: root.expanded ? col.implicitHeight + 10 : 0
        // color: "transparent"
        color: "white"
        clip: true
        border.width: 1
        border.color: "#808080"
        radius: width * 0.5

        Behavior on width {
            NumberAnimation {
                duration: 250
                easing.type: Easing.OutCubic
            }
        }

        Column {
            id: col
            spacing: 4
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: 5

            MenuButton {  // 截图
                id: frameSlectBtn
                icon.source: "qrc:/XR/screet.png"
                icon.width:  root.iconSize
                icon.height: root.iconSize
                width:       root.iconSize
                height:      root.iconSize

                CMouseOpacityArea {
                    toolTipText: qsTr("Screen Shot")
                }

                Rectangle {
                    anchors.fill: parent
                    radius: root.iconSize * 0.5
                    color: screenMode ? pressColor :backColor
                }

                onPressed: {
                    screenMode = !screenMode
                    renderer.setScreenMode(screenMode)
                    collapseTimer.restart()
                }

                Connections {
                    target: renderer.screetShot
                    function onCancelScreetShot() {
                        screenMode = false
                    }
                }

            }

            MenuButton { // 测距
                id: measureBtn
                icon.source: "qrc:/XR/measuredistance.png"
                icon.width:  root.iconSize
                icon.height: root.iconSize
                width:       root.iconSize
                height:      root.iconSize

                CMouseOpacityArea {
                    toolTipText: qsTr("Measure")
                }

                Rectangle {
                    anchors.fill: parent
                    radius: root.iconSize * 0.5
                    color: measureMode ? pressColor :backColor
                }

                onPressed: {
                    measureMode = !measureMode
                    renderer.setDistMeasureMode(measureMode)
                    collapseTimer.restart()
                }
            }

            MenuButton { // 定位
                id: locationBtn
                icon.source: "qrc:/XR/location.png"
                icon.width:  root.iconSize
                icon.height: root.iconSize
                width:       root.iconSize
                height:      root.iconSize

                CMouseOpacityArea {
                    toolTipText: qsTr("Location")
                }

                Rectangle {
                    anchors.fill: parent
                    radius:  root.iconSize * 0.5
                    color: backColor
                }

                onPressed: {
                    locationMode = !locationMode
                    Locations.signalShowLocation(locationMode)
                    collapseTimer.restart()
                }
            }

            MenuButton { //兴趣点
                id: landMarkBtn
                icon.source: "qrc:/XR/pushPin.png"
                icon.width:  root.iconSize
                icon.height: root.iconSize
                width:       root.iconSize
                height:      root.iconSize

                CMouseOpacityArea {
                    toolTipText: qsTr("LandMark")
                }

                Rectangle {
                    anchors.fill: parent
                    radius: root.iconSize * 0.5
                    color: backColor
                }

                onPressed: {
                    landMarkMode = !landMarkMode
                    renderer.setLandMarkMode(landMarkMode)
                    collapseTimer.restart()
                }
            }


            MenuButton {
                id: eraseRouteBtn
                icon.source: "qrc:/icons/ui/erase.svg"
                icon.width:  root.iconSize
                icon.height: root.iconSize
                width:       root.iconSize
                height:      root.iconSize

                CMouseOpacityArea {
                    toolTipText: qsTr("Erase")
                }

                Rectangle {
                    anchors.fill: parent
                    radius:  root.iconSize * 0.5
                    color:  backColor
                }

                onPressed: {
                    core.clearRouteData()
                    collapseTimer.restart()
                }
            }
        }
    }

    MouseArea {
        id: hoverArea
        anchors.fill: bgRect
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
        cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor

        // onEntered: {
        //     root.expanded = true
        // }

        // onExited: {
        //     root.expanded = false
        // }

        onPressed: {
            root.expanded = true
        }
    }
}

