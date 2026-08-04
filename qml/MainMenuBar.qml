import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Qt.labs.settings 1.1


import AppXr 1.0

//左上方的工具栏
Item {
    id: menu
    implicitWidth: menuLayout.width

    property  var    targetPlot:    null
    // property  var    lastItem:      menuSettings
    // property  bool   is3DVisible:   visible3DButton.checked
    // property  bool   is2DVisible:   visible2DButton.checked
    property bool is3DVisible:  true
    property bool is2DVisible:  true
    property  int    numPlots:      appSettings.numPlots
    property  bool   syncPlots:     appSettings.syncPlots
    property  int    instruments:   appSettings.instruments
    property  int    settingsWidth: theme.menuWidth*20
    property  string filePath:      devSettings.filePath
    property  bool   extraInfoVis:  appSettings.extraInfoVis
    property  bool   autopilotInfofVis: appSettings.autopilotInfofVis

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
        // let lastItemTmp = lastItem

        //控制DeviceSettingsViewer、DisplaySettingsViewer的显示和隐藏
        if (currentItem) {
            currentItem.active = !(currentItem.active)
        }

        // if (lastItem && lastItem !== currentItem) {
        //     lastItem.active = false
        // }

        // lastItem = currentItem

        if (!wasOpen && currentItem.active && (currentItem === menuSettings || currentItem === menuDisplay)) {
            menuBarSettingOpened()
        }
    }

    RowLayout {
        id: menuLayout
        spacing: 0

        ColumnLayout {
            id: mainLayout
            Layout.alignment: Qt.AlignTop
            Layout.preferredWidth: theme.menuWidth
            Layout.topMargin: 6
            spacing: 4
            Layout.margins: 4

            Component.onCompleted: {
                resetButtonOpacity()
            }

            function highlightAllButtons() {
                mainLayout.opacity = 1
            }

            function resetButtonOpacity() {
                mainLayout.opacity = 0.5
            }

            MenuButton {
                id: menuSettings
                icon.source: "qrc:/icons/ui/plug.svg"
                Layout.fillWidth: true
                CMouseOpacityArea {
                    toolTipText: qsTr("Connections")
                    onContainsMouseChanged: containsMouse ? mainLayout.highlightAllButtons() : mainLayout.resetButtonOpacity()
                }

                onPressed: {
                    itemChangeActive(menuSettings)
                }
            }

        //     CheckButton {
        //         id: visible3DButton
        //         implicitWidth: theme.menuWidth
        //         icon.source: "qrc:/icons/ui/map.svg"
        //         backColor: theme.controlBackColor
        //         borderColor:  theme.controlBackColor
        //         checkedBorderColor: "black"
        //         checked: true

        //         CMouseOpacityArea {
        //             toolTipText: qsTr("Display 3D")
        //             onContainsMouseChanged: containsMouse ? mainLayout.highlightAllButtons() : mainLayout.resetButtonOpacity()
        //         }

        //         onClicked: {
        //             if (!visible3DButton.checked && !visible2DButton.checked) {
        //                 visible2DButton.checked = true
        //             }
        //         }

        //         Settings {
        //             id: visible3DSettings
        //             property alias visible3DButtonChecked: visible3DButton.checked
        //         }
        //     }

        //     CheckButton {
        //         id: visible2DButton
        //         implicitWidth: theme.menuWidth
        //         icon.source: "qrc:/icons/ui/ripple.svg"
        //         backColor: theme.controlBackColor
        //         borderColor:  theme.controlBackColor
        //         checkedBorderColor: "black"
        //         checked: false

        //         CMouseOpacityArea {
        //             toolTipText: qsTr("Display 2D")
        //             onContainsMouseChanged: containsMouse ? mainLayout.highlightAllButtons() : mainLayout.resetButtonOpacity()
        //         }

        //         // onClicked: {
        //         //     if (!visible3DButton.checked && !visible2DButton.checked) {
        //         //         visible3DButton.checked = true
        //         //     }
        //         // }

        //         // Settings {
        //         //     id: visible2DSettings
        //         //     property alias visible2DButtonChecked: visible2DButton.checked
        //         // }
        //     }

        }


        // 左上角第一个按钮（串口连接、文件导入等）
        DeviceSettingsViewer {
            id: devSettings
            // visible: menuSettings.active
            visible: false
            Layout.maximumHeight: menu.height
            menuWidth: settingsWidth
            y:0
        }

        // 左上角第二个按钮settings按钮显示出来
        DisplaySettingsViewer {
            id: appSettings
            Layout.alignment: Qt.AlignTop
            // visible: menuDisplay.active
            visible: false
            Layout.maximumHeight: menu.height
            menuWidth: settingsWidth
            y:0
            targetPlot: menu.targetPlot
        }

    }

    function handleChildSignal(langStr) {
        languageChanged(langStr)
    }

    function handleSyncPlotEnabled() {
        syncPlotEnabled()
    }

    Component.onCompleted: {
        appSettings.languageChanged.connect(handleChildSignal)
        appSettings.syncPlotEnabled.connect(handleSyncPlotEnabled)
    }
}
