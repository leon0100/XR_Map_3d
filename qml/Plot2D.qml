import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import Qt.labs.settings 1.1

import WaterFall 1.0
import AppXr 1.0
import QmlCommon 1.0
import Communication 1.0

WaterFall {
    id: plot

    property bool is3dVisible: false
    property int  indx: 0
    property int  instruments: 0
    property int  plotSize: theme.screenSize * 0.35
    property int  plotIconSize: theme.iconSize * 1.5

    // horizontal: horisontalVertical.checked
    horizontal: menuToolBar.layoutHorizontal
    property bool currentFrameChecked: plotCheckRec.currentFrameChecked
    // property bool bottomLineChecked:   bottomLine.checked


    function closePlotCheckOutside(globalX, globalY) {
        if (!plotCheckBtn.settingVisible) {
            return
        }
        var pos = plotCheckRec.mapFromItem(null, globalX, globalY)
        if (pos.x < 0 || pos.y < 0 || pos.x > plotCheckRec.width ||
                pos.y > plotCheckRec.height) {
            var btnPos = plotCheckBtn.mapFromItem(null, globalX, globalY)
            if (btnPos.x < 0 || btnPos.y < 0 || btnPos.x > plotCheckBtn.width ||
                    btnPos.y > plotCheckBtn.height) {
                plotCheckBtn.settingVisible = false
                plotCheckBtn.color = "#879fc6"
                plotCheckBtn.border.color = "#879fdd"
            }
        }

    }

    // function setLevels(low, high) {
    //     echogramLevelsSlider.startValue   = low
    //     echogramLevelsSlider.stopValue    = high
    //     echogramLevelsSlider.startPointY  = echogramLevelsSlider.valueToPosition(low);
    //     echogramLevelsSlider.stopPointY   = echogramLevelsSlider.valueToPosition(high);
    //     echogramLevelsSlider.update()
    // }

    function setAim(mouseX, mouseY) {
        plot.plotMousePosition(mouseX, mouseY, true)
    }
    function resetAim() {
        plot.plotMousePosition(-1, -1)
    }
    // function doVerZoomEvent(paramX) {
    //     verZoomEvent(paramX)
    // }
    function doVerScrollEvent(paramX) {
        verScrollEvent(paramX)
    }

    onEnabledChanged: {
        if (enabled) {
            update();
        }
    }

    signal plotCursorChanged(int indx, real from, real to)
    signal updateOtherPlot(int indx)
    signal plotPressed(int indx, int mousex, int mousey)
    signal plotReleased(int indx)
    signal settingsClicked()

    PinchArea {
        id: pinch2D
        anchors.fill: parent
        enabled: true

        property int thresholdXAxis: 15
        property int thresholdYAxis: 15
        property double zoomThreshold: 0.1

        property bool movementX: false
        property bool movementY: false
        property bool zoomY: false
        property point pinchStartPos: Qt.point(-1, -1)

        function clearPinchMovementState() {
            movementX = false
            movementY = false
            zoomY = false
        }

        onPinchStarted: {
            menuBlock.visible = false

            mousearea.enabled = false
            plot.plotMousePosition(-1, -1)

            clearPinchMovementState()
            pinchStartPos = Qt.point(pinch.center.x, pinch.center.y)
        }

        onPinchUpdated: {
            if (movementX) {
                let val = -(pinch.previousCenter.x - pinch.center.x)
                plot.horScrollEvent(val)
                updateOtherPlot(indx)
            }
            else if (movementY) {
                let val = pinch.previousCenter.y - pinch.center.y
                plot.verScrollEvent(val)
                plotCursorChanged(indx, cursorFrom(), cursorTo())
            }
            else if (zoomY) {
                let val = (pinch.previousScale - pinch.scale) * 500.0
                // plot.verZoomEvent(val)
                plotCursorChanged(indx, cursorFrom(), cursorTo())
            }
            else {
                if (Math.abs(pinchStartPos.x - pinch.center.x) > thresholdXAxis) {
                    movementX = true
                }
                else if (Math.abs(pinchStartPos.y - pinch.center.y) > thresholdYAxis) {
                    movementY = true
                }
                else if (pinch.scale > (1.0 + zoomThreshold) || pinch.scale < (1.0 - zoomThreshold)) {
                    zoomY = true
                }
            }
        }

        onPinchFinished: {
            mousearea.enabled = true
            plot.plotMousePosition(-1, -1)

            clearPinchMovementState()
            pinchStartPos = Qt.point(-1, -1)
        }

        MouseArea {
            id: mousearea
            enabled: true
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            property int   lastMouseX:   -1
            property bool  wasMoved:    false
            property point startMousePos: Qt.point(-1, -1)
            property real  mouseThreshold: 4
            property bool  isPanning: false
            property int   panStartX: -1
            property bool  batchCorrect: plot.batchCorrect
            property bool  depthCorrectMode: plot.depthCorrect
            property bool  deleteFrameMode: plotCheckRec.deleteFrameChecked

            hoverEnabled: true

            Timer {
                id: longPressTimer
                interval: 500
                repeat: false
                onTriggered: {
                    if (Qt.platform.os === "android" && theme.instrumentsGrade !== 0 && !mousearea.wasMoved) {
                        plot.onCursorMoved(mousearea.mouseX, mousearea.mouseY)
                        menuBlock.position(mousearea.mouseX, mousearea.mouseY)
                    }
                }
            }

            onClicked: function(mouse) {
                lastMouseX = mouse.x
                plot.focus = true

                if (mouse.button === Qt.RightButton) {
                    if (theme.instrumentsGrade !== 0) {
                        menuBlock.position(mouse.x, mouse.y)
                    }
                }

                wasMoved = false
            }

            onPressed: function(mouse) {
                lastMouseX = mouse.x

                if (Qt.platform.os === "android") {
                    startMousePos = Qt.point(mouse.x, mouse.y)
                    longPressTimer.start()
                }

                if(batchCorrect) {
                    if (mouse.button === Qt.RightButton) {
                        menuBlock.visible = false
                        if(plot.currentFrameChecked) {
                            plot.plotMousePosition(mouse.x, mouse.y)
                        }

                        isPanning = false
                        panStartX = mouse.x
                    }

                    plot.clearBatchCorrect()
                }
                else {
                    if (mouse.button === Qt.LeftButton) {
                        menuBlock.visible = false
                        if(plot.currentFrameChecked) {
                            plot.plotMousePosition(mouse.x, mouse.y)
                        }

                        isPanning = false
                        panStartX = mouse.x
                    }

                }

                wasMoved = false

                closePlotCheckOutside(mouse.x, mouse.y)
            }

            onReleased: function(mouse) {
                lastMouseX = -1

                if (Qt.platform.os === "android") {
                    longPressTimer.stop()
                }

                if(batchCorrect) {
                    if(mouse.button === Qt.LeftButton) {
                        plot.updateBatchCorrect()
                    }
                }

                wasMoved = false
                startMousePos = Qt.point(-1, -1)
                plotReleased(indx)
                isPanning = false
                panStartX = -1
            }

            onCanceled: {
                lastMouseX = -1

                if (Qt.platform.os === "android") {
                    longPressTimer.stop()
                }

                wasMoved = false
                startMousePos = Qt.point(-1, -1)
                plotReleased(indx)
                isPanning = false
                panStartX = -1
                batchCorrect = false
                plot.clearBatchCorrect()
            }

            onPositionChanged: function(mouse) {
                // plot.onCursorMoved(mouse.x, mouse.y)
                if (Qt.platform.os === "android") {
                    if (!wasMoved) {
                        var currDelta = Math.sqrt(Math.pow((mouse.x - startMousePos.x), 2)
                                                + Math.pow((mouse.y - startMousePos.y), 2));
                        if (currDelta > mouseThreshold) {
                            wasMoved = true;
                        }
                    }
                }

                var delta  = mouse.x - lastMouseX
                lastMouseX = mouse.x

                if(batchCorrect) {
                    if (mousearea.pressedButtons & Qt.RightButton) {
                        if(!isPanning) {
                            var totalDelta = mouse.x - panStartX
                            if (Math.abs(totalDelta) > mouseThreshold) {
                                isPanning = true
                                plot.plotMousePosition(-1, -1)
                            }
                        }

                        if (isPanning) {
                            if (delta !== 0) {
                                plot.horScrollEvent(delta)
                                updateOtherPlot(indx)
                            }
                        }
                    }

                    if(mousearea.pressedButtons & Qt.LeftButton) {
                        plot.drawBatchCorrect(mouse.x, mouse.y)
                    }
                }
                else {
                    if (mousearea.pressedButtons & Qt.LeftButton) {
                        if(!isPanning) {
                            if (Math.abs(mouse.x - panStartX) > mouseThreshold) {
                                isPanning = true
                                plot.plotMousePosition(-1, -1)
                            }
                        }
                        else {
                            if (delta !== 0) {
                                plot.horScrollEvent(delta)
                                updateOtherPlot(indx)
                            }
                        }
                    }
                }


                if (depthCorrectMode) {
                    if (mousearea.pressedButtons & Qt.LeftButton) {
                        plot.drawDepthCorrect(mouseX, mouseY)
                    }

                }

                if (deleteFrameMode) {
                    plot.updateDeleteFrameMousePos(mouse.x, mouse.y)
                }

            }

            onDoubleClicked: function(mouse) {
               if (deleteFrameMode && mouse.button === Qt.LeftButton) {
                   plotCheckRec.deleteFrameFunc(mouse.x, mouse.y)
               }
            }

            onWheel: function(wheel) {
                if (wheel.modifiers & Qt.ControlModifier) {
                    let val = -wheel.angleDelta.y
                    // plot.verZoomEvent(val)
                    plotCursorChanged(indx, cursorFrom(), cursorTo())
                }
                else if (wheel.modifiers & Qt.ShiftModifier) {
                    let val = -wheel.angleDelta.y
                    plot.verScrollEvent(val)
                    plotCursorChanged(indx, cursorFrom(), cursorTo())
                }
                else {
                    plot.scaleYZoomEvent(wheel.angleDelta.y)
                }
            }
        }
    }

    onHeightChanged: {
        if(menuBlock.visible) {
            menuBlock.position(menuBlock.x, menuBlock.y)
        }
    }

    onWidthChanged: {
        if(menuBlock.visible) {
            menuBlock.position(menuBlock.x, menuBlock.y)
        }
    }


    XRButton {
        id: plotCheckBtn
        buttonText: qsTr("Echogram")
        iconSource: "qrc:/icons/ui/ripple.svg"

        anchors.left: parent.left
        anchors.leftMargin: plotIconSize * 0.5
        anchors.bottom: parent.bottom
        anchors.bottomMargin: plotIconSize * 0.5

        clickAction: function() {
            if(plotCheckBtn.settingVisible) {
                plotCheckRec.x = theme.screenWidth * 0.5 + plotIconSize * 0.5
                plotCheckRec.y = plotCheckBtn.y - plotCheckRec.height - plotIconSize * 0.1
            }
        }
    }

    Plot2DRec {
        id: plotCheckRec
        parent: mainview.contentItem
        expanded: plotCheckBtn.settingVisible
        visible: plotCheckBtn.settingVisible
        x: theme.screenWidth * 0.5 + plotIconSize * 0.5
        y: plotCheckBtn.y - plotCheckRec.height - plotIconSize * 0.1
        dragArea: plot
        targetPlot: plot
    }

    XRButton {
        id: bathymetryBtn
        buttonText: qsTr("Bathymetry")
        iconSource: "qrc:/icons/ui/file_settings.svg"
        anchors.left: plotCheckBtn.right
        anchors.leftMargin: plotIconSize * 0.5
        anchors.bottom: parent.bottom
        anchors.bottomMargin: plotIconSize * 0.5

        clickAction: function() {
            if(plotCheckBtn.settingVisible) {
                bathymetryRec.x = theme.screenWidth * 0.5 + plotIconSize * 0.5
                bathymetryRec.y = plotCheckBtn.y - bathymetryRec.height - plotIconSize * 0.1
            }
        }
    }

    BathymetryRec {
        id: bathymetryRec
        parent: mainview.contentItem
        expanded: bathymetryBtn.settingVisible
        visible: bathymetryBtn.settingVisible
        x: theme.screenWidth * 0.5 + plotIconSize * 0.5
        y: plotCheckBtn.y - bathymetryRec.height - plotIconSize * 0.1
        dragArea: plot
        targetPlot: plot
    }

    XRButton {
        id: isobathsBtn
        buttonText: qsTr("Isobaths")
        iconSource: "qrc:/XR/contour.png"
        anchors.left: bathymetryBtn.right
        anchors.leftMargin: plotIconSize * 0.5
        anchors.bottom: parent.bottom
        anchors.bottomMargin: plotIconSize * 0.5

        clickAction: function() {

        }
    }

    IsobathsRec {
        id: isobathsRec
        parent: mainview.contentItem
        expanded: isobathsBtn.settingVisible
        visible: isobathsBtn.settingVisible
        x: theme.screenWidth * 0.5 + plotIconSize * 0.5
        y: plotCheckBtn.y - isobathsRec.height - plotIconSize * 0.1
        dragArea: plot
        targetPlot: plot
    }


    RowLayout {
        id: menuBlock
        Layout.alignment: Qt.AlignHCenter
        spacing: 1
        visible: false
        Layout.margins: 0

        function position(mx, my) {
            var oy = plot.height - (my + implicitHeight)
            if(oy < 0) {
                my = my + oy
            }

            if(my < 0) {
                my = 0
            }

            var ox = plot.width - (mx - implicitWidth)
            if(ox < 0) {
                mx = mx + ox
            }

            x = mx
            y = my
            visible = true
        }
    }
}



