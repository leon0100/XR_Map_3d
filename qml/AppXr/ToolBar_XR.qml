import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15


Rectangle {
    id: toolBar_XR
    objectName:  "toolBar_XR"
    height: iconSize + 3
    color: "#eeeeee"
    border.color: "#c0c0c0"

    signal openClicked()
    signal saveClicked()

    property  var    menuPopup:     null
    property  var    targetPlot:    null
    property  var    lastItem:      menuSettings
    property  int    settingsWidth: theme.menuWidth*20
    property  int    iconSize:      Math.min(Screen.width, Screen.height) * 0.05

    property color backColor: "#d6e6ff"

    property bool contourMode: false
    property bool bluetoothMode: false
    property bool polygonMode: false

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

            // property bool configMode: false
            // Rectangle {
            //     anchors.fill: parent
            //     radius: 2
            //     color: menuBtn.configMode ? backColor : "transparent"
            // }

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


            property bool screenMode: false
            Rectangle {
                anchors.fill: parent
                radius: 2
                color: frameSlectBtn.screenMode ? backColor : "transparent"
            }

            onPressed: {
                screenMode = !screenMode
                renderer.setScreenMode(screenMode)
            }

            Connections {
                target: renderer.screetShot
                function onCancelScreetShot() {
                    frameSlectBtn.screenMode = false
                }
            }
        }

        MenuButton {  // 历史截图
            id: historyScreenBtn
            icon.source: "qrc:/XR/historyScreen.png"
            icon.width:  toolBar_XR.iconSize
            icon.height: toolBar_XR.iconSize
            width:       toolBar_XR.iconSize
            height:      toolBar_XR.iconSize


            property bool historyScreenMode: false
            Rectangle {
                anchors.fill: parent
                radius: 2
                color: historyScreenBtn.historyScreenMode ? backColor : "transparent"
            }

            onPressed: {
                historyScreenMode = !historyScreenMode
            }
        }

        MenuButton {  // 测距
            id: measureBtn
            icon.source: "qrc:/XR/measuredistance.png"
            icon.width:  toolBar_XR.iconSize
            icon.height: toolBar_XR.iconSize
            width:       toolBar_XR.iconSize
            height:      toolBar_XR.iconSize

            // CMouseOpacityArea {
            //     id: btn3
            //     toolTipText: qsTr("Measure")
            // }

            property bool measureMode: false
            Rectangle {
                anchors.fill: parent
                radius: 2
                color: measureBtn.measureMode ? backColor : "transparent"
            }

            onPressed: {
                measureMode = !measureMode
            }
        }

        MenuButton { // 定位
            id: locationBtn
            icon.source: "qrc:/XR/location.png"
            icon.width:  toolBar_XR.iconSize
            icon.height: toolBar_XR.iconSize
            width:       toolBar_XR.iconSize
            height:      toolBar_XR.iconSize

            // CMouseOpacityArea {
            //     id: btn4
            //     toolTipText: qsTr("Location")
            // }
            property bool locationMode: false
            Rectangle {
                anchors.fill: parent
                radius: 2
                color: locationBtn.locationMode ? backColor : "transparent"
            }

            onPressed: {
                locationMode = !locationMode
            }
        }

        MenuButton { //兴趣点
            id: landMarkBtn
            icon.source: "qrc:/XR/pushPin.png"
            icon.width:  toolBar_XR.iconSize
            icon.height: toolBar_XR.iconSize
            width:       toolBar_XR.iconSize
            height:      toolBar_XR.iconSize

            CMouseOpacityArea {
                id: btn5
                toolTipText: qsTr("LandMark")
            }
            property bool landMarkMode: false
            Rectangle {
                anchors.fill: parent
                radius: 2
                color: landMarkBtn.landMarkMode ? backColor : "transparent"
            }

            onPressed: {
                landMarkMode = !landMarkMode
            }
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

            Rectangle {
                anchors.fill: parent
                radius: 2
                color: contourMode ? backColor : "transparent"
            }

            onPressed: {
                contourMode = !contourMode
                if(contourMode) {
                    bluetoothMode = false
                }
            }
        }



        // MenuButton {
        //     id: menuSettings
        //     icon.source: "qrc:/icons/ui/plug.svg"
        //     icon.width: toolBar_XR.iconSize
        //     icon.height: toolBar_XR.iconSize
        //     width: toolBar_XR.iconSize
        //     height: toolBar_XR.iconSize

        //     CMouseOpacityArea {
        //         toolTipText: qsTr("Connections")
        //     }

        //     onPressed: itemChangeActive(menuSettings)
        // }

        // MenuButton {
        //     id: menuDisplay
        //     icon.source: "./settings-outline.svg"
        //     icon.width: toolBar_XR.iconSize
        //     icon.height: toolBar_XR.iconSize
        //     width: toolBar_XR.iconSize
        //     height: toolBar_XR.iconSize

        //     CMouseOpacityArea {
        //         toolTipText: qsTr("Settings")
        //         onContainsMouseChanged: hoverBackgroundColor
        //     }

        //     onPressed: itemChangeActive(menuDisplay)
        // }

        Item {
            width: toolBarXR.width - x - toolBar_XR.iconSize*5.2
            height: 1
        }

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

    }




    // //（串口连接、文件导入等）
    // DeviceSettingsViewer {
    //     id: devSettings
    //     visible: menuSettings.active
    //     menuWidth: settingsWidth
    //     x: 8
    //     y: 62
    // }

    // //settings按钮显示出来
    // DisplaySettingsViewer {
    //     id: appSettings
    //     visible: menuDisplay.active
    //     menuWidth: settingsWidth
    //     x: 8
    //     y: 62
    //     targetPlot: toolBar_XR.targetPlot
    // }


}


