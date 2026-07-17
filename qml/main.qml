import QtQuick 2.15
import SceneGraphRendering 1.0
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.15
import Qt.labs.settings 1.1

import WaterFall 1.0
import BottomTrack 1.0
import QmlCommon 1.0
import Communication 1.0
import AppXr 1.0


ApplicationWindow  {
    id:        mainview

    visible: true
    width:  Screen.width * 0.5
    minimumWidth: 512
    height: Screen.height * 0.5
    minimumHeight: 256
    visibility: Window.FullScreen
    color:      "black"
    title:      qsTr("XR-Viewer")

    readonly property int _rightBarWidth:                360
    readonly property int _activeObjectParamsMenuHeight: 500
    readonly property int _sceneObjectsListHeight:       300
    readonly property int screenSize: theme.screenSize
    readonly property int footHeight: screenSize * 0.02
    readonly property int iconSize:   footHeight * 0.5

    Settings {
        id: appSettings
        property bool isFullScreen: false
    }

    Loader {
        id: stateGroupLoader
        active: Qt.platform.os === "windows"
        sourceComponent: stateGroupComp
    }

    MenuBar_XR {
        id: menuToolBar
    }

    header: Item {
        id: headerContainer
        implicitHeight: toolBarExpanded ? toolBarXR.height: 0
        clip: true

        property bool toolBarExpanded: true

        Behavior on implicitHeight {
            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
        }

        ToolBar_XR {
            id: toolBarXR
            width: parent.width
            anchors.top: parent.top
            Component.onCompleted: {
                toolBarXR.menuPopup  = menuToolBar
                toolBarXR.targetPlot = waterViewFirst  //把qPlot2D类与ToolBar_XR绑定
            }
        }
    }

    ExpandToolBar {
        id: expandToolBar
        visible: visualisationLayout.splitMode !== 1
    }


    CollapseRectangle {
        id: collapseBar
        anchors.top: expandSate ? headerContainer.bottom : parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        property bool expandSate: true
        isExpanded: expandSate
        onToggleClicked: {
            headerContainer.toolBarExpanded = state
            collapseBar.expandSate = state
        }
    }


    DistMeasure { }

    LocationsQml { }

    LandMarks { }

    LandMarkPoint { }

    IsobathsExtraSettings {
        // visible: toolBarXR.contourMode
        visible: expandToolBar.contourMode
        x: toolBarXR.iconSize * 3.5
        targetPlot: toolBarXR.targetPlot
    }

    BleLivedataScanning {
        id: bleLivedataScaning
        // visible: toolBarXR.bluetoothMode
        visible: expandToolBar.bluetoothMode
    }

    BleDataPanel {
       visible: bleLivedataScaning.isShowDataPanel
    }

    MapTileLoad {
       id: mapTileLoad
    }

    LiveData {

    }

    ExportAsData {

    }

    IsobathsSet {
        id: isobathsSet
        visible: true
        x: 2
        y: expandToolBar.iconSize * 4
        targetPlot: toolBarXR.targetPlot
    }

    Connections {
        target: mapTileLoad

        function onUpdateMapCheck(value) {
            menuToolBar.receiveMapCheck(value)
        }
    }


    Component {
        id: stateGroupComp
        StateGroup {
            state: appSettings.isFullScreen ? "FullScreen" : "Windowed"

            states: [
                State {
                    name: "FullScreen"
                    StateChangeScript {
                        script: { // empty
                        }
                    }
                    PropertyChanges {
                        target: mainview
                        visibility: "FullScreen"

                        flags: Qt.FramelessWindowHint
                        x: 0
                        y: - 1
                        width: Screen.width
                        height: Screen.height + 1
                    }
                },
                State {
                    name: "Windowed"
                    StateChangeScript {
                        script: {
                            if (Qt.platform.os !== "android") {
                                mainview.flags = Qt.Window
                            }
                        }
                    }
                    PropertyChanges {
                        target: mainview
                        visibility: "Windowed"
                    }
                }

            ]
        }
    }

    KProgress {
        id: fileProgress
        showCancelButton: true
    }



    function setFullScreenMode(enabled) {
        appSettings.isFullScreen = enabled
        if (enabled) {
            mainview.showFullScreen()
        }
        else {
            mainview.showNormal()
        }
    }

    function toggleFullScreenMode() {
        setFullScreenMode(mainview.visibility !== Window.FullScreen)
    }

    function handleUpdateBottomTrack() {
        menuBar.updateBottomTrack()
    }

    function refreshAllGraphicsAfterResume() {
        if (renderer) {
            renderer.update()
            renderer.onCameraMoved()
        }

        if (waterViewFirst) {
            waterViewFirst.update()
        }

        if (waterViewSecond && waterViewSecond.visible) {
            waterViewSecond.update()
        }

        if (syncLoupePlot3D) {
            syncLoupePlot3D.update()
        }

        // if (syncLoupeOverlay && syncLoupeOverlay.visible) {
        //     syncLoupeOverlay.refreshLoupePlot()
        // }

        mainview.update()
    }

    function scheduleResumeRefreshIfNeeded() {
        if (Qt.platform.os !== "android") {
            return
        }

        if (Qt.application.state !== Qt.ApplicationActive) {
            return
        }

        // Defer refresh until window/surface is active again.
        Qt.callLater(refreshAllGraphicsAfterResume)
    }

    function handleAndroidBack() {
        if (Qt.platform.os !== "android") {
            return false
        }

        // Step 1: close modal/popup/menu overlays.
        if (profilePickDialog.visible) {
            profilePickDialog.close()
            return true
        }

        if (profilesDialog.visible) {
            profilesDialog.close()
            return true
        }

        if (showBanner) {
            showBanner = false
            return true
        }

        if (typeof contactDialog !== "undefined" && contactDialog.visible) {
            contactDialog.visible = false
            return true
        }

        if (menuBlock.visible) {
            menuBlock.visible = false
            return true
        }

        if (geoMenuBlock.visible) {
            geoMenuBlock.visible = false
            return true
        }

        if (rulerMenuBlock.visible) {
            rulerMenuBlock.visible = false
            return true
        }

        if (waterViewFirst.closeTransientUi && waterViewFirst.closeTransientUi()) {
            return true
        }

        if (waterViewSecond.visible && waterViewSecond.closeTransientUi && waterViewSecond.closeTransientUi()) {
            return true
        }

        // Step 2: cancel active editing modes.
        if (renderer.geoJsonEnabled) {
            const geo = renderer.geoJsonController
            if (geo && geo.drawing) {
                renderer.geojsonCancelDrawing()
                return true
            }
        }

        if (renderer.rulerDrawing) {
            renderer.rulerCancelDrawing()
            return true
        }

        if (renderer.rulerEnabled || renderer.rulerSelected || renderer.rulerHasGeometry) {
            renderer.clearRuler()
            return true
        }

        // Step 3: close settings panels.
        let settingsClosed = false

        if (waterViewFirst.settingsOpen) {
            waterViewFirst.closeSettings()
            settingsClosed = true
        }

        if (waterViewSecond.visible && waterViewSecond.settingsOpen) {
            waterViewSecond.closeSettings()
            settingsClosed = true
        }

        if (menuBar.hasOpenMenus) {
            menuBar.closeMenus()
            settingsClosed = true
        }

        if (settingsClosed) {
            return true
        }

        // Step 4: root screen -> send app to background.
        core.moveAppToBackground()
        return true
    }

    onVisibilityChanged: function(windowVisibility) {
        if (windowVisibility === Window.FullScreen) {
            scheduleResumeRefreshIfNeeded()
        }
    }

    Connections {
        target: Qt.application

        function onStateChanged() {
            scheduleResumeRefreshIfNeeded()
        }
    }

    Component.onCompleted: {
        theme.updateResCoeff()
        core.progress = fileProgress

        menuBar.languageChanged.connect(handleChildSignal)
        menuBar.syncPlotEnabled.connect(handleSyncPlotEnabled)
        menuBar.menuBarSettingOpened.connect(onMenuBarSettingsOpened)

        waterViewFirst.plotCursorChanged.connect(handlePlotCursorChanged)
        waterViewFirst.updateOtherPlot.connect(handleUpdateOtherPlot)
        waterViewFirst. plotPressed.connect(handlePlotPressed)
        waterViewFirst. plotReleased.connect(handlePlotReleased)
        waterViewFirst.settingsClicked.connect(onPlotSettingsClicked)

        waterViewSecond.plotCursorChanged.connect(handlePlotCursorChanged)
        waterViewSecond.updateOtherPlot.connect(handleUpdateOtherPlot)
        waterViewSecond.plotPressed.connect(handlePlotPressed)
        waterViewSecond.plotReleased.connect(handlePlotReleased)
        waterViewSecond.settingsClicked.connect(onPlotSettingsClicked)

        scene3DToolbar.updateBottomTrack.connect(handleUpdateBottomTrack)
        scene3DToolbar.mosaicLAngleOffsetChanged.connect(handleMosaicLOffsetChanged)
        scene3DToolbar.mosaicRAngleOffsetChanged.connect(handleMosaicROffsetChanged)

        if (appSettings.isFullScreen) {
            mainview.showFullScreen()
        }

        // contacts
        function setupConnections() {
            if (typeof contacts !== "undefined") {
                contactConnections.target = contacts;
            }
            else {
                Qt.callLater(setupConnections);
            }
        }
        Qt.callLater(setupConnections);
    }


    KDialogOK {
        id: dialogOK
    }
    KDialogYesNo {
        id: dialogYesNo
    }
    KDialogCheck {
        id: checkDialog
    }
    KDialogCheck2 {
        id: checkDialog2
    }
    KDialogLoading {
        id: dialogLoading
    }

    Connections {
        target: GetInterface

        function onShowDialogInfo(type, msg) {
            switch (type) {
                case 0: dialogOK.show(msg);      break;
                case 1: dialogYesNo.show(msg);   break;
                case 2: checkDialog.show(msg);   break;
                case 3: checkDialog2.show(msg);  break;
                case 4: dialogLoading.show(msg); break;
            }
        }

        function onFlashDialog(type) {
            switch (type) {
                case 0: dialogOK.flash();     break
                case 1: dialogYesNo.flash();  break
                case 2: checkDialog.flash();  break
                case 3: checkDialog2.flash(); break
            }
        }
    }


    property bool   showBanner: false
    property string selectedLanguageStr: qsTr("Undefined")


    SplitView {
        id: splitLayer
        visible: !showBanner
        Layout.fillHeight: true
        Layout.fillWidth:  true
        anchors.fill:      parent
        orientation:       Qt.Vertical


        //添加键盘快捷键映射
        property var hotkeysMapScan: ({
            "57":  { "functionName":  "toggleFullScreen",  "parameter": undefined },   // F11
            // "41":  { "functionName":  "openFile",          "parameter": undefined },   // O
            "44":  { "functionName":  "closeFile",         "parameter": undefined },   // W
            "33":  { "functionName":  "updateBottomTrack", "parameter": undefined },   // R
            "34":  { "functionName":  "updateMosaic",      "parameter": undefined },   // T
            "35":  { "functionName":  "closeSettings",     "parameter": undefined },   // Esc
            "100": { "functionName":  "horScrollLeft",     "parameter": 5 },           // Left Arrow
            "102": { "functionName":  "horScrollRight",    "parameter": 5 },           // Right Arrow
            "101": { "functionName":  "verScrollUp",       "parameter": 5 },           // Up Arrow
            "103": { "functionName":  "verScrollDown",     "parameter": 5 },           // Down Arrow
        })

        Keys.onReleased: function(event) {
            if (event.key === Qt.Key_Back || event.key === Qt.Key_Escape) {
                if (handleAndroidBack()) {
                    event.accepted = true
                    return
                }
            }

            let sc = event.nativeScanCode.toString()
            let hotkeyData = hotkeysMapScan[sc];
            if (hotkeyData === undefined) {
                return
            }

            let fn = hotkeyData["functionName"];
            let p = hotkeyData["parameter"];

            // high priority
            if (fn === "toggleFullScreen") {
                toggleFullScreenMode()
                return;
            }
            if (fn === "openFileDialog") {
                menuBar.openFileDialog()
                return;
            }
            if (fn === "closeFile") {
                core.closeLogFile()
                return;
            }
            if (fn === "updateBottomTrack") {
                menuBar.updateBottomTrack()
            }
            if (fn === "updateMosaic") {
                scene3DToolbar.updateMosaic()
            }
            if (fn === "closeSettings") {
                waterViewFirst.closeSettings()
                if (waterViewSecond.enabled) {
                    waterViewSecond.closeSettings()
                }
                menuBar.closeMenus()
                splitLayer.focus = true
                return;
            }

            if (mainview.activeFocusItem &&
                (mainview.activeFocusItem instanceof TextEdit || mainview.activeFocusItem instanceof TextField)) {
                return;
            }

            if (fn !== undefined) {
                if (p === undefined) {
                    p = 5
                }

                switch (fn) {
                case "horScrollLeft": {
                    waterViewFirst.horScrollEvent(-p)
                    if (waterViewSecond.enabled) {
                        waterViewSecond.horScrollEvent(-p)
                    }
                    break
                }
                case "horScrollRight": {
                    waterViewFirst.horScrollEvent(p)
                    if (waterViewSecond.enabled) {
                        waterViewSecond.horScrollEvent(p)
                    }
                    break
                }
                case "verScrollUp": {
                    waterViewFirst.verScrollEvent(-p)
                    if (waterViewSecond.enabled) {
                        waterViewSecond.verScrollEvent(-p)
                    }
                    break
                }
                case "verScrollDown": {
                    waterViewFirst.verScrollEvent(p)
                    if (waterViewSecond.enabled) {
                        waterViewSecond.verScrollEvent(p)
                    }
                    break
                }
                case "verZoomOut": {
                    waterViewFirst.verZoomEvent(-p)
                    if (waterViewSecond.enabled) {
                        waterViewSecond.verZoomEvent(-p)
                    }
                    break
                }
                case "verZoomIn": {
                    waterViewFirst.verZoomEvent(p)
                    if (waterViewSecond.enabled) {
                        waterViewSecond.verZoomEvent(p)
                    }
                    break
                }
                case "scene3dZoomIn": {
                    if (menuBar.is3DVisible) {
                        renderer.zoomStepTrigger(1)
                    }
                    break
                }
                case "scene3dZoomOut": {
                    if (menuBar.is3DVisible) {
                        renderer.zoomStepTrigger(-1)
                    }
                    break
                }
                case "mosaicPrevTheme": {
                    if (menuBar.is3DVisible) {
                        scene3DToolbar.mosaicPrevTheme()
                    }
                    break
                }
                case "mosaicNextTheme": {
                    if (menuBar.is3DVisible) {
                        scene3DToolbar.mosaicNextTheme()
                    }
                    break
                }
                case "mosaicLowLevelUp": {
                    if (menuBar.is3DVisible) {
                        scene3DToolbar.mosaicLowLevelUp(p)
                    }
                    break
                }
                case "mosaicLowLevelDown": {
                    if (menuBar.is3DVisible) {
                        scene3DToolbar.mosaicLowLevelDown(p)
                    }
                    break
                }
                case "mosaicHighLevelUp": {
                    if (menuBar.is3DVisible) {
                        scene3DToolbar.mosaicHighLevelUp(p)
                    }
                    break
                }
                case "mosaicHighLevelDown": {
                    if (menuBar.is3DVisible) {
                        scene3DToolbar.mosaicHighLevelDown(p)
                    }
                    break
                }
                case "surfacePrevTheme": {
                    if (menuBar.is3DVisible) {
                        scene3DToolbar.surfacePrevTheme()
                    }
                    break
                }
                case "surfaceNextTheme": {
                    if (menuBar.is3DVisible) {
                        scene3DToolbar.surfaceNextTheme()
                    }
                    break
                }
                case "surfaceStepDown": {
                    if (menuBar.is3DVisible) {
                        scene3DToolbar.surfaceStepDown(p)
                    }
                    break
                }
                case "surfaceStepUp": {
                    if (menuBar.is3DVisible) {
                        scene3DToolbar.surfaceStepUp(p)
                    }
                    break
                }
                case "toggleBottomTrack3D": {
                    if (menuBar.is3DVisible) {
                        scene3DToolbar.toggleBottomTrack()
                    }
                    break
                }
                case "toggleIsobaths3D": {
                    if (menuBar.is3DVisible) {
                        scene3DToolbar.toggleIsobaths()
                    }
                    break
                }
                case "toggleMosaic3D": {
                    if (menuBar.is3DVisible) {
                        scene3DToolbar.toggleMosaic()
                    }
                    break
                }
                case "cameraShiftXMinus3D": {
                    if (menuBar.is3DVisible) {
                        renderer.panStepTrigger(-1, 0)
                    }
                    break
                }
                case "cameraShiftXPlus3D": {
                    if (menuBar.is3DVisible) {
                        renderer.panStepTrigger(1, 0)
                    }
                    break
                }
                case "cameraShiftYMinus3D": {
                    if (menuBar.is3DVisible) {
                        renderer.panStepTrigger(0, -1)
                    }
                    break
                }
                case "cameraShiftYPlus3D": {
                    if (menuBar.is3DVisible) {
                        renderer.panStepTrigger(0, 1)
                    }
                    break
                }
                case "resetCameraTop3D": {
                    if (menuBar.is3DVisible) {
                        renderer.resetCameraAngleTrigger()
                    }
                    break
                }
                case "cameraShiftZMinus3D": {
                    if (menuBar.is3DVisible) {
                        renderer.zStepTrigger(-1)
                    }
                    break
                }
                case "cameraShiftZPlus3D": {
                    if (menuBar.is3DVisible) {
                        renderer.zStepTrigger(1)
                    }
                    break
                }
                case "resetDepthZoom3D": {
                    if (menuBar.is3DVisible) {
                        Scene3dToolBarController.onCancelZoomButtonClicked()
                    }
                    break
                }
                case "increaseLowLevel": {
                    let newLow = Math.min(120, waterViewFirst.getLowEchogramLevel() + p)
                    let newHigh = waterViewFirst.getHighEchogramLevel()
                    if (newLow > newHigh) newHigh = newLow
                    waterViewFirst.plotEchogramSetLevels(newLow, newHigh)
                    waterViewFirst.setLevels(newLow, newHigh)
                    if (waterViewSecond.enabled) {
                        let newSLow = Math.min(120, waterViewSecond.getLowEchogramLevel() + p)
                        let newSHigh = waterViewSecond.getHighEchogramLevel()
                        if (newSLow > newSHigh) newSHigh = newSLow
                        waterViewSecond.plotEchogramSetLevels(newSLow, newSHigh)
                        waterViewSecond.setLevels(newSLow, newSHigh)
                    }
                    break
                }
                case "decreaseLowLevel": {
                    let newLow = Math.max(0, waterViewFirst.getLowEchogramLevel() - p)
                    let newHigh = waterViewFirst.getHighEchogramLevel()
                    waterViewFirst.plotEchogramSetLevels(newLow, newHigh)
                    waterViewFirst.setLevels(newLow, newHigh)
                    if (waterViewSecond.enabled) {
                        let newSLow = Math.max(0, waterViewSecond.getLowEchogramLevel() - p)
                        let newSHigh = waterViewSecond.getHighEchogramLevel()
                        waterViewSecond.plotEchogramSetLevels(newSLow, newSHigh)
                        waterViewSecond.setLevels(newSLow, newSHigh)
                    }
                    break
                }
                case "increaseHighLevel": {
                    let newHigh = Math.min(120, waterViewFirst.getHighEchogramLevel() + p)
                    let newLow = waterViewFirst.getLowEchogramLevel()
                    waterViewFirst.plotEchogramSetLevels(newLow, newHigh)
                    waterViewFirst.setLevels(newLow, newHigh)
                    if (waterViewSecond.enabled) {
                        let newSHigh = Math.min(120, waterViewSecond.getHighEchogramLevel() + p)
                        let newSLow = waterViewSecond.getLowEchogramLevel()
                        waterViewSecond.plotEchogramSetLevels(newSLow, newSHigh)
                        waterViewSecond.setLevels(newSLow, newSHigh)
                    }
                    break
                }
                case "decreaseHighLevel": {
                    let newHigh = Math.max(0, waterViewFirst.getHighEchogramLevel() - p)
                    let newLow = waterViewFirst.getLowEchogramLevel()
                    if (newHigh < newLow) newLow = newHigh
                    waterViewFirst.plotEchogramSetLevels(newLow, newHigh)
                    waterViewFirst.setLevels(newLow, newHigh)
                    if (waterViewSecond.enabled) {
                        let newSHigh = Math.max(0, waterViewSecond.getHighEchogramLevel() - p)
                        let newSLow = waterViewSecond.getLowEchogramLevel()
                        if (newSHigh < newSLow) newSLow = newSHigh
                        waterViewSecond.plotEchogramSetLevels(newSLow, newSHigh)
                        waterViewSecond.setLevels(newSLow, newSHigh)
                    }
                    break
                }
                case "prevTheme": {
                    let themeId = waterViewFirst.getThemeId()
                    if (themeId > 0) waterViewFirst.plotEchogramTheme(themeId - 1)
                    if (waterViewSecond.enabled) {
                        let themeSId = waterViewSecond.getThemeId()
                        if (themeSId > 0) waterViewSecond.plotEchogramTheme(themeSId - 1)
                    }
                    break
                }
                case "nextTheme": {
                    let themeId = waterViewFirst.getThemeId()
                    if (themeId < 9) waterViewFirst.plotEchogramTheme(themeId + 1)
                    if (waterViewSecond.enabled) {
                        let themeSId = waterViewSecond.getThemeId()
                        if (themeSId < 9) waterViewSecond.plotEchogramTheme(themeSId + 1)
                    }
                    break
                }
                case "toggleEchogramType": {
                    waterViewFirst.toggleEchogramType()
                    if (waterViewSecond.enabled) {
                        waterViewSecond.toggleEchogramType()
                    }
                    break
                }
                case "clickConnections": {
                    menuBar.clickConnections()
                    break
                }
                case "clickSettings": {
                    menuBar.clickSettings()
                    break
                }
                case "click3D": {
                    menuBar.click3D()
                    break
                }
                case "click2D": {
                    menuBar.click2D()
                    break
                }
                default: {
                    break
                }
                }
            }
        }

        // handle: Rectangle {
        //     implicitHeight: theme.controlHeight/2
        //     color:          SplitHandle.pressed ? "#A0A0A0" : "#707070"

        //     Rectangle {
        //         width:  parent.width
        //         height: 1
        //         color:  "#A0A0A0"
        //     }

        //     Rectangle {
        //         y:      parent.height
        //         width:  parent.width
        //         height: 1
        //         color:  "#A0A0A0"
        //     }
        // }

        Item {
            id:  visualisationLayout
            SplitView.fillHeight: true
            SplitView.fillWidth:  true
            Layout.fillHeight: true
            Layout.fillWidth:  true

            readonly property bool landscapeMode: mainview.width > mainview.height
            readonly property int  rows:    landscapeMode ? 1 : 2
            readonly property int  columns: landscapeMode ? 2 : 1

            property int  lastKeyPressed: Qt.Key_unknown
            property real splitRatio: 0.5

            property int splitMode: 0  // 0: 正常分窗,  1: 声呐全屏(地图小窗),  2: 地图全屏(声呐小窗)
            readonly property real edgeThreshold: 0.1  // 边缘阈值（10%）
            readonly property int  cornerWindowWidth:  screenSize * 0.3
            readonly property int  cornerWindowHeight: screenSize * 0.2

            readonly property real primaryLength: landscapeMode ? width : height
            readonly property real splitLength: Math.max(0, primaryLength)
            readonly property real handlePaneLength: Math.round(splitLength * splitRatio)

            function clampSplitRatio(ratio) {
                if (!isFinite(ratio)) {
                    return 0.5
                }
                return Math.max(0.0, Math.min(1.0, ratio))
            }

            Behavior on splitRatio {
                NumberAnimation {
                    duration: 120
                    easing.type: Easing.OutCubic
                }
            }

            Keys.onPressed: function(event) {
                visualisationLayout.lastKeyPressed = event.key;
            }

            Keys.onReleased: {
                visualisationLayout.lastKeyPressed = Qt.Key_unknown;
            }

            GraphicsScene3dView {
                id:      renderer
                visible: (menuBar !== null) ? menuBar.is3DVisible : false
                objectName: "GraphicsScene3dView"
                x: visualisationLayout.splitMode === 1 ? 10 : 0
                y: visualisationLayout.splitMode === 1 ?
                    (visualisationLayout.height - visualisationLayout.cornerWindowHeight - 10) : 0
                z: visualisationLayout.splitMode === 1 ? 10 : 1
                width: {
                   if (visualisationLayout.splitMode === 1) {
                       // 声呐全屏模式：地图小窗固定宽度
                       return visualisationLayout.cornerWindowWidth
                   }
                   // 正常分窗或地图全屏模式
                    return visualisationLayout.landscapeMode ? visualisationLayout.handlePaneLength : visualisationLayout.width
                }
                height: {
                   if (visualisationLayout.splitMode === 1) {
                       // 声呐全屏模式：地图小窗固定高度
                       return visualisationLayout.cornerWindowHeight
                   }
                   // 正常分窗或地图全屏模式
                   return visualisationLayout.landscapeMode ? visualisationLayout.height : visualisationLayout.handlePaneLength
                }

                focus: true

                ScreetRect { }

                MapLevelChoose { }

                PolygonTool {
                    x: toolBarXR.iconSize
                    visible: toolBarXR.polygonMode
                }

                property bool longPressTriggered: false
                property int  currentZoom: -1
                property bool syncLoupeUiAllowed: (menuBar !== null) ? (menuBar.is3DVisible && !menuBar.is2DVisible) : false

                function resetScenePointerState() {
                    mousearea3D.startMousePos       = Qt.point(-1, -1)
                    mousearea3D.wasMoved            = false
                    mousearea3D.vertexMode          = false
                    mousearea3D.lastMouseKeyPressed = Qt.NoButton
                    longPressTimer.stop()
                    renderer.longPressTriggered     = false
                    renderer.cancelPointerInteraction()
                }

                // onSendDataZoom: function(zoom) {
                //     currentZoom = zoom;
                // }

                PinchArea {
                    id:    pinch3D
                    anchors.fill: parent
                    enabled: !extraInfoPanel.touchInteractionActive

                    onPinchStarted: {
                        menuBlock.visible = false
                        mousearea3D.enabled = false
                    }

                    onPinchUpdated: function(pinch) {
                        var shiftScale = pinch.scale - pinch.previousScale;
                        var shiftAngle = pinch.angle - pinch.previousAngle;
                        renderer.pinchTrigger(pinch.previousCenter, pinch.center, shiftScale, shiftAngle)
                    }

                    onPinchFinished: {
                        mousearea3D.enabled = true
                    }

                    MouseArea {
                        id: mousearea3D
                        enabled:              true
                        anchors.fill:         parent
                        acceptedButtons:      Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
                        focus:                true
                        hoverEnabled:         true
                        Keys.enabled:         true
                        Keys.onDeletePressed: function(event) { renderer.keyPressTrigger(event.key) }
                        Keys.onReturnPressed: function(event) { renderer.keyPressTrigger(event.key) }
                        Keys.onEnterPressed:  function(event) { renderer.keyPressTrigger(event.key) }
                        Keys.onEscapePressed: function(event) {
                            if (Qt.platform.os === "android") {
                                if (mainview.handleAndroidBack()) {
                                    event.accepted = true
                                    return
                                }
                            }
                            if (renderer.geoJsonEnabled) {
                                renderer.geojsonCancelDrawing()
                            }
                            else {
                                renderer.clearRuler()
                            }
                        }

                        property int   lastMouseKeyPressed: Qt.NoButton // TODO: maybe this mouseArea should be outside pinchArea
                        property point startMousePos:       Qt.point(-1, -1)
                        property bool  wasMoved:            false
                        property real  mouseThreshold:      15
                        property bool  vertexMode:          false

                        onEntered: {
                            mousearea3D.forceActiveFocus();
                        }

                        onWheel: function(wheel) {
                            renderer.mouseWheelTrigger(wheel.buttons, wheel.x, wheel.y, wheel.angleDelta, visualisationLayout.lastKeyPressed)
                        }

                        onPositionChanged: function(mouse) {
                            if (Qt.platform.os === "android") {
                                if (!wasMoved) {
                                    var delta = Math.sqrt(Math.pow((mouse.x - startMousePos.x), 2) + Math.pow((mouse.y - startMousePos.y), 2));
                                    if (delta > mouseThreshold) {
                                        wasMoved = true;
                                    }
                                }
                                if (renderer.longPressTriggered && !wasMoved) {
                                    if (renderer.geoJsonEnabled || renderer.rulerEnabled || renderer.rulerHasGeometry) {
                                        vertexMode = true
                                    } else {
                                        if (!vertexMode) {
                                            renderer.switchToBottomTrackVertexComboSelectionMode(mouse.x, mouse.y)
                                        }
                                        vertexMode = true
                                    }
                                }
                            }

                            const activeButtons = (Qt.platform.os === "android" && lastMouseKeyPressed !== Qt.NoButton)
                                    ? lastMouseKeyPressed : mouse.buttons
                            renderer.mouseMoveTrigger(activeButtons, mouse.x, mouse.y, visualisationLayout.lastKeyPressed)
                        }

                        onPressed: function(mouse) {
                            menuBlock.visible      = false
                            geoMenuBlock.visible   = false
                            rulerMenuBlock.visible = false
                            startMousePos          = Qt.point(mouse.x, mouse.y)
                            wasMoved               = false
                            vertexMode             = false
                            longPressTimer.start()
                            renderer.longPressTriggered = false

                            lastMouseKeyPressed    = mouse.buttons
                            renderer.mousePressTrigger(mouse.buttons, mouse.x, mouse.y, visualisationLayout.lastKeyPressed)
                        }

                        onDoubleClicked: function(mouse) {
                            if(mouse.button === Qt.LeftButton) {
                                renderer.mouseDoubleClickTrigger(mouse.buttons, mouse.x, mouse.y, visualisationLayout.lastKeyPressed)
                            }
                        }

                        onReleased: function(mouse) {
                            startMousePos = Qt.point(-1, -1)
                            wasMoved = false
                            longPressTimer.stop()

                            renderer.mouseReleaseTrigger(lastMouseKeyPressed, mouse.x, mouse.y, visualisationLayout.lastKeyPressed)

                            if (mouse.button === Qt.RightButton || (Qt.platform.os === "android" && vertexMode)) {
                                if (renderer.geoJsonEnabled) {
                                    geoMenuBlock.position(mouse.x, mouse.y)
                                }
                                else if (renderer.rulerEnabled || renderer.rulerSelected) {
                                    rulerMenuBlock.position(mouse.x, mouse.y)
                                }
                                else {
                                    menuBlock.position(mouse.x, mouse.y)
                                }
                            }

                            vertexMode = false
                            lastMouseKeyPressed = Qt.NoButton
                        }

                        onCanceled: {
                            renderer.resetScenePointerState()
                        }
                    }
                }

                Timer {
                    id: longPressTimer
                    interval: 500 // ms
                    repeat: false

                    onTriggered: {
                        renderer.longPressTriggered = true
                    }
                }

                Scene3DToolbar{
                    id: scene3DToolbar
                    x: renderer.width * 0.5
                    y: renderer.height - scene3DToolbar.height * 1.5
                    Keys.forwardTo: [mousearea3D]
                    visible: visualisationLayout.splitMode !== 1
                }


                Connections {
                    id: contactConnections
                    target: null
                    function onContactChanged() {
                        contactDialog.visible = contacts.contactVisible
                        if (contacts.contactVisible) {
                            contactDialog.info           = contacts.contactInfo
                            contactDialog.inputFieldText = contacts.contactInfo
                            contactDialog.x              = contacts.contactPositionX
                            contactDialog.y              = contacts.contactPositionY
                            contactDialog.indx           = contacts.contactIndx
                            contactDialog.lat            = contacts.contactLat
                            contactDialog.lon            = contacts.contactLon
                            contactDialog.depth          = contacts.contactDepth
                        }
                    }
                }

                RowLayout {
                    id: menuBlock
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 1
                    visible: false
                    Layout.margins: 0

                    function position(mx, my) {
                        var oy = renderer.height - (my + implicitHeight)
                        if (oy < 0) {
                            my = my + oy
                        }
                        if (my < 0) {
                            my = 0
                        }
                        var ox = renderer.width - (mx - implicitWidth)
                        if (ox < 0) {
                            mx = mx + ox
                        }
                        x = mx
                        y = my
                        visible = true
                    }

                    ButtonGroup { id: pencilbuttonGroup }

                    CheckButton {
                        icon.source: "qrc:/icons/ui/arrow_bar_to_down.svg"
                        backColor: theme.controlBackColor
                        checkable: false
                        // implicitWidth: theme.controlHeight

                        onClicked: {
                            renderer.bottomTrackActionEvent(BottomTrack.MinDistProc)
                            menuBlock.visible = false
                        }

                        ButtonGroup.group: pencilbuttonGroup
                    }

                    CheckButton {
                        icon.source: "qrc:/icons/ui/arrow_bar_to_up.svg"
                        backColor: theme.controlBackColor
                        checkable: false
                        // implicitWidth: theme.controlHeight

                        onClicked: {
                            renderer.bottomTrackActionEvent(BottomTrack.MaxDistProc)
                            menuBlock.visible = false
                        }

                        ButtonGroup.group: pencilbuttonGroup
                    }

                    CheckButton {
                        icon.source: "qrc:/icons/ui/eraser.svg"
                        backColor: theme.controlBackColor
                        checkable: false
                        // implicitWidth: theme.controlHeight

                        onClicked: {
                            renderer.bottomTrackActionEvent(BottomTrack.ClearDistProc)
                            menuBlock.visible = false
                        }

                        ButtonGroup.group: pencilbuttonGroup
                    }

                    CheckButton {
                        icon.source: "qrc:/icons/ui/x.svg"
                        backColor: theme.controlBackColor
                        checkable: false
                        // implicitWidth: theme.controlHeight

                        onClicked: {
                            renderer.bottomTrackActionEvent(BottomTrack.Undefined)

                            menuBlock.visible = false
                        }

                        ButtonGroup.group: pencilbuttonGroup
                    }
                }

                RowLayout {
                    id: geoMenuBlock
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 1
                    visible: false
                    Layout.margins: 0

                    property var geo: renderer.geoJsonController

                    onGeoChanged: {
                        //console.log("GeoJson menu updated, drawing: " + geo.drawing + ", selectedFeatureId: " + geo.selectedFeatureId)
                    }

                    function position(mx, my) {
                        var oy = renderer.height - (my + implicitHeight)
                        if (oy < 0) {
                            my = my + oy
                        }
                        if (my < 0) {
                            my = 0
                        }
                        var ox = renderer.width - (mx - implicitWidth)
                        if (ox < 0) {
                            mx = mx + ox
                        }
                        x = mx
                        y = my
                        visible = true
                    }

                    CheckButton {
                        icon.source: "qrc:/icons/ui/plus.svg"
                        backColor: theme.controlBackColor
                        checkable: false
                        // implicitWidth: theme.controlHeight
                        visible: geoMenuBlock.geo && geoMenuBlock.geo.drawing

                        onClicked: {
                            renderer.geojsonFinishDrawing()
                            geoMenuBlock.visible = false
                        }
                    }

                    CheckButton {
                        icon.source: "qrc:/icons/ui/stack_backward.svg"
                        backColor: theme.controlBackColor
                        checkable: false
                        // implicitWidth: theme.controlHeight
                        visible: geoMenuBlock.geo && geoMenuBlock.geo.drawing

                        onClicked: {
                            renderer.geojsonUndoLastVertex()
                            geoMenuBlock.visible = false
                        }
                    }

                    CheckButton {
                        icon.source: "qrc:/icons/ui/x.svg"
                        backColor: theme.controlBackColor
                        checkable: false
                        // implicitWidth: theme.controlHeight
                        visible: geoMenuBlock.geo && geoMenuBlock.geo.drawing

                        onClicked: {
                            renderer.geojsonCancelDrawing()
                            geoMenuBlock.visible = false
                        }
                    }

                    CheckButton {
                        icon.source: "qrc:/icons/ui/timeline_event_x.svg"
                        backColor: theme.controlBackColor
                        checkable: false
                        // implicitWidth: theme.controlHeight
                        visible: geoMenuBlock.geo && !geoMenuBlock.geo.drawing && geoMenuBlock.geo.selectedFeatureId !== ""

                        onClicked: {
                            renderer.geojsonDeleteSelectedFeature()
                            geoMenuBlock.visible = false
                        }
                    }
                }


                Rectangle {
                    anchors.left:   parent.left
                    anchors.bottom: parent.bottom
                    height: visualisationLayout.splitMode === 1 ?
                            visualisationLayout.cornerWindowHeight * 0.02 : footHeight
                    color: "#363636"
                    opacity: 0.9

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 1

                        Label {
                            text: qsTr(" Longitude:%1°   Latitude:%2°").arg(renderer.currLon.toFixed(6)).arg(renderer.currLat.toFixed(6))
                            color: "white"
                            font.bold: true
                            font.pixelSize: parent.height * 0.7
                        }

                        Rectangle {
                            width: 1
                            height: parent.height * 0.8
                            color: "#AAAAAA"
                            Layout.alignment: Qt.AlignVCenter
                        }

                        Label {
                            text: qsTr(" Zoom: %1").arg(core.currMapLevel)
                            color: "white"
                            font.bold: true
                            font.pixelSize: parent.height * 0.7
                        }
                    }
                }

                Rectangle {
                    x: visualisationLayout.cornerWindowWidth - width * 1.1
                    y: 1
                    width: visualisationLayout.cornerWindowHeight * 0.12
                    height: width
                    color: "#fffafa"
                    radius: width * 0.15
                    visible: visualisationLayout.splitMode === 1

                    Image {
                        anchors.fill: parent
                        source: "qrc:/XR/expand_map.svg"
                        fillMode: Image.PreserveAspectFit
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor

                        onClicked: {
                            visualisationLayout.splitMode  = 0
                            visualisationLayout.splitRatio = 0.5
                        }
                    }
                }


                Rectangle {
                    x: visualisationLayout.cornerWindowWidth - width * 1.1
                    y: visualisationLayout.cornerWindowHeight - height - 1
                    width: visualisationLayout.cornerWindowHeight * 0.12
                    height: width
                    color: "#fffafa"
                    radius: width * 0.15
                    visible: visualisationLayout.splitMode === 1

                    Image {
                        anchors.fill: parent
                        source: "qrc:/XR/expandWholeWindow.svg"
                        fillMode: Image.PreserveAspectFit
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor

                        onClicked: {
                            visualisationLayout.splitMode  = 2
                            visualisationLayout.splitRatio = 1
                        }
                    }
                }

            }


            Item {
                id: sceneSplitHandle
                visible: visualisationLayout.splitMode === 0
                x: visualisationLayout.landscapeMode ? Math.round(visualisationLayout.handlePaneLength - width * 0.5)
                   : Math.round((visualisationLayout.width - width) * 0.5)
                y: visualisationLayout.landscapeMode ? Math.round((visualisationLayout.height - height) * 0.5)
                   : Math.round(visualisationLayout.handlePaneLength - height * 0.5)
                width:  iconSize * 2
                height: screenSize * 0.2
                z: 10000

                Rectangle {
                    anchors.fill: parent
                    radius: width * 0.5
                    color: "#8D8D8D"
                    border.color: "#D0D0D0"
                    border.width: 1
                    opacity:  (sceneSplitHandleMouse.containsMouse || sceneSplitHandleMouse.pressed) ? 1.0 : 0.5
                }

                MouseArea {
                    id: sceneSplitHandleMouse
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton
                    hoverEnabled: true
                    preventStealing: true
                    cursorShape: visualisationLayout.landscapeMode ? Qt.SplitHCursor : Qt.SplitVCursor
                    property real dragStartGlobalPos: 0
                    property real dragStartRatio: visualisationLayout.splitRatio

                    onPressed: function(mouse) {
                        dragStartRatio = visualisationLayout.splitRatio
                        const mappedPos = sceneSplitHandleMouse.mapToItem(visualisationLayout, mouse.x, mouse.y)
                        dragStartGlobalPos = visualisationLayout.landscapeMode ? mappedPos.x : mappedPos.y
                    }

                    onPositionChanged: function(mouse) {
                        if (!pressed || visualisationLayout.splitLength <= 0) {
                            return
                        }

                        const mappedPos = sceneSplitHandleMouse.mapToItem(visualisationLayout, mouse.x, mouse.y)
                        const currentGlobalPos = visualisationLayout.landscapeMode ? mappedPos.x : mappedPos.y
                        const delta = currentGlobalPos - dragStartGlobalPos
                        const startLength = dragStartRatio * visualisationLayout.splitLength
                        const newRatio = (startLength + delta) / visualisationLayout.splitLength

                        visualisationLayout.splitRatio = visualisationLayout.clampSplitRatio(newRatio)
                        if (visualisationLayout.splitRatio < visualisationLayout.edgeThreshold) {
                            // 滑块靠近左边缘，触发声呐全屏模式
                            visualisationLayout.splitMode  = 1
                            visualisationLayout.splitRatio = 0
                        }
                        else if (visualisationLayout.splitRatio > (1 - visualisationLayout.edgeThreshold)) {
                            // 滑块靠近右边缘，触发地图全屏模式
                            visualisationLayout.splitMode  = 2
                            visualisationLayout.splitRatio = 1
                        }
                        else {
                            // 正常分窗模式
                            visualisationLayout.splitMode = 0
                        }
                    }

                }
            }


            Item {
                id: plotsContainer
                visible: menuBar.is2DVisible

                // 根据分割模式计算位置和尺寸
                x: {
                    if (visualisationLayout.splitMode === 2) {
                         // 地图全屏模式：声呐小窗在右下角
                         return visualisationLayout.width - visualisationLayout.cornerWindowWidth - 10
                     }
                     // 正常分窗或声呐全屏模式
                     return visualisationLayout.landscapeMode ?
                            (visualisationLayout.splitMode === 0 ? visualisationLayout.handlePaneLength : 0) : 0
                }
                y: {
                    if (visualisationLayout.splitMode === 2) {
                        // 地图全屏模式：声呐小窗在右下角
                        return visualisationLayout.height - visualisationLayout.cornerWindowHeight - 10
                    }
                    return visualisationLayout.landscapeMode ?
                            0 : (visualisationLayout.splitMode === 0 ? visualisationLayout.handlePaneLength : 0)
                }
                z: visualisationLayout.splitMode === 2 ? 10 : 1
                width: {
                    if (visualisationLayout.splitMode === 2) {
                        // 地图全屏模式：声呐小窗固定宽度
                        return visualisationLayout.cornerWindowWidth
                    }
                    // 正常分窗或声呐全屏模式
                    return visualisationLayout.landscapeMode ? (visualisationLayout.splitMode === 0
                               ? Math.max(0, visualisationLayout.width - visualisationLayout.handlePaneLength)
                               : visualisationLayout.width) : visualisationLayout.width
                }
                height: {
                    if (visualisationLayout.splitMode === 2) {
                        // 地图全屏模式：声呐小窗固定高度
                        return visualisationLayout.cornerWindowHeight
                    }
                    // 正常分窗或声呐全屏模式
                    return visualisationLayout.landscapeMode ? visualisationLayout.height : (visualisationLayout.splitMode === 0
                        ? Math.max(0, visualisationLayout.height - visualisationLayout.handlePaneLength) : visualisationLayout.height)
                }

                GridLayout {
                    anchors.fill: parent
                    rows    : 2
                    columns : 1
                    columnSpacing: 0
                    rowSpacing: 0

                    Plot2D {
                        id: waterViewFirst
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.rowSpan   : 1
                        Layout.columnSpan: 1
                        focus: true
                        instruments: menuBar.instruments
                        indx: 1
                        is3dVisible: menuBar.is3DVisible
                        onTimelinePositionChanged: historyScroll.value = waterViewFirst.timelinePosition
                        Component.onCompleted: waterViewFirst.setIndx(waterViewFirst.indx);

                        Rectangle {
                            x: 1
                            y: 1
                            width: visualisationLayout.cornerWindowHeight * 0.12
                            height: width
                            color: "#fffafa"
                            radius: width * 0.15
                            visible: visualisationLayout.splitMode === 2

                            Image {
                                anchors.fill: parent
                                source: "qrc:/XR/expand_sonar.svg"
                                fillMode: Image.PreserveAspectFit
                            }

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor

                                onClicked: {
                                    visualisationLayout.splitMode  = 0
                                    visualisationLayout.splitRatio = 0.5
                                }
                            }
                        }

                        Rectangle {
                            x: 1
                            y: visualisationLayout.cornerWindowHeight - height - 1
                            width: visualisationLayout.cornerWindowHeight * 0.12
                            height: width
                            color: "#fffafa"
                            radius: width * 0.15
                            visible: visualisationLayout.splitMode === 2

                            Image {
                                anchors.fill: parent
                                source: "qrc:/XR/expandWholeWindow.svg"
                                fillMode: Image.PreserveAspectFit
                            }

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor

                                onClicked: {
                                    visualisationLayout.splitMode  = 1
                                    visualisationLayout.splitRatio = 1
                                }
                            }
                        }
                    }

                    Plot2D {
                        id: waterViewSecond
                        enabled: menuBar.numPlots === 2
                        visible: menuBar.numPlots === 2
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.rowSpan   : 1
                        Layout.columnSpan: 1
                        focus: true
                        instruments: menuBar.instruments
                        indx: 2

                        onEnabledChanged: {
                            // waterViewSecond.setPlotEnabled = enabled
                        }

                        onVisibleChanged: {
                            if (visible && menuBar.syncPlots) {
                                setCursorFromTo(waterViewFirst.cursorFrom(), waterViewFirst.cursorTo())
                                update()
                            }
                        }

                        onTimelinePositionChanged: {
                            historyScroll.value = timelinePosition
                        }

                        Component.onCompleted: {
                            setIndx(waterViewSecond.indx);
                        }
                    }

                    CSlider {
                        id: historyScroll
                        Layout.margins: 0
                        Layout.fillWidth: true
                        Layout.columnSpan: parent.columns
                        Layout.preferredHeight: theme.iconSize
                        value: waterViewFirst.timelinePosition
                        stepSize: 0.0001
                        from: 0
                        to: 1
                        barWidth: 50 * theme.resCoeff
                        onValueChanged: core.setTimelinePosition(value);
                        onMoved: core.resetAim();
                    }
                }
            }
        }

    }


    MainMenuBar {
        id: menuBar
        objectName: "menuBar"
        Layout.fillHeight: true
        Keys.forwardTo: [splitLayer, mousearea3D]
        height: visualisationLayout.height
        Component.onCompleted: {
            menuBar.targetPlot = waterViewFirst
        }
        visible: !showBanner
    }

    function handleChildSignal(langStr) {
        mainview.showBanner = true
        selectedLanguageStr = langStr
    }

    function handleSyncPlotEnabled() {
        waterViewSecond.setCursorFromTo(waterViewFirst.cursorFrom(), waterViewFirst.cursorTo())
        waterViewSecond.update()
    }

    function handlePlotCursorChanged(indx, from, to) {
        // if (!menuBar.syncPlots) {
        //     if (syncLoupeOverlay && syncLoupeOverlay.visible) {
        //         syncLoupeOverlay.refreshLoupePlot()
        //     }
        //     return;
        // }

        if (indx === 1 && waterViewSecond.enabled) {
            waterViewSecond.setCursorFromTo(from, to)
            waterViewSecond.update()
        }
        if (indx === 2) {
            waterViewFirst.setCursorFromTo(from, to)
            waterViewFirst.update()
        }

        // if (syncLoupeOverlay.visible) {
        //     syncLoupeOverlay.refreshLoupePlot()
        // }
    }

    function handleUpdateOtherPlot(indx) {
        if (indx === 1 && waterViewSecond.enabled) {
            waterViewSecond.update()
        }
        if (indx === 2) {
            waterViewFirst.update()
        }
    }
    function handlePlotPressed(indx, mouseX, mouseY) {
        let r = core.getConvertedMousePos(indx, mouseX, mouseY)

        if (indx === 1 && waterViewSecond.enabled) {
            waterViewSecond.setAim(r.x, r.y)
        }
        if (indx === 2) {
            waterViewFirst.setAim(r.x, r.y)
        }
    }
    function handlePlotReleased(indx) {
        if (indx === 1 && waterViewSecond.enabled) {
            waterViewSecond.resetAim()
        }
        if (indx === 2) {
            waterViewFirst.resetAim()
        }
    }
    function onPlotSettingsClicked() {
        menuBar.closeMenus()
    }
    function onMenuBarSettingsOpened() {
        waterViewFirst.closeSettings()
        waterViewSecond.closeSettings()
    }
    function handleMosaicLOffsetChanged(val) {
        waterViewFirst.mosaicLOffsetChanged(val)
        waterViewSecond.mosaicLOffsetChanged(val)
    }
    function handleMosaicROffsetChanged(val) {
        waterViewFirst.mosaicROffsetChanged(val)
        waterViewSecond.mosaicROffsetChanged(val)
    }


}
