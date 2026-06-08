import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15


Rectangle {
    id: toolBar_XR
    objectName:  "toolBar_XR"
    height: iconSize + 3
    color:  Qt.platform.os === "android" ? "#828282" : "#eeeeee"
    border.color: "#c0c0c0"

    signal openClicked()
    signal saveClicked()

    property  var  menuPopup:     null
    property  var  targetPlot:    null
    property  var  lastItem:      menuSettings
    property  int  settingsWidth: theme.menuWidth*20
    property  int  iconSize:      Math.min(Screen.width, Screen.height) * 0.05

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

    function closeMenus() {
        if (menuSettings.active) {
            menuSettings.active = false
        }
        if (menuDisplay.active) {
            menuDisplay.active = false
        }
    }

    function itemChangeActive(currentItem) {
           let wasOpen = currentItem.active
           let lastItemTmp = lastItem

           if (currentItem) {
               currentItem.active = !(currentItem.active)
           }

           if (lastItem && lastItem !== currentItem) {
               lastItem.active = false
           }

           lastItem = currentItem

           if (!wasOpen && currentItem.active && (currentItem === menuSettings || currentItem === menuDisplay)) {
               menuBarSettingOpened()
           }
    }


    Loader {
        anchors.fill: parent
        sourceComponent: Qt.platform.os === "android" ? androidRowComponent : windowsRowComponent
        // sourceComponent: Qt.platform.os === "android" ? windowsRowComponent : androidRowComponent
    }


    Component
    {
        id: windowsRowComponent

        Row {
            anchors.fill: parent

            Row {
                anchors.fill: parent
                anchors.margins: 2
                spacing: 3


            MenuButton {  //菜单
                id: menuBtn
                icon.source: "qrc:/XR/config.png"
                icon.width:  toolBar_XR.iconSize
                icon.height: toolBar_XR.iconSize
                width:       toolBar_XR.iconSize
                height:      toolBar_XR.iconSize

                Rectangle {
                    anchors.fill: parent
                    radius: 2
                }

                onPressed: {
                    if(menuPopup) {
                        if (menuPopup.visible) {
                            menuPopup.close()
                        }
                        else {
                            menuPopup.open()
                            contourMode = false
                            bluetoothMode = false
                        }
                    }
                }
            }

            // 垂直分割线
            Rectangle {
                width: 2
                height: parent.height * 0.8
                color: "#d0d0d0"
                anchors.verticalCenter: parent.verticalCenter
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
                anchors.right: parent.right
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

        Row {
            anchors.fill: parent
            anchors.margins: 2
            spacing: 3


            MenuButton {  //菜单
                id: menuBtn
                icon.source: "qrc:/XR/config.png"
                icon.width:  toolBar_XR.iconSize
                icon.height: toolBar_XR.iconSize
                width:       toolBar_XR.iconSize
                height:      toolBar_XR.iconSize

                Rectangle {
                    anchors.fill: parent
                    radius: 2
                    color: contourMode ? backColor : "transparent"
                }

                onPressed: {
                    if(menuPopup) {
                        if (menuPopup.visible) {
                            menuPopup.close()
                        }
                        else {
                            menuPopup.open()
                            contourMode   = false
                            bluetoothMode = false
                        }
                    }
                }
            }

            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 5

                // 系统时间
                Text {
                    id: systemTime
                    font.pixelSize: toolBar_XR.iconSize * 0.9
                    color: "white"
                    text: Qt.formatDateTime(new Date(), "HH:mm")
                }

                // 电量图标和百分比
                Item {
                    id: batteryStatus

                    width: toolBar_XR.iconSize * 2.8
                    height: toolBar_XR.iconSize * 0.8
                    anchors.verticalCenter: parent.verticalCenter


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

                            color: theme.batteryValue > 0.2 ? "#4CAF50" : "#F44336"

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
                    text: theme.batteryValue + "%"
                    color: "white"
                    font.pixelSize: toolBar_XR.iconSize * 0.55
                    verticalAlignment: Text.AlignVCenter
                    anchors.verticalCenter: parent.verticalCenter
                }

                // 网络信号强度
                Item {
                    id: networkStatus

                    width: toolBar_XR.iconSize * 1.2
                    height: toolBar_XR.iconSize
                    anchors.verticalCenter: parent.verticalCenter


                    property int signalLevel: 3   // 0~4

                    Row {
                        anchors.bottom: parent.bottom
                        spacing: 1

                        Repeater {
                            model: 4

                            Rectangle {
                                width: toolBar_XR.iconSize * 0.18
                                height: (index + 1) * toolBar_XR.iconSize * 0.22

                                anchors.bottom: parent.bottom

                                radius: 1

                                color: index < signalLevel
                                       ? "#4CAF50"
                                       : "#808080"
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

}



