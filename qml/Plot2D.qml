import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import Qt.labs.settings 1.1

import WaterFall 1.0


WaterFall {
    id: plot

    property bool is3dVisible: false
    property int  indx: 0
    property int  instruments: instrumentsGradeList.currentIndex
    property int  plotSize: theme.screenSize * 0.35
    property int  iconSize: plotSize * 0.08

    horizontal: horisontalVertical.checked
    property bool currentFrameChecked: currentFrame.checked

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


    RowLayout {
        id: settingsRow
        anchors.left:   parent.left
        anchors.bottom: parent.bottom
        visible:        true

        MenuFrame {
            id: leftPanel
            isOpacityControlled: true
            Layout.alignment: Qt.AlignLeft | Qt.AlignBottom
            Layout.leftMargin: (indx === 1 && !is3dVisible && height > plot.height - 130 * theme.resCoeff) ? width : 0
            Layout.bottomMargin: 20

            ColumnLayout {
                id: plotControl
                spacing: 4

                CheckButton {
                    id: plotCheckButton
                    backColor:   theme.controlBackColor
                    borderColor: theme.controlBackColor
                    checkedBorderColor: theme.controlBorderColor
                    iconSource: "qrc:/icons/ui/settings.svg"
                    implicitWidth: theme.menuWidth

                    onCheckedChanged: {
                        if (checked) {
                            settingsClicked()
                        }
                    }
                }
            }
        }


        MenuScroll {
            id: settingsScroll
            visible: plotCheckButton.checked
            Layout.alignment: Qt.AlignBottom
            Layout.bottomMargin: 10
            Layout.preferredWidth: plot.width - theme.menuWidth - 50
            Layout.maximumWidth: plot.width - theme.menuWidth - 50

            MenuFrame {
                id: plotSettings
                width: settingsScroll.availableWidth - settingsScroll.padding * 2

                ParamGroup {
                    groupName: qsTr("Sonar Viewer")
                    width: plotSettings.width - plotSettings.horizontalMargins * 2

                    RowLayout {
                        id: rowDataset
                        Layout.fillWidth: true
                        visible: instruments > 1

                        CText {
                            text: qsTr("Channels:")
                        }

                        function setChannelNamesToBackend() {
                            plotDatasetChannelFromStrings(channel1Combo.currentText, channel2Combo.currentText)
                            plotCursorChanged(indx, cursorFrom(), cursorTo())
                        }

                        CCombo  {
                            id: channel1Combo

                            property bool suppressTextSignal: false

                            Layout.fillWidth: true
                            visible: true

                            onCurrentTextChanged: {
                                if (suppressTextSignal) {
                                    return
                                }

                                rowDataset.setChannelNamesToBackend()
                            }

                            Component.onCompleted: {
                                model = dataset.channelsNameList()

                                let index = model.indexOf(core.ch1Name)
                                if (index >= 0) {
                                    channel1Combo.currentIndex = index
                                }
                            }

                            Connections {
                                target: core
                                function onChannelListUpdated() {
                                    let list = dataset.channelsNameList()

                                    channel1Combo.suppressTextSignal = true

                                    channel1Combo.model = []
                                    channel1Combo.model = list

                                    let newIndex = list.indexOf(core.ch1Name)
                                    if (newIndex >= 0) {
                                        channel1Combo.currentIndex = newIndex
                                    }
                                    else {
                                        channel1Combo.currentIndex = 0
                                    }

                                    channel1Combo.suppressTextSignal = false
                                }
                            }
                        }

                        CCombo  {
                            id: channel2Combo

                            property bool suppressTextSignal: false

                            Layout.fillWidth: true
                            visible: true

                            onCurrentTextChanged: {
                                if (suppressTextSignal) {
                                    return
                                }

                                rowDataset.setChannelNamesToBackend()
                            }


                            Component.onCompleted: {
                                model = dataset.channelsNameList()

                                let index = model.indexOf(core.ch2Name)
                                if (index >= 0) {
                                    channel2Combo.currentIndex = index
                                }
                            }

                            Connections {
                                target: core
                                function onChannelListUpdated() {
                                    let list = dataset.channelsNameList()

                                    channel2Combo.suppressTextSignal = true

                                    channel2Combo.model = []
                                    channel2Combo.model = list

                                    let newIndex = list.indexOf(core.ch2Name)

                                    if (newIndex >= 0) {
                                        channel2Combo.currentIndex = newIndex
                                    }
                                    else {
                                        channel2Combo.currentIndex = 0
                                    }

                                    channel2Combo.suppressTextSignal = false
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth:  true

                        CCheck {
                            id: echogramVisible
                            Layout.fillWidth: true
                            checked: true
                            text: qsTr("Echogram")
                            onCheckedChanged: plotEchogramVisible(checked)
                            Component.onCompleted: plotEchogramVisible(checked)
                        }

                        CCombo {
                            id: echoTheme
                            Layout.fillWidth: true
                            model: [qsTr("Blue"), qsTr("Sepia"), qsTr("WRGBD"), qsTr("WhiteBlack"), qsTr("BlackWhite")]
                            currentIndex: 0

                            onCurrentIndexChanged: plotEchogramTheme(currentIndex)
                            Component.onCompleted: plotEchogramTheme(currentIndex)

                            Settings {
                                category: "Plot2D_" + plot.indx

                                property alias waterfallThemeId: echoTheme.currentIndex
                            }
                        }

                        CCombo {
                            id: echogramTypesList
                            Layout.fillWidth: true
                            // Layout.preferredWidth: 120
                            model: [qsTr("Raw"), qsTr("Side-Scan")]
                            currentIndex: 0

                            onCurrentIndexChanged: plotEchogramCompensation(currentIndex)
                            Component.onCompleted: plotEchogramCompensation(currentIndex)

                            Settings {
                                category: "Plot2D_" + plot.indx

                                property alias echogramTypesList: echogramTypesList.currentIndex
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        // CCheck {
                        //     id: rulerVisible
                        //     implicitWidth: iconSize * 2
                        //     text: qsTr("Ruler")
                        //     onCheckedChanged: plotGridVerticalNumber(gridNumber.value*rulerVisible.checked)
                        // }

                        CText {
                            text: qsTr("upper(m)")
                            font.pixelSize: iconSize
                            horizontalAlignment: Text.AlignRight
                            Layout.alignment: Qt.AlignVCenter
                        }
                        TextField {
                           id: upperMin
                           text: (plot.minUpRng / 100).toFixed(0)
                           Layout.fillWidth: true
                           Layout.preferredWidth: iconSize * 4
                           horizontalAlignment: TextInput.AlignHCenter
                           font.pixelSize: iconSize
                           selectByMouse: true
                           validator: IntValidator { bottom: -100; top: 100;}
                           // onEditingFinished: applyRange()
                           // Component.onCompleted: applyRange()
                        }

                        Item {
                            Layout.preferredWidth: iconSize
                        }

                        CText {
                            text: qsTr("lower(m)")
                            font.pixelSize: iconSize
                            horizontalAlignment: Text.AlignRight
                            Layout.alignment: Qt.AlignVCenter
                        }
                        TextField {
                            id: lowerMax
                            text: (plot.maxLoRng / 100).toFixed(0)
                            Layout.fillWidth: true
                            Layout.preferredWidth: iconSize * 4
                            horizontalAlignment: TextInput.AlignHCenter
                            font.pixelSize: iconSize
                            selectByMouse: true
                            validator: IntValidator { bottom: -100; top: 10000;}
                            // onEditingFinished: applyRange()
                            // Component.onCompleted: applyRange()
                        }

                        Item {
                            Layout.preferredWidth: iconSize
                        }

                        Rectangle {
                            id: applyBtn
                            width: iconSize * 4
                            height: iconSize * 1.2
                            radius: 4
                            color: mouseArea.pressed ? "#888888" : "#555555"
                            border.color: "#aaaaaa"

                            Text {
                                anchors.centerIn: parent
                                text: qsTr("Apply")
                                color: "white"
                                font.pixelSize: iconSize
                            }

                            MouseArea {
                                id: mouseArea
                                anchors.fill: parent
                                onClicked: {
                                    let upperVal = parseInt(upperMin.text)
                                    let lowerVal = parseInt(lowerMax.text)
                                    plot.resetUpLoRng(upperVal, lowerVal)
                                }
                            }
                        }
                    }


                    CCheck {
                        id: currentFrame
                        checked: true
                        text: qsTr("Current Frame")
                        onCheckedChanged: {
                            currentFrameChecked = !currentFrameChecked
                            if(!currentFrameChecked) {
                                plot.plotMousePosition(-1, -1)
                            }
                        }
                    }

                    CCheck {
                        id: horisontalVertical
                        checked: true
                        text: qsTr("Horizontal")
                    }

                }
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
