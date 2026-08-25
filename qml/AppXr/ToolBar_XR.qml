import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import "../"

Rectangle {
    id: toolBar_XR
    objectName:  "toolBar_XR"
    height: iconSize + 3
    color: "#828282"
    border.color: "#c0c0c0"

    signal openClicked()
    signal saveClicked()

    property  var  menuPopup:     null
    property  var  targetPlot:    null
    property  var  expandBar:     null
    // property  var  lastItem:      menuSettings
    property  int  settingsWidth: theme.menuWidth  * 20
    property  int  iconSize:      theme.screenSize * 0.05

    property color backColor: "#d6e6ff"

    property bool screenMode:    false
    property bool locationMode:  false
    property bool landMarkMode:  false
    property bool contourMode:   false
    property bool measureMode:   false
    property bool bluetoothMode: false
    property bool polygonMode:   false

    signal languageChanged(string langStr)
    signal menuBarSettingOpened()
    signal syncPlotEnabled()


    function updateBottomTrack() {
        appSettings.updateBottomTrack()
    }

    function clickConnections() {
        itemChangeActive(menuSettings)
    }

    function clickSettings() {
        itemChangeActive(menuDisplay)
    }

    function click2D() {
        visible2DButton.checked = !visible2DButton.checked;
        visible2DButton.clicked()
    }

    function click3D() {
        visible3DButton.checked = !visible3DButton.checked;
        visible3DButton.clicked()
    }

    function itemChangeActive(currentItem) {
           let wasOpen = currentItem.active

           if (currentItem) {
               currentItem.active = !(currentItem.active)
           }

           if (!wasOpen && currentItem.active && (currentItem === menuSettings || currentItem === menuDisplay)) {
               menuBarSettingOpened()
           }
    }

    MouseArea {
        anchors.fill: parent
        onPressed: {
            if(targetPlot) {
                targetPlot.closeEchoBathyIsobathOutside(0, 0)
                scene3DToolbar.closeIsobathsOutside(0, 0)
            }
            expandBar.expanded = false
        }
    }


    Loader {
        anchors.fill: parent
        // sourceComponent: Qt.platform.os === "android" ? androidRowComponent : windowsRowComponent
        sourceComponent: androidRowComponent
    }

    Component
    {
        id: windowsRowComponent

        RowLayout {
            anchors.fill: parent

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 3

            // MenuButton {  //菜单
            //     // id: menuBtn
            //     icon.source: "qrc:/XR/config.png"
            //     icon.width:  toolBar_XR.iconSize
            //     icon.height: toolBar_XR.iconSize
            //     width:       toolBar_XR.iconSize
            //     height:      toolBar_XR.iconSize

            //     Rectangle {
            //         anchors.fill: parent
            //         radius: 2
            //     }

            //     onPressed: {
            //         if(menuPopup) {
            //             if (menuPopup.visible) {
            //                 // menuPopup.close()
            //                 menuPopup.resuqestColoseMenu()
            //             }
            //             else {
            //                 menuPopup.open()
            //                 contourMode = false
            //                 bluetoothMode = false
            //             }
            //         }
            //     }
            // }

            XRButton {
                id: menuBtn
                iconSource: "qrc:/XR/config.png"
                checkable: false
                Layout.preferredWidth:  toolBar_XR.iconSize
                Layout.preferredHeight: toolBar_XR.iconSize

                clickAction: function() {
                    if (targetPlot) {
                        if(menuPopup) {
                            if (menuPopup.visible) {
                                // menuPopup.close()
                                menuPopup.resuqestColoseMenu()
                            }
                            else {
                                menuPopup.open()
                                contourMode = false
                                bluetoothMode = false
                            }
                        }
                    }
                }
            }

            // 垂直分割线
            Rectangle {
                width: 2
                height: parent.height * 0.8
                color: "#d0d0d0"
                Layout.alignment: Qt.AlignVCenter
            }

            MenuButton {  // 截图
                id: frameSlectBtn
                icon.source: "qrc:/XR/screet.png"
                icon.width:  toolBar_XR.iconSize
                icon.height: toolBar_XR.iconSize
                width:       toolBar_XR.iconSize
                height:      toolBar_XR.iconSize

                CMouseOpacityArea {
                    toolTipText: qsTr("Screen Shot")
                }

                Rectangle {
                    anchors.fill: parent
                    radius: 2
                    color: screenMode ? backColor : "transparent"
                }

                onPressed: {
                    screenMode = !screenMode
                    renderer.setScreenMode(screenMode)
                }

                Connections {
                    target: renderer.screetShot
                    function onCancelScreetShot() {
                        screenMode = false
                    }
                }
            }

            // MenuButton {  // 历史截图
            //     id: historyScreenBtn
            //     icon.source: "qrc:/XR/historyScreen.png"
            //     icon.width:  toolBar_XR.iconSize
            //     icon.height: toolBar_XR.iconSize
            //     width:       toolBar_XR.iconSize
            //     height:      toolBar_XR.iconSize


            //     property bool historyScreenMode: false
            //     Rectangle {
            //         anchors.fill: parent
            //         radius: 2
            //         color: historyScreenBtn.historyScreenMode ? backColor : "transparent"
            //     }

            //     onPressed: {
            //         historyScreenMode = !historyScreenMode
            //     }
            // }

            MenuButton {  // 测距
                id: measureBtn
                icon.source: "qrc:/XR/measuredistance.png"
                icon.width:  toolBar_XR.iconSize
                icon.height: toolBar_XR.iconSize
                width:       toolBar_XR.iconSize
                height:      toolBar_XR.iconSize

                CMouseOpacityArea {
                    toolTipText: qsTr("Measure")
                }

                Rectangle {
                    anchors.fill: parent
                    radius: 2
                    color: measureMode ? backColor : "transparent"
                }

                onPressed: {
                    measureMode = !measureMode
                    renderer.setDistMeasureMode(measureMode)
                }

            }

            MenuButton { // 定位
                id: locationBtn
                icon.source: "qrc:/XR/location.png"
                icon.width:  toolBar_XR.iconSize
                icon.height: toolBar_XR.iconSize
                width:       toolBar_XR.iconSize
                height:      toolBar_XR.iconSize

                CMouseOpacityArea {
                    toolTipText: qsTr("Location")
                }

                // Rectangle {
                //     anchors.fill: parent
                //     radius: 2
                //     color: locationMode ? backColor : "transparent"
                // }

                onPressed: {
                    locationMode = !locationMode
                    Locations.signalShowLocation(locationMode)
                }

                // Connections {
                //     target: Locations
                //     function onSignalShowLocation(show) {
                //         locationMode = show
                //     }
                // }
            }

            MenuButton { //兴趣点
                id: landMarkBtn
                icon.source: "qrc:/XR/pushPin.png"
                icon.width:  toolBar_XR.iconSize
                icon.height: toolBar_XR.iconSize
                width:       toolBar_XR.iconSize
                height:      toolBar_XR.iconSize

                CMouseOpacityArea {
                    toolTipText: qsTr("LandMark")
                }

                // Rectangle {
                //     anchors.fill: parent
                //     radius: 2
                //     color: landMarkMode ? backColor : "transparent"
                // }

                onPressed: {
                    landMarkMode = !landMarkMode
                    renderer.setLandMarkMode(landMarkMode)
                }

                // Connections {
                //     target: renderer.screetShot
                //     function onCloseLandMark() {
                //         landMarkMode = false;
                //     }
                // }
            }

            MenuButton { //等值线
                id: contourBtn
                icon.source: "qrc:/XR/contour.png"
                icon.width:  toolBar_XR.iconSize
                icon.height: toolBar_XR.iconSize
                width:       toolBar_XR.iconSize
                height:      toolBar_XR.iconSize

                CMouseOpacityArea {
                    id: btn6
                    toolTipText: qsTr("Contours")
                }

                // Rectangle {
                //     anchors.fill: parent
                //     radius: 2
                //     color: contourMode ? backColor : "transparent"
                // }

                onPressed: {
                    contourMode = !contourMode
                    if(contourMode) {
                        bluetoothMode = false
                    }
                }
            }

        }



            Row {
                Layout.alignment: Qt.AlignRight
                spacing: 3

                MenuButton {
                    id: serialPortBtn
                    icon.source: "qrc:/XR/serialPort.png"
                    icon.width:  toolBar_XR.iconSize
                    icon.height: toolBar_XR.iconSize
                    width:       toolBar_XR.iconSize
                    height:      toolBar_XR.iconSize

                    CMouseOpacityArea {
                        toolTipText: qsTr("SerialPort")
                    }

                    property bool serialPortMode: false
                    Rectangle {
                        anchors.fill: parent
                        radius: 2
                        color: serialPortBtn.serialPortMode ? backColor : "transparent"
                    }

                    onPressed: {
                        serialPortMode = !serialPortMode
                    }
                }

                MenuButton {
                    id: blueToothBtn
                    icon.source: "qrc:/XR/bluetooth.png"
                    icon.width:  toolBar_XR.iconSize
                    icon.height: toolBar_XR.iconSize
                    width:       toolBar_XR.iconSize
                    height:      toolBar_XR.iconSize

                    CMouseOpacityArea {
                        toolTipText: qsTr("Bluetooth")
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: 2
                        color: bluetoothMode ? backColor : "transparent"
                    }

                    onPressed: {
                        bluetoothMode = !bluetoothMode
                        if(bluetoothMode) {
                            contourMode = false
                        }
                    }
                }

                Item {
                    width: 5
                    height: 1
                }

                Image {
                    id: toslonLogo
                    source: "qrc:/XR/TOSLON2.png"
                    fillMode: Image.PreserveAspectFit
                    width:  toolBar_XR.iconSize * 3
                    height: toolBar_XR.iconSize
                    anchors.verticalCenter: parent.verticalCenter
                    ToolTip.visible: containsMouse
                    ToolTip.text: qsTr("TOSLON")

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked:   Qt.openUrlExternally("https://www.Toslon.com")
                    }
                }

                Item {
                    width: 3
                    height: 1
                }

            }

        }

    }


    Component
    {
        id: androidRowComponent

        Item {
            anchors.fill: parent

            // MenuButton {  //菜单
            //     id: menuBtn
            //     anchors.left: parent.left
            //     anchors.leftMargin: 2
            //     anchors.verticalCenter: parent.verticalCenter
            //     icon.source: "qrc:/XR/config.png"
            //     icon.width:  toolBar_XR.iconSize
            //     icon.height: toolBar_XR.iconSize
            //     width:       toolBar_XR.iconSize
            //     height:      toolBar_XR.iconSize

            //     Rectangle {
            //         anchors.fill: parent
            //         radius: 2
            //         color: contourMode ? backColor : "transparent"
            //     }

            //     onPressed: {
            //         targetPlot.closeEchoBathyIsobathOutside(0, 0)
            //         if(menuPopup) {
            //             if (menuPopup.menuVisible) {
            //                 menuPopup.close()
            //             }
            //             else {
            //                 menuPopup.open()
            //                 contourMode   = false
            //                 bluetoothMode = false
            //             }
            //         }

            //         if(targetPlot) {
            //             targetPlot.closeEchoBathyIsobathOutside(0, 0)
            //         }
            //         expandBar.expanded = false
            //     }
            // }
            XRButton {
                id: menuBtn
                anchors.left: parent.left
                anchors.leftMargin: 2
                anchors.verticalCenter: parent.verticalCenter
                iconSource: "qrc:/XR/config.png"
                checkable: false
                border.width: 0
                normalColor: "transparent"
                radius: 0
                width:  toolBar_XR.iconSize
                height: toolBar_XR.iconSize

                clickAction: function() {
                    if(menuPopup) {
                        if (menuPopup.menuVisible) {
                            menuPopup.close()
                        }
                        else {
                            menuPopup.open()
                            contourMode   = false
                            bluetoothMode = false
                        }
                    }

                    if(targetPlot) {
                        targetPlot.closeEchoBathyIsobathOutside(0, 0)
                        scene3DToolbar.closeIsobathsOutside(0, 0)
                    }
                    expandBar.expanded = false
                }
            }

            Row {
                anchors.right: parent.right
                anchors.rightMargin: 5
                anchors.verticalCenter: parent.verticalCenter
                spacing: 5

                // 系统时间
                Text {
                    font.pixelSize: toolBar_XR.iconSize * 0.9
                    color: "white"
                    text: theme.updateSystemTime
                }

                // 电量图标和百分比
                Item {
                    id: batteryStatus
                    width: toolBar_XR.iconSize * 2.8
                    height: toolBar_XR.iconSize * 0.8
                    anchors.verticalCenter: parent.verticalCenter
                    visible: Qt.platform.os === "android"

                    Rectangle {
                        id: batteryBody
                        width: parent.width * 0.85
                        height: parent.height
                        border.width: 1
                        border.color: "white"
                        color: "transparent"
                        radius: 2

                        Rectangle {
                            anchors.left: parent.left
                            anchors.top: parent.top
                            anchors.bottom: parent.bottom
                            anchors.margins: 1
                            width: (parent.width - 2) * theme.batteryValue
                            color: theme.batteryValue > 0.2 ? "#00C853" : "#F44336"
                            radius: 1
                        }
                    }

                    Rectangle {
                        width: parent.width * 0.08
                        height: parent.height * 0.5
                        anchors.left: batteryBody.right
                        anchors.leftMargin: 1
                        anchors.verticalCenter: batteryBody.verticalCenter
                        color: "white"
                        radius: 1
                    }
                }

                Text {
                    text: Math.floor(theme.batteryValue * 100.0) + "%"
                    color: "white"
                    font.pixelSize: toolBar_XR.iconSize * 0.55
                    verticalAlignment: Text.AlignVCenter
                    anchors.verticalCenter: parent.verticalCenter
                    visible: Qt.platform.os === "android"
                }

                // 网络信号强度
                Item {
                    id: networkStatus
                    width:  toolBar_XR.iconSize * 1.2
                    height: toolBar_XR.iconSize
                    anchors.verticalCenter: parent.verticalCenter

                    Row {
                        anchors.bottom: parent.bottom
                        spacing: 1

                        Repeater {
                            model: 4

                            Rectangle {
                               width: toolBar_XR.iconSize * 0.18
                               height: (index + 1) * toolBar_XR.iconSize * 0.2
                               anchors.bottom: parent.bottom
                               radius: 1
                               color: index < theme.systemNetStatus ? "#00C853" : "#808080"
                            }
                        }
                    }
                }

                Image {
                    id: toslonLogo
                    source: "qrc:/XR/TOSLON2.png"
                    fillMode: Image.PreserveAspectFit
                    width:  toolBar_XR.iconSize * 3
                    height: toolBar_XR.iconSize
                    anchors.verticalCenter: parent.verticalCenter
                    // ToolTip.visible: toslonLogo.containsMouse
                    ToolTip.text: qsTr("TOSLON")

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked:  {
                            Qt.openUrlExternally("https://www.Toslon.com")

                            if(targetPlot) {
                                targetPlot.closeEchoBathyIsobathOutside(0, 0)
                            }
                            scene3DToolbar.closeIsobathsOutside(0, 0)
                            expandBar.expanded = false
                        }
                    }
                }

                Item {
                    width: 3
                    height: 1
                }
            }

        }

    }

}



