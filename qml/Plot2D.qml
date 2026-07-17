import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import Qt.labs.settings 1.1

import WaterFall 1.0
import AppXr 1.0
import QmlCommon 1.0

WaterFall {
    id: plot

    property bool is3dVisible: false
    property int  indx: 0
    property int  instruments: instrumentsGradeList.currentIndex
    property int  plotSize: theme.screenSize * 0.35
    property int  plotIconSize: theme.iconSize * 1.5

    // horizontal: horisontalVertical.checked
    horizontal: menuToolBar.layoutHorizontal
    property bool currentFrameChecked: currentFrame.checked
    property bool bottomLineChecked:   bottomLine.checked

    function setLevels(low, high) {
        echogramLevelsSlider.startValue   = low
        echogramLevelsSlider.stopValue    = high
        echogramLevelsSlider.startPointY  = echogramLevelsSlider.valueToPosition(low);
        echogramLevelsSlider.stopPointY   = echogramLevelsSlider.valueToPosition(high);
        echogramLevelsSlider.update()
    }

    function closeSettings() {
        plotCheckButton.checked = false
    }

    function setAim(mouseX, mouseY) {
        plotMousePosition(mouseX, mouseY, true)
    }
    function resetAim() {
        plotMousePosition(-1, -1)
    }
    function doVerZoomEvent(paramX) {
        verZoomEvent(paramX)
    }
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
            console.info("onPinchUpdated")

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
                plot.verZoomEvent(val)
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
            property int   contactMouseX: -1
            property int   contactMouseY: -1
            property bool  isPanning: false
            property int   panStartX: -1

            hoverEnabled: true

            Timer {
                id: longPressTimer
                interval: 500
                repeat: false
                onTriggered: {
                    if (Qt.platform.os === "android" && theme.instrumentsGrade !== 0 && !mousearea.wasMoved) {
                        plot.onCursorMoved(mousearea.mouseX, mousearea.mouseY)
                        mousearea.contactMouseX = mousearea.mouseX
                        mousearea.contactMouseY = mousearea.mouseY
                        plot.simplePlotMousePosition(mousearea.mouseX, mousearea.mouseY)
                        menuBlock.position(mousearea.mouseX, mousearea.mouseY)
                    }
                }
            }

            onClicked: function(mouse) {
                lastMouseX = mouse.x
                plot.focus = true

                if (mouse.button === Qt.RightButton) {
                    contactMouseX = mouse.x
                    contactMouseY = mouse.y

                    plot.simplePlotMousePosition(mouse.x, mouse.y)

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

                if (mouse.button === Qt.LeftButton) {
                    menuBlock.visible = false
                    if(plot.currentFrameChecked) {
                        plot.plotMousePosition(mouse.x, mouse.y)
                        // plotPressed(indx, mouse.x, mouse.y)
                    }

                    isPanning = false
                    panStartX = mouse.x
                }

                if (mouse.button === Qt.RightButton) {
                    contactMouseX = mouse.x
                    contactMouseY = mouse.y

                    plot.simplePlotMousePosition(mouse.x, mouse.y)
                }

                wasMoved = false
            }

            onReleased: function(mouse) {
                lastMouseX = -1

                if (Qt.platform.os === "android") {
                    longPressTimer.stop()
                }

                if (mouse.button === Qt.RightButton) {
                    contactMouseX = mouse.x
                    contactMouseY = mouse.y

                    plot.simplePlotMousePosition(mouse.x, mouse.y)
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

                if (mousearea.pressedButtons & Qt.LeftButton) {
                    // plot.plotMousePosition(mouse.x, mouse.y)
                    // plotPressed(indx, mouse.x, mouse.y)

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
                            updateOtherPlot(indx)      //同步另一个声呐视图
                        }
                    }
                }

                if (mouse.button === Qt.RightButton) {
                    contactMouseX = mouse.x
                    contactMouseY = mouse.y
                    plot.simplePlotMousePosition(mouse.x, mouse.y)
                }
            }

            onWheel: function(wheel) {
                if (wheel.modifiers & Qt.ControlModifier) {
                    let val = -wheel.angleDelta.y
                    plot.verZoomEvent(val)
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



    CheckButton {
        id: plotCheckButton
        iconSource: "qrc:/icons/ui/settings.svg"
        implicitWidth: theme.menuWidth
        anchors.left: parent.left
        anchors.bottomMargin: theme.menuWidth * 0.5 - plotIconSize
        anchors.leftMargin: plotIconSize * 0.5
        anchors.bottom: parent.bottom
    }


    MenuScroll {
        id: settingsScroll
        visible: plotCheckButton.checked
        anchors.left: parent.left
        anchors.leftMargin: plotIconSize * 0.5 + theme.menuWidth
        anchors.bottom: parent.bottom
        width: plot.width * 0.8

        MenuFrame {
            id: plotSettings
            width: parent.width
            anchors.margins: plotIconSize * 0.5

            ColumnLayout {
                spacing: plotIconSize

                RowLayout {
                    // Layout.fillWidth:  true

                    CCheck {
                        id: echogramVisible
                        Layout.fillWidth: true
                        checked: true
                        text: qsTr("Color Scheme")
                        height: plotIconSize
                        onCheckedChanged: plotEchogramVisible(checked)
                        Component.onCompleted: plotEchogramVisible(checked)
                    }

                    CCombo {
                        id: echoTheme
                        Layout.fillWidth: true
                        model: [qsTr("Blue"), qsTr("Sepia"), qsTr("WRGBD"), qsTr("WhiteBlack"), qsTr("BlackWhite")]
                        currentIndex: 0
                        height: plotIconSize

                        onCurrentIndexChanged: plotEchogramTheme(currentIndex)
                        Component.onCompleted: plotEchogramTheme(currentIndex)

                        Settings {
                            category: "Plot2D_" + plot.indx
                            property alias waterfallThemeId: echoTheme.currentIndex
                        }
                    }

                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: "#555555"
                }

                RowLayout {
                    Layout.fillWidth: true

                    // CCheck {
                    //     id: rulerVisible
                    //     implicitWidth: plotIconSize * 2
                    //     text: qsTr("Ruler")
                    //     onCheckedChanged: plotGridVerticalNumber(gridNumber.value*rulerVisible.checked)
                    // }

                    CText {
                        text: qsTr("upper(m)")
                        font.pixelSize: plotIconSize
                        horizontalAlignment: Text.AlignRight
                        Layout.alignment: Qt.AlignVCenter
                    }
                    TextField {
                       id: upperMin
                       text: (plot.minUpRng / 100).toFixed(0)
                       Layout.fillWidth: true
                       Layout.preferredWidth: plotIconSize * 4
                       horizontalAlignment: TextInput.AlignHCenter
                       font.pixelSize: plotIconSize
                       selectByMouse: true
                       validator: IntValidator { bottom: 0; top: 511;}
                       onTextChanged: {
                           let value    = parseInt(text)
                           let lowValue = parseInt(lowerMax.text)
                           if(isNaN(value)) {
                               text = "0"
                               // value = 0
                           }
                           else if(value > 511) {
                               text = "511"
                               // value = 511
                           }
                           // plot.minUpRng = value
                       }

                       // onEditingFinished: applyRange()
                       // Component.onCompleted: applyRange()
                    }

                    Item {
                        Layout.preferredWidth: plotIconSize
                    }

                    CText {
                        text: qsTr("lower(m)")
                        font.pixelSize: plotIconSize
                        horizontalAlignment: Text.AlignRight
                        Layout.alignment: Qt.AlignVCenter
                    }
                    TextField {
                        id: lowerMax
                        text: (plot.maxLoRng / 100).toFixed(0)
                        Layout.fillWidth: true
                        Layout.preferredWidth: plotIconSize * 4
                        horizontalAlignment: TextInput.AlignHCenter
                        font.pixelSize: plotIconSize
                        selectByMouse: true
                        validator: IntValidator { bottom: 1; top: 512;}
                        onTextChanged: {
                            let value = parseInt(text)
                            let upValue = parseInt(upperMin.text)
                            if(isNaN(value)) {
                                text = "1"
                                // value = 1
                            }
                            else if(value > 512) {
                                text = "512"
                                // value = 512
                            }
                            else if(value < 1) {
                                text= "1"
                                // value = 1
                            }
                            // plot.maxLoRng = value

                        }

                        // onEditingFinished: applyRange()
                        // Component.onCompleted: applyRange()
                    }

                    Item {
                        Layout.preferredWidth: plotIconSize
                    }

                    Rectangle {
                        id: applyBtn
                        width: plotIconSize * 4
                        height: plotIconSize * 1.2
                        radius: 4
                        color: mouseArea.pressed ? "#888888" : "#555555"
                        border.color: "#aaaaaa"

                        Text {
                            anchors.centerIn: parent
                            text: qsTr("Apply")
                            color: "white"
                            font.pixelSize: plotIconSize
                        }

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            onClicked: {
                                let upperVal = parseInt(upperMin.text)
                                let lowerVal = parseInt(lowerMax.text)
                                if(upperVal >= lowerVal) {
                                    upperVal = lowerVal - 1
                                }

                                plot.resetUpLoRng(upperVal, lowerVal)
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: "#555555"
                }


                RowLayout {
                    Layout.fillWidth:  true

                    XRSlider {
                        title: "Sensitivity"
                        Layout.preferredWidth: plotIconSize * 8
                        Layout.alignment: Qt.AlignVCenter

                        from: 1
                        to: 7
                        value: 3
                        onValueChanged: {
                            // plot.setSensitivity(value)
                        }
                    }

                    CCheck {
                        id: bottomLine
                        checked: false
                        text: qsTr("Bottom Line")
                        height: plotIconSize
                        onCheckedChanged: {
                            bottomLineChecked = !bottomLineChecked
                            plot.setBottomLineVisible(bottomLineChecked)
                        }
                    }

                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: "#555555"
                }

                RowLayout {
                    Layout.fillWidth: true

                    CCheck {
                        id: currentFrame
                        checked: false
                        text: qsTr("Current Frame")
                        height: plotIconSize
                        onCheckedChanged: {
                            currentFrameChecked = !currentFrameChecked
                            if(!currentFrameChecked) {
                                plot.plotMousePosition(-1, -1)
                            }
                        }
                    }

                    // CCheck {
                    //     id: addMarks
                    //     checked: false
                    //     text: qsTr("Add Marks")
                    //     height: plotIconSize
                    //     onCheckedChanged: {
                    //         currentFrameChecked = !currentFrameChecked
                    //         if(!currentFrameChecked) {
                    //             plot.plotMousePosition(-1, -1)
                    //         }
                    //     }
                    // }

                    ExpandCheckBox {
                        id: addMarks
                        checked: false
                        text: qsTr("Add Marks")
                        height: plotIconSize
                        onCheckedChanged: {
                            currentFrameChecked = !currentFrameChecked
                            if(!currentFrameChecked) {
                                plot.plotMousePosition(-1, -1)
                            }
                        }
                    }



                    CCheck {
                        id: deleteFrame
                        checked: false
                        text: qsTr("Delete Frame")
                        height: plotIconSize
                        onCheckedChanged: {
                            currentFrameChecked = !currentFrameChecked
                            if(!currentFrameChecked) {
                                plot.plotMousePosition(-1, -1)
                            }
                        }
                    }

                }

                // CCheck {
                //     id: horisontalVertical
                //     checked: true
                //     text: qsTr("Horizontal")
                // }

            }
        }
    }






    CContact {
        id: contactDialog

        onVisibleChanged: {
            if (!visible) {
                parent.focus = true

                if (accepted) {
                    plot.setContact(contactDialog.indx, contactDialog.inputFieldText)
                    updateOtherPlot(plot.indx)
                    accepted = false
                }
                contactDialog.info = ""
                contactDialog.inputFieldText = ""
            }
        }

        onDeleteButtonClicked: {
            plot.deleteContact(contactDialog.indx)
            updateOtherPlot(plot.indx)
        }

        onCopyButtonClicked: {
            plot.updateContact()
        }

        onSetActiveButtonClicked: {
            plot.setActiveContact(contactDialog.indx)
        }

        onInputAccepted: {
            contactDialog.visible = false
            plot.updateContact()
        }

        onSetButtonClicked: {
            contactDialog.visible = false
            plot.updateContact()
        }
    }

    onContactVisibleChanged: {
        contactDialog.visible = plot.contactVisible;

        if (contactDialog.visible) {
            contactDialog.info           = plot.contactInfo
            contactDialog.inputFieldText = plot.contactInfo
        }
        else {
            contactDialog.info = ""
            contactDialog.inputFieldText = ""
        }

        contactDialog.x     = plot.contactPositionX
        contactDialog.y     = plot.contactPositionY
        contactDialog.indx  = plot.contactIndx
        contactDialog.lat   = plot.contactLat
        contactDialog.lon   = plot.contactLon
        contactDialog.depth = plot.contactDepth
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
