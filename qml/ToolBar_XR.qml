import QtQuick 2.15
import QtQuick.Controls 2.15


import Bluetooth 1.0

Rectangle {
    id: toolBar_XR
    objectName:  "toolBar_XR"
    height: 65
    width: parent ? parent.width : 400
    color: "#f5f5f5"
    border.color: "#c0c0c0"


    signal openClicked()
    signal saveClicked()


    property  var    targetPlot:    null
    property  var    lastItem:      menuSettings
    property  bool   is3DVisible:   visible3DButton.checked
    property  bool   is2DVisible:   visible2DButton.checked
    property  int    numPlots:      appSettings.numPlots
    property  bool   syncPlots:     appSettings.syncPlots
    property  int    instruments:   appSettings.instruments
    property  int    settingsWidth: theme.controlHeight*20
    property  string filePath:      devSettings.filePath
    property  bool   extraInfoVis:  appSettings.extraInfoVis
    property  bool   autopilotInfofVis: appSettings.autopilotInfofVis

    property int iconSize: 62

    signal languageChanged(string langStr)
    signal menuBarSettingOpened()
    signal syncPlotEnabled()

    signal blueToothClicked(bool active)

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
        spacing: 2

        MenuButton {  // 截图
            id: frameSlectBtn
            icon.source: "qrc:/XR/screet.png"
            icon.width: toolBar_XR.iconSize
            icon.height: toolBar_XR.iconSize
            width: toolBar_XR.iconSize
            height: toolBar_XR.iconSize

            CMouseOpacityArea {
                id: btn1
                toolTipText: qsTr("Screetshot")
            }

            property bool screenMode: false
            Rectangle {
                anchors.fill: parent
                radius: 2
                color: frameSlectBtn.screenMode ? "#a8d8ff" : btn1.containsMouse ? "#d6e6ff" : "transparent"
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
            icon.width: toolBar_XR.iconSize
            icon.height: toolBar_XR.iconSize
            width: toolBar_XR.iconSize
            height: toolBar_XR.iconSize

            CMouseOpacityArea {
                id: btn2
                toolTipText: qsTr("History Screen")
            }
            Rectangle {
                anchors.fill: parent
                radius: 2
                color: btn2.containsMouse ? "#d6e6ff" : "transparent"
            }

            onPressed: itemChangeActive(historyScreenBtn)
        }

        MenuButton {  // 测距
            id: measureBtn
            icon.source: "qrc:/XR/measuredistance.png"
            icon.width: toolBar_XR.iconSize
            icon.height: toolBar_XR.iconSize
            width: toolBar_XR.iconSize
            height: toolBar_XR.iconSize

            CMouseOpacityArea {
                id: btn3
                toolTipText: qsTr("Measure")
            }
            Rectangle {
                anchors.fill: parent
                radius: 2
                color: btn3.containsMouse ? "#d6e6ff" : "transparent"
            }

            onPressed: itemChangeActive(measureBtn)
        }

        MenuButton { // 定位
            id: locationBtn
            icon.source: "qrc:/XR/location.png"
            icon.width: toolBar_XR.iconSize
            icon.height: toolBar_XR.iconSize
            width: toolBar_XR.iconSize
            height: toolBar_XR.iconSize

            CMouseOpacityArea {
                id: btn4
                toolTipText: qsTr("Location")
            }
            Rectangle {
                anchors.fill: parent
                radius: 2
                color: btn4.containsMouse ? "#d6e6ff" : "transparent"
            }

            onPressed: itemChangeActive(locationBtn)
        }

        MenuButton { //兴趣点
            id: landMarkBtn
            icon.source: "qrc:/XR/pushPin.png"
            icon.width: toolBar_XR.iconSize
            icon.height: toolBar_XR.iconSize
            width: toolBar_XR.iconSize
            height: toolBar_XR.iconSize

            CMouseOpacityArea {
                id: btn5
                toolTipText: qsTr("LandMark")
            }
            Rectangle {
                anchors.fill: parent
                radius: 2
                color: btn5.containsMouse ? "#d6e6ff" : "transparent"
            }

            onPressed: itemChangeActive(landMarkBtn)
        }

        MenuButton { //等值线
            id: contourBtn
            icon.source: "qrc:/XR/contour.png"
            icon.width: toolBar_XR.iconSize
            icon.height: toolBar_XR.iconSize
            width: toolBar_XR.iconSize
            height: toolBar_XR.iconSize

            CMouseOpacityArea {
                id: btn6
                toolTipText: qsTr("Contours")
            }
            Rectangle {
                anchors.fill: parent
                radius: 2
                color: btn6.containsMouse ? "#d6e6ff" : "transparent"
            }

            onPressed: itemChangeActive(contourBtn)
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
            width: toolBarXR.width - x - 350
            height: 1
        }

        MenuButton {
            id: serialPortBtn
            icon.source: "qrc:/XR/serialPort.png"
            icon.width: toolBar_XR.iconSize
            icon.height: toolBar_XR.iconSize
            width: toolBar_XR.iconSize
            height: toolBar_XR.iconSize

            CMouseOpacityArea {
                toolTipText: qsTr("SerialPort")
                onContainsMouseChanged: hoverBackgroundColor
            }
            onPressed: itemChangeActive(serialPortBtn)
        }

        MenuButton {
            id: blueToothBtn
            icon.source: "qrc:/XR/bluetooth.png"
            icon.width: toolBar_XR.iconSize
            icon.height: toolBar_XR.iconSize
            width: toolBar_XR.iconSize
            height: toolBar_XR.iconSize

            CMouseOpacityArea {
                toolTipText: qsTr("Bluetooth")
                onContainsMouseChanged: hoverBackgroundColor
            }

            onPressed: {
                itemChangeActive(blueToothBtn)
                blueToothBtn = !blueToothBtn
                blueToothClicked(blueToothBtn.active)
            }
        }

        Image {
            id: toslonLogo
            source: "qrc:/XR/TOSLON2.png"
            fillMode: Image.PreserveAspectFit
            width: 200
            height: 60
            anchors.verticalCenter: parent.verticalCenter
            ToolTip.visible: containsMouse
            ToolTip.text: qsTr("TOSLON")

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor

                onClicked: {
                    // 点击事件
                }
            }
        }


    }



    IsobathsExtraSettings {
        id: isobathsSet
        isobathsCheckButton: isobathsCheckButton
        visible: contourBtn.active
        // anchors.bottom:           isobathsCheckButton.top
        // anchors.horizontalCenter: isobathsCheckButton.horizontalCenter
        x: contourBtn.x
        y: 62
        z: 2
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

    BleLivedataScanning {
        id: bleLivedata
        visible: blueToothBtn.active
        x: parent.width - width - 10
        y: 62

        onVisibleChanged: {
            BleManager.setBleLiveScanningVisible(visible)
            MapViewControlMenuController.onUpdateClicked()
        }
    }


}


