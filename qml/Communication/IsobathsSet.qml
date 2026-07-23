import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts  1.12
import QtQuick.Dialogs  1.3
import Qt.labs.settings 1.1
import QtQuick.Window   2.15

import "../"
import AppXr 1.0


Item {
    id: isobathsSet

    width: toggleButton.width
    height: toggleButton.height
    z: 9999

    property bool isobathsDrawOpen:  false
    property bool bluetoothDrawOpen: false
    property int  isobathSize:  theme.screenSize * 0.35
    property int  layoutHeight: isobathSize * 0.1
    property var  targetPlot:   null
    property int  iconSize:     isobathSize * 0.05

    property int currentCommPage: 0

    property bool outlineMode:     false
    property bool isShowBoatTrack: true
    property bool isShowOutline:   true
    property bool isContours:      true
    property bool isShowIsobaths:  true
    property bool isShowBoat:      true

    Component.onCompleted: {
       IsobathsViewControlMenuController.onIsobathsVisibilityCheckBoxCheckedChanged(isShowIsobaths)
       IsobathsViewControlMenuController.onContoursVisibilityCheckBoxCheckedChanged(isShowIsobaths)
       IsobathsViewControlMenuController.onVertexVisibilityCheckBoxCheckedChanged(isShowBoat)
    }

    Connections {
        target: UdpManager
        function onSignalCancelUdpOn(isOn) {
            onOffControl.isOn = isOn
        }
    }


    // ------------------ 侧边按钮 -------------------
    ColumnLayout {
        id: toggleButton
        width: iconSize * 2
        spacing: 2

        Rectangle {
            id: isobathsToggleBtn
            width: iconSize * 1.6
            height: iconSize * 6.4
            color: "#879fc6"
            opacity: 0.75

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    isobathsDrawOpen = !isobathsDrawOpen
                    if(isobathsDrawOpen) {
                        bluetoothDrawOpen = false
                        bluetoothToggleBtn.color = "#879fc6"
                        parent.color = "#4a5f82"
                        isobathsToggleBtn.opacity = 0.95
                    }
                    else {
                        parent.color = "#879fc6"
                        isobathsToggleBtn.opacity = 0.75
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 2

                Image {
                    source: "qrc:/XR/contour.png"
                    Layout.preferredWidth: iconSize * 1.1
                    Layout.preferredHeight: iconSize * 1.1
                    Layout.alignment: Qt.AlignHCenter
                    fillMode: Image.PreserveAspectFit
                }

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: iconSize * 3.2
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("Isobaths")
                        color: "white"
                        font.pixelSize: iconSize
                        rotation: 90
                        transformOrigin: Item.Center
                    }
                }
            }
        }

        Rectangle {
            id: bluetoothToggleBtn
            width: iconSize * 1.6
            height: iconSize * 7.2
            color: "#879fc6"
            opacity: 0.75

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    bluetoothDrawOpen = !bluetoothDrawOpen
                    if(bluetoothDrawOpen) {
                        isobathsDrawOpen = false
                        isobathsToggleBtn.color = "#879fc6"
                        parent.color = "#4a5f82"
                        bluetoothToggleBtn.opacity = 0.95
                    }
                    else {
                        parent.color = "#879fc6"
                        bluetoothToggleBtn.opacity = 0.75
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 2

                Image {
                    source: "qrc:/icons/ui/focus_2.svg"
                    Layout.preferredWidth: iconSize * 1.1
                    Layout.preferredHeight: iconSize * 1.1
                    Layout.alignment: Qt.AlignHCenter
                    fillMode: Image.PreserveAspectFit
                }

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: iconSize * 4
                    Text {
                        anchors.centerIn: parent
                        text: qsTr("Bathymetry")
                        color: "white"
                        font.pixelSize: iconSize
                        rotation: 90
                        transformOrigin: Item.Center
                    }
                }
            }
        }

    }


    // ------------------------- isobaths抽屉 -------------------------
    Rectangle {
        id: isobathsContent
        width:  isobathSize
        height: isobathSize * 1.2
        anchors.top: toggleButton.top
        // anchors.right: toggleButton.left
        // anchors.rightMargin: isobathsDrawOpen ? 0 : -(width + toggleButton.width)
        anchors.left: toggleButton.right
        anchors.leftMargin: isobathsDrawOpen ? 0 : -(width + toggleButton.width * 1.5)

        color: "#dbe3f2"
        border.color: "#d8e0ef"
        border.width: 1
        radius: iconSize * 0.25

        // 拦截鼠标事件，防止点击穿透到地图
        MouseArea {
            anchors.fill: parent
            enabled: isobathsDrawOpen
            preventStealing: true
        }

        Behavior on anchors.rightMargin {
            NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
        }


        Rectangle {
            id: isobathsTitleBar
            anchors.top: parent.top
            anchors.left: parent.left
            height: iconSize * 1.5
            color: "#3498db"

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 3
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Isobaths Settings")
                font.pixelSize: iconSize
            }
        }

        ColumnLayout {
            anchors.top: isobathsTitleBar.bottom
            anchors.topMargin: iconSize
            anchors.left: parent.left
            anchors.leftMargin: iconSize
            anchors.right: parent.right
            anchors.rightMargin: iconSize
            spacing: 12

            RowLayout {
                spacing: 32
                Layout.alignment: Qt.AlignHCenter

                Button {
                    id: outlineButton
                    text: outlineMode ? qsTr("Clear Outline") : qsTr("Draw Outline")
                    font.pixelSize: iconSize
                    implicitWidth:  isobathSize * 0.4
                    Layout.preferredHeight: iconSize * 1.5
                    palette.button: "#b9c6db"

                    onClicked: {
                        outlineMode = !outlineMode
                        if(targetPlot) {
                            targetPlot.drawPolygonOutline(outlineMode)
                        }
                    }
                }

                Button {
                    id: updateBottomTrackButton
                    text: qsTr("Draw Isobaths")
                    font.pixelSize: iconSize
                    implicitWidth:  isobathSize * 0.4
                    Layout.preferredHeight: iconSize * 1.5
                    palette.button: "#b9c6db"

                    onClicked: {
                        if (targetPlot) {
                            targetPlot.doDistProcessing( 0, 1, 0, 0, 1000, 1, 0, 0, 0, 0, false)
                        }
                    }
                }

            }


            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: layoutHeight * 6
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 5
                Layout.bottomMargin: 5

                border.color: "#7f8fa6"
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    spacing: 2

                    Rectangle {
                        id: boatTrack
                        Layout.fillWidth: true
                        Layout.preferredHeight: layoutHeight
                        color: "#f9f9fb"

                        property bool checked: true

                        SequentialAnimation {
                            id: flashAnim1
                            running: false
                            loops: 1

                            ColorAnimation {
                                target: boatTrack
                                property: "color"
                                to: "#9ecbff"
                                duration: 100
                            }
                            ColorAnimation {
                                target: boatTrack
                                property: "color"
                                to: "#d6e6ff"
                                duration: 100
                            }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: iconSize * 0.5
                            anchors.rightMargin: iconSize * 0.5
                            spacing: iconSize * 0.5
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                source: "qrc:/icons/ui/route.svg"
                                width: iconSize * 1.2
                                height: iconSize * 1.2
                                fillMode: Image.PreserveAspectFit
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Rectangle {
                                width: iconSize * 1.1
                                height: iconSize * 1.1
                                radius: 5
                                border.color: "#b0b3b8"
                                border.width: 1
                                anchors.verticalCenter: parent.verticalCenter

                                Rectangle {
                                    anchors.centerIn: parent
                                    width: parent.width * 0.8
                                    height: parent.height * 0.8
                                    radius: parent.height * 0.4
                                    color: "#66E07A"
                                    visible: isShowBoatTrack
                                }
                            }

                            Text {
                                text: qsTr("Boat Track")
                                font.pixelSize: iconSize
                                color: "black"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true

                            onClicked: {
                                flashAnim1.restart()
                                isShowBoatTrack = !isShowBoatTrack
                                BoatTrackControlMenuController.onVisibilityCheckBoxCheckedChanged(isShowBoatTrack)
                            }

                            onEntered: parent.color = "#d6e6ff"
                            onExited:  parent.color = "#f9f9fb"
                        }
                    }


                    Rectangle {
                        id: showOutline
                        Layout.fillWidth: true
                        Layout.preferredHeight: layoutHeight
                        color: "#f9f9fb"

                        property bool checked: true

                        SequentialAnimation {
                            id: flashAnim2
                            running: false
                            loops: 1

                            ColorAnimation {
                                target: showOutline
                                property: "color"
                                to: "#9ecbff"
                                duration: 100
                            }
                            ColorAnimation {
                                target: showOutline
                                property: "color"
                                to: "#d6e6ff"
                                duration: 100
                            }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: iconSize * 0.5
                            anchors.rightMargin: iconSize * 0.5
                            spacing: iconSize * 0.5
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                source: "qrc:/XR/outline.svg"
                                width: iconSize * 1.2
                                height: iconSize * 1.2
                                fillMode: Image.PreserveAspectFit
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Rectangle {
                                width: iconSize * 1.1
                                height: iconSize * 1.1
                                radius: 5
                                border.color: "#b0b3b8"
                                border.width: 1
                                anchors.verticalCenter: parent.verticalCenter

                                Rectangle {
                                    anchors.centerIn: parent
                                    width: parent.width * 0.8
                                    height: parent.height * 0.8
                                    radius: parent.height * 0.4
                                    color: "#66E07A"
                                    visible: isShowOutline
                                }
                            }

                            Text {
                                text: qsTr("Track Boundary")
                                font.pixelSize: iconSize
                                color: "black"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true

                            onClicked: {
                                flashAnim2.restart()
                                isShowOutline = !isShowOutline
                                IsobathsViewControlMenuController.onOutlineVisibleChanged(isShowOutline)
                                // IsobathsViewControlMenuController.autoDrawTrackBoundary()
                            }

                            onEntered: parent.color = "#d6e6ff"
                            onExited:  parent.color = "#f9f9fb"
                        }
                    }



                    Rectangle {
                        id: contour
                        Layout.fillWidth: true
                        Layout.preferredHeight: layoutHeight
                        color: "#f9f9fb"

                        property bool checked: true

                        SequentialAnimation {
                            id: flashAnim3
                            running: false
                            loops: 1

                            ColorAnimation {
                                target: contour
                                property: "color"
                                to: "#9ecbff"
                                duration: 100
                            }
                            ColorAnimation {
                                target: contour
                                property: "color"
                                to: "#d6e6ff"
                                duration: 100
                            }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: iconSize * 0.5
                            anchors.rightMargin: iconSize * 0.5
                            spacing: iconSize * 0.5
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                source: "qrc:/XR/contour_map.svg"
                                width: iconSize * 1.2
                                height: iconSize * 1.2
                                fillMode: Image.PreserveAspectFit
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Rectangle {
                                width: iconSize * 1.1
                                height: iconSize * 1.1
                                radius: 5
                                border.color: "#b0b3b8"
                                border.width: 1
                                anchors.verticalCenter: parent.verticalCenter

                                Rectangle {
                                    anchors.centerIn: parent
                                    width: parent.width * 0.8
                                    height: parent.height * 0.8
                                    radius: parent.height * 0.4
                                    color: "#66E07A"
                                    visible: isContours
                                }
                            }

                            Text {
                                text: qsTr("Contours")
                                font.pixelSize: iconSize
                                color: "black"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true

                            onClicked: {
                                flashAnim3.restart()
                                isContours = !isContours
                                // IsobathsViewControlMenuController.onProcessStateChanged(isContours);
                                IsobathsViewControlMenuController.onContoursVisibilityCheckBoxCheckedChanged(isContours)
                            }

                            onEntered: parent.color = "#d6e6ff"
                            onExited:  parent.color = "#f9f9fb"
                        }
                    }


                    Rectangle {
                        id: isobaths
                        Layout.fillWidth: true
                        Layout.preferredHeight: layoutHeight
                        color: "#f9f9fb"

                        property bool checked: true

                        SequentialAnimation {
                            id: flashAnim_isobaths
                            running: false
                            loops: 1

                            ColorAnimation {
                                target: isobaths
                                property: "color"
                                to: "#9ecbff"
                                duration: 100
                            }
                            ColorAnimation {
                                target: isobaths
                                property: "color"
                                to: "#d6e6ff"
                                duration: 100
                            }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: iconSize * 0.5
                            anchors.rightMargin: iconSize * 0.5
                            spacing: iconSize * 0.5
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                source: "qrc:/XR/contour.png"
                                width: iconSize * 1.2
                                height: iconSize * 1.2
                                fillMode: Image.PreserveAspectFit
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Rectangle {
                                width: iconSize * 1.1
                                height: iconSize * 1.1
                                radius: 5
                                border.color: "#b0b3b8"
                                border.width: 1
                                anchors.verticalCenter: parent.verticalCenter

                                Rectangle {
                                    anchors.centerIn: parent
                                    width: parent.width * 0.8
                                    height: parent.height * 0.8
                                    radius: parent.height * 0.4
                                    color: "#66E07A"
                                    visible: isShowIsobaths
                                }
                            }

                            Text {
                                text: qsTr("Isobaths")
                                font.pixelSize: iconSize
                                color: "black"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true

                            onClicked: {
                                flashAnim_isobaths.restart()
                                isShowIsobaths = !isShowIsobaths
                                // IsobathsViewControlMenuController.onProcessStateChanged(isShowIsobaths);
                                IsobathsViewControlMenuController.onIsobathsVisibilityCheckBoxCheckedChanged(isShowIsobaths)
                            }

                            onEntered: parent.color = "#d6e6ff"
                            onExited:  parent.color = "#f9f9fb"
                        }
                    }


                    Rectangle {
                        id: showBoat
                        Layout.fillWidth: true
                        Layout.preferredHeight: layoutHeight
                        color: "#f9f9fb"

                        property bool checked: true

                        SequentialAnimation {
                            id: flashAnim4
                            running: false
                            loops: 1

                            ColorAnimation {
                                target: showBoat
                                property: "color"
                                to: "#9ecbff"
                                duration: 100
                            }
                            ColorAnimation {
                                target: showBoat
                                property: "color"
                                to: "#d6e6ff"
                                duration: 100
                            }
                        }

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: iconSize * 0.5
                            anchors.rightMargin: iconSize * 0.5
                            spacing: iconSize * 0.5
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                source: "qrc:/icons/ui/speedboat.svg"
                                width: iconSize * 1.2
                                height: iconSize * 1.2
                                fillMode: Image.PreserveAspectFit
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Rectangle {
                                width: iconSize * 1.1
                                height: iconSize * 1.1
                                radius: 5
                                border.color: "#b0b3b8"
                                border.width: 1
                                anchors.verticalCenter: parent.verticalCenter

                                Rectangle {
                                    anchors.centerIn: parent
                                    width: parent.width * 0.8
                                    height: parent.height * 0.8
                                    radius: parent.height * 0.4
                                    color: "#66E07A"
                                    visible: isShowBoat
                                }
                            }

                            Text {
                                text: qsTr("Boat")
                                font.pixelSize: iconSize
                                color: "black"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true

                            onClicked: {
                                flashAnim4.restart()
                                isShowBoat = !isShowBoat
                                NavigationArrowControlMenuController.onVisibilityCheckBoxCheckedChanged(isShowBoat)
                                // IsobathsViewControlMenuController.onVertexVisibilityCheckBoxCheckedChanged(isShowBoat)
                            }

                            onEntered: parent.color = "#d6e6ff"
                            onExited: parent.color = "#f9f9fb"
                        }
                    }

                }

            }


            // ----------------- Render Span ----------------
            RowLayout {
                spacing: 10

                Text {
                    text: qsTr("Render Span")
                    font.pixelSize: iconSize
                    Layout.fillWidth: true
                }

                Item {
                    Layout.fillWidth: true
                }

                Rectangle {
                    id: renderSpanControl
                    width:  iconSize * 3.3
                    height: iconSize * 1.3
                    radius: iconSize * 0.65
                    color:  hovered ? (renderSpanControl.isOn ? "#36D85A" : "#AFCFFF")
                                    : (renderSpanControl.isOn?  "#66E07A" : "#D0D0D2")
                    property bool isOn: true
                    property bool hovered: false

                    // 滑块
                    Rectangle {
                        id: slider
                        width:  iconSize * 1.2
                        height: iconSize * 1.2
                        radius: iconSize * 0.6
                        anchors.verticalCenter: parent.verticalCenter
                        x: renderSpanControl.isOn ? parent.width-width-2 : 2
                        color: "#FAFAFA"
                        scale: mouse1Area.pressed ? 0.9 : 1.0

                        Behavior on x {
                            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                        }
                        Behavior on scale {
                            NumberAnimation { duration: 100 }
                        }
                    }

                    Text {
                        anchors {
                            left: parent.left
                            leftMargin: 5
                            verticalCenter: parent.verticalCenter
                        }
                        text: qsTr("Auto")
                        font.pixelSize: iconSize * 0.8
                        visible: renderSpanControl.isOn
                        Behavior on opacity { NumberAnimation { duration: 150 } }
                    }

                    MouseArea {
                        id: mouse1Area
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            if(renderSpanControl.isOn) {
                                renderSpanControl.isOn = false;
                                core.setAutoRenderSpan(false);
                            }
                            else {
                                renderSpanControl.isOn = true
                                core.setAutoRenderSpan(true);
                            }
                        }
                        onEntered: renderSpanControl.hovered = true
                        onExited:  renderSpanControl.hovered = false
                    }

                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }


                SpinBoxCustom {
                    id: edgeLimitSpinBox
                    implicitWidth: isobathSize * 0.4
                    from: 10
                    to: 1000
                    stepSize: 5
                    value: 100
                    editable: false
                    Layout.rightMargin: 10
                    enabled: !renderSpanControl.isOn

                    property int decimals: 1

                    onFocusChanged: isobathsSet.focus = true

                    Component.onCompleted: {
                        IsobathsViewControlMenuController.onEdgeLimitChanged(edgeLimitSpinBox.value)
                    }

                    onValueChanged: {
                        IsobathsViewControlMenuController.onEdgeLimitChanged(edgeLimitSpinBox.value)
                    }

                    Connections {
                        target: IsobathsViewControlMenuController
                        function onEdgeLimitChanged(val) {
                            edgeLimitSpinBox.value = val
                        }
                    }
                }

            }


            // ------------------- Contour Interval -------------------
            RowLayout {
                Text {
                    text: qsTr("Contour Interval")
                    font.pixelSize: iconSize
                    Layout.fillWidth: true
                }

                SpinBoxCustom {
                    id: contourStep
                    implicitWidth: isobathSize * 0.4
                    from: 1
                    to: 200
                    stepSize: 1
                    value: 10
                    editable: false
                    Layout.rightMargin: 10

                    property real realValue: value / 10

                    onFocusChanged: isobathsSet.focus = true

                    Component.onCompleted: {
                        IsobathsViewControlMenuController.onSetSurfaceLineStepSize(contourStep.realValue)
                    }

                    onRealValueChanged: {
                        IsobathsViewControlMenuController.onSetSurfaceLineStepSize(contourStep.realValue)
                    }

                    Settings {
                        property alias isobathsSurfaceLineStepSizeSpinBox: contourStep.value
                    }
                }
            }


            // ================= 垂直缩放 =================
            RowLayout {
                spacing: 20

                Text {
                    text: qsTr("Vertical Scale")
                    font.pixelSize: iconSize
                }

                Slider {
                    id: verticalScaleSlider
                    Layout.fillWidth: true

                    from: 0.5
                    to: 10.0
                    stepSize: 0.1
                    value: IsobathsViewControlMenuController.verticalScale()

                    onValueChanged:
                        IsobathsViewControlMenuController.onVerticalScaleSliderValueChanged(value)

                    background: Rectangle {
                        x: verticalScaleSlider.leftPadding
                        y: verticalScaleSlider.topPadding + verticalScaleSlider.availableHeight / 2 - height / 2
                        width: verticalScaleSlider.availableWidth
                        height: iconSize * 0.3
                        color: "#f2f2f2"
                    }

                    handle: Rectangle {
                        x: verticalScaleSlider.leftPadding +
                           verticalScaleSlider.visualPosition * (verticalScaleSlider.availableWidth - width)
                        y: verticalScaleSlider.topPadding +
                           verticalScaleSlider.availableHeight / 2 - height / 2

                        width: iconSize * 1.2
                        height: iconSize * 1.2
                        radius: width / 2

                        color: verticalScaleSlider.pressed ? "#dddddd" : "#f6f6f6"
                        border.color: "#aaaaaa"
                    }
                }
            }

        }
    }



    property bool isShowDataPanel: false

    onVisibleChanged: {
        BleManager.setBleLiveScanningVisible(visible)
    }

    Connections {
        target: BleManager
        function onConnectedChanged(connected) {
            switchControl.isOn = connected
            readControl.isReading  = connected
            root.isShowDataPanel = true
            // root.visible = false
        }
    }






    // ----------------- Bathymetry Config抽屉面板------------------
    Rectangle {
        id: bathymetryConfigContent
        width:  isobathSize * 1.6
        height: isobathSize * 0.6
        anchors.top:  toggleButton.top
        anchors.left: toggleButton.right
        anchors.leftMargin: bluetoothDrawOpen ? 0 : -(width + toggleButton.width * 1.5)

        color: "#dbe3f2"
        border.color: "#d8e0ef"
        border.width: 1
        radius: iconSize * 0.25

        // 拦截鼠标事件，防止点击穿透到地图
        MouseArea {
            anchors.fill: parent
            enabled: bluetoothDrawOpen
            preventStealing: true
        }

        Behavior on anchors.rightMargin {
            NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
        }


        Rectangle {
            id: bathymetryConfigTitle
            anchors.top: parent.top
            anchors.left: parent.left
            height: iconSize * 1.5

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 3
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Bathymetry Config")
                font.pixelSize: iconSize
            }
        }


        ColumnLayout {
            anchors.top: bathymetryConfigTitle.bottom
            anchors.topMargin: iconSize
            anchors.left: parent.left
            anchors.leftMargin: iconSize
            anchors.right: parent.right
            anchors.rightMargin: iconSize
            spacing: 12


            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: qsTr("Parameter:")
                    font.pixelSize: iconSize
                    verticalAlignment: Text.AlignVCenter
                    // width: 200 //布局中，width不起效果xxx
                    // implicitWidth: 200  //Text在布局中implicitWidth仅读不能赋值xxx
                }

                Text {
                    text: qsTr("Sound Velocity")
                    font.pixelSize: iconSize
                    verticalAlignment: Text.AlignVCenter
                }


                TextField {
                    id: soundSpeedField
                    Layout.preferredWidth: iconSize * 3.2
                    Layout.preferredHeight: iconSize * 1.5
                    text: "1500"
                    font.pixelSize: iconSize * 0.9
                    horizontalAlignment: TextInput.AlignHCenter
                    topPadding: 0
                    bottomPadding: 0
                    selectByMouse: true
                    validator: IntValidator { bottom: 1; top: 3000 }
                }

                Text {
                    text: "m/s"
                    font.pixelSize: iconSize
                    verticalAlignment: Text.AlignVCenter
                }


                Rectangle {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 2
                    color: "#888888"
                }

                Text {
                    text: "Draft"
                    font.pixelSize: iconSize
                    verticalAlignment: Text.AlignVCenter
                }

                TextField {
                    id: draftOffsetField
                    Layout.preferredWidth: iconSize * 3
                    Layout.preferredHeight: iconSize * 1.5
                    text: "0"
                    font.pixelSize: iconSize * 0.9
                    horizontalAlignment: TextInput.AlignHCenter
                    topPadding: 0
                    bottomPadding: 0
                    selectByMouse: true
                    validator: IntValidator { bottom: 1; top: 3000 }
                }

                Text {
                    text: "cm"
                    font.pixelSize: iconSize
                    verticalAlignment: Text.AlignVCenter
                }

                Rectangle {
                    id: applyBtn
                    width: iconSize * 3
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
                            let soundVelocity = parseInt(soundSpeedField.text)
                            let draftOffset   = parseInt(draftOffsetField.text)
                            core.bathyMetryConfigApply(soundVelocity, draftOffset)
                        }
                    }
                }

            }


            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "#555555"
            }


            // 第二行:
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                CheckBox {
                    id: depthFilterCheck
                    font.pixelSize: iconSize
                    rightPadding: 0
                    checked: false
                    onCheckedChanged: {
                        core.setDepthFilterVisible(checked, depthFilterXRSlider.value)
                    }
                }
                XRSlider {
                    id: depthFilterXRSlider
                    title: qsTr("Depth Filter")
                    Layout.preferredWidth: plotIconSize * 6
                    Layout.alignment: Qt.AlignVCenter
                    fontSize: iconSize
                    spacing: iconSize * 0.3
                    sliderLen: iconSize * 5

                    from: 1
                    to: 4
                    value: 1
                    onValueChanged: {
                        console.log("深度滤波:")
                        core.setDepthFilterVisible(depthFilterCheck.checked, depthFilterXRSlider.value)
                    }
                }

                Rectangle {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 2
                    color: "#888888"
                }

                XRSlider {
                    title: "Outline Correct"
                    Layout.preferredWidth: plotIconSize * 6
                    Layout.alignment: Qt.AlignVCenter
                    fontSize: iconSize
                    spacing: iconSize * 0.1
                    sliderLen: iconSize * 5

                    from: 1
                    to: 4
                    value: 1
                    onValueChanged: {
                        // plot.setSensitivity(value)
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
                spacing: 10

                CheckBox {
                    id: attitudeCorrectCheck
                    text: qsTr("Attitude Correct")
                    font.pixelSize: iconSize
                    checked: false
                    onCheckedChanged: {
                        console.log("Attitude Correct......", checked)
                        // attitudeCorrectionEnable = checked
                    }
                }

                Rectangle {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 2
                    color: "#888888"
                }

                CheckBox {
                    id: batchCorrectionCheck
                    text: qsTr("Batch Correct")
                    font.pixelSize: iconSize
                    checked: false
                    onCheckedChanged: {
                        core.batchCorrect = checked
                    }
                }

                Rectangle {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 2
                    color: "#888888"
                }

                CheckBox {
                    id: depthCorrectionCheck
                    text: qsTr("Depth Correct")
                    font.pixelSize: iconSize
                    checked: false
                    onCheckedChanged: {
                        core.depthCorrect = checked
                    }
                }


            }



        }


    }












































/*
    // ----------------- Bluetooth Live Data 抽屉 ------------------
    Rectangle {
        id: bluetoothContent
        width:  isobathSize
        height: isobathSize * 0.8
        anchors.top:   toggleButton.top
        // anchors.right: toggleButton.left
        // anchors.rightMargin: bluetoothDrawOpen ? 0 : -(width + toggleButton.width)
        anchors.left: toggleButton.right
        anchors.leftMargin: bluetoothDrawOpen ? 0 : -(width + toggleButton.width * 1.5)

        color: "#f0f0f0"
        border.color: "#3498db"
        border.width: 1
        radius: 5

        // 拦截鼠标事件，防止点击穿透到地图
        MouseArea {
            anchors.fill: parent
            enabled: bluetoothDrawOpen
            preventStealing: true
        }

        Rectangle {
            anchors.fill: parent
            color: "#dbe3f2"
        }

        Behavior on anchors.rightMargin {
            NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
        }


        Rectangle {
            id: bluetoothTitleBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            height: iconSize * 1.5
            color: "#9fb6cd"

            RowLayout {
                anchors.fill: parent
                spacing: 1

                Repeater {
                    model: [ qsTr("Bluetooth"), qsTr("WiFi"), qsTr("SerialPort") ]

                    delegate: Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        color: currentCommPage === index ? "#ffffff" : "#9fb6cd"

                        Text {
                            anchors.centerIn: parent
                            text: modelData
                            font.pixelSize: iconSize
                            font.bold: currentCommPage === index
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                currentCommPage = index
                            }
                        }
                    }
                }
            }
        }

        Item {
            id: bluetoothPage
            visible: currentCommPage === 0
            anchors.top: bluetoothTitleBar.bottom
            anchors.left:         parent.left
            anchors.right:        parent.right
            anchors.bottom:       parent.bottom
            anchors.margins: iconSize * 0.5
            ColumnLayout
            {
                anchors.fill: parent

                spacing: 10

                RowLayout
                {
                    anchors.margins: 10
                    spacing: 10

                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.preferredWidth: 2
                        Layout.minimumWidth: 2

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 4

                            Rectangle {
                                id: keepBoatView
                                Layout.fillWidth: true
                                Layout.preferredHeight: layoutHeight
                                radius: layoutHeight * 0.2
                                color: "#f9f9fb"
                                border.color: "#b0b3b8"
                                border.width: 2

                                property bool checked: true

                                SequentialAnimation {
                                   id: flashAnim11
                                   running: false
                                   loops: 1

                                   ColorAnimation {
                                       target: keepBoatView
                                       property: "color"
                                       to: "#9ecbff"
                                       duration: 100
                                   }
                                   ColorAnimation {
                                       target: keepBoatView
                                       property: "color"
                                       to: "#d6e6ff"
                                       duration: 100
                                   }
                                }

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: iconSize * 0.5
                                    anchors.rightMargin: iconSize * 0.5
                                    spacing: iconSize * 0.5
                                    anchors.verticalCenter: parent.verticalCenter

                                    Rectangle {
                                        width: iconSize * 1.1
                                        height: iconSize * 1.1
                                        radius: 5
                                        border.color: "#b0b3b8"
                                        border.width: 1
                                        anchors.verticalCenter: parent.verticalCenter

                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: parent.width * 0.8
                                            height: parent.height * 0.8
                                            radius: parent.height * 0.4
                                            color: "#66E07A"
                                        }
                                    }

                                    Text {
                                        text: qsTr("Keep Boat in View")
                                        font.pixelSize: iconSize
                                        color: "black"
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true

                                    onClicked: {
                                        flashAnim11.restart()
                                        if(switchControl.isOn) {
                                            core.location(1)
                                        }

                                    }

                                    onEntered: parent.color = "#d6e6ff"
                                    onExited: parent.color = "#f9f9fb"
                                }
                            }


                            Rectangle {
                                id: showDataPanel
                                Layout.fillWidth: true
                                Layout.preferredHeight: layoutHeight
                                radius: layoutHeight * 0.2
                                color: "#f9f9fb"
                                border.color: "#b0b3b8"
                                border.width: 2

                                property bool checked: true

                                SequentialAnimation {
                                   id: flashAnim22
                                   running: false
                                   loops: 1

                                   ColorAnimation {
                                       target: showDataPanel
                                       property: "color"
                                       to: "#9ecbff"
                                       duration: 100
                                   }
                                   ColorAnimation {
                                       target: showDataPanel
                                       property: "color"
                                       to: "#d6e6ff"
                                       duration: 100
                                   }
                                }

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: iconSize * 0.5
                                    anchors.rightMargin: iconSize * 0.5
                                    spacing: iconSize * 0.6
                                    anchors.verticalCenter: parent.verticalCenter

                                    Rectangle {
                                        width: iconSize * 1.1
                                        height: iconSize * 1.1
                                        radius: 5
                                        border.color: "#b0b3b8"
                                        border.width: 1
                                        anchors.verticalCenter: parent.verticalCenter

                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: parent.width * 0.8
                                            height: parent.height * 0.8
                                            radius: parent.height * 0.4
                                            color: "#66E07A"
                                            visible: isShowDataPanel
                                        }
                                    }

                                    Text {
                                        text: qsTr("Show Data Panel")
                                        font.pixelSize: iconSize
                                        color: "black"
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true

                                    onClicked: {
                                        flashAnim22.restart()
                                        isShowDataPanel = !isShowDataPanel
                                    }

                                    onEntered: parent.color = "#d6e6ff"
                                    onExited:  parent.color = "#f9f9fb"
                                }
                            }

                        }
                    }

                    Item {
                        id: rootItem
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Rectangle {
                            id: centerRect
                            color: "transparent"
                            width: layoutHeight * 2
                            anchors.centerIn: parent

                            ColumnLayout {
                                id: mainCol
                                anchors.centerIn: parent
                                spacing: 20

                                Rectangle {
                                    id: switchControl
                                    width:  layoutHeight * 2.2
                                    height: layoutHeight
                                    radius: layoutHeight * 0.3
                                    color:  hovered ? (switchControl.isOn ? "#36D85A" : "#D6E6FF")
                                                    : (switchControl.isOn?  "#66E07A" : "#D0D0D2")
                                    property bool isOn: false
                                    property bool hovered: false

                                    // 滑块
                                    Rectangle {
                                        width:  layoutHeight * 0.9
                                        height: layoutHeight * 0.9
                                        radius: layoutHeight * 0.45
                                        anchors.verticalCenter: parent.verticalCenter
                                        x: switchControl.isOn ? parent.width-width-2 : 2
                                        color: "#FAFAFA"
                                        scale: mouse1Area2.pressed ? 0.9 : 1.0

                                        Behavior on x {
                                            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                                        }
                                        Behavior on scale {
                                            NumberAnimation { duration: 100 }
                                        }
                                    }

                                    Text {
                                        anchors {
                                            left: parent.left
                                            leftMargin: 10
                                            verticalCenter: parent.verticalCenter
                                        }
                                        text: qsTr("ON")
                                        font.pixelSize: iconSize * 0.8
                                        font.bold: true
                                        visible: switchControl.isOn
                                        Behavior on opacity { NumberAnimation { duration: 150 } }
                                    }

                                    Text {
                                        anchors {
                                            right: parent.right
                                            rightMargin: 10
                                            verticalCenter: parent.verticalCenter
                                        }
                                        text: qsTr("OFF")
                                        font.pixelSize: iconSize * 0.8
                                        font.bold: true
                                        visible: !switchControl.isOn
                                        Behavior on opacity { NumberAnimation { duration: 150 } }
                                    }

                                    MouseArea {
                                        id: mouse1Area2
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onClicked: {
                                            if(switchControl.isOn) {
                                                switchControl.isOn = false;
                                                BleManager.operateBleOnOff(false)
                                            } else {
                                                BleManager.operateBleOnOff(true)
                                            }
                                        }
                                        onEntered: switchControl.hovered = true
                                        onExited:  switchControl.hovered = false
                                    }

                                    Behavior on color {
                                        ColorAnimation { duration: 200 }
                                    }
                                }

                                Rectangle {
                                    id: readControl
                                    width:  layoutHeight * 2.2
                                    height: layoutHeight
                                    radius: layoutHeight * 0.3
                                    color:  hovered ? (readControl.isReading ? "#36D85A" : "#D6E6FF")
                                                    : (readControl.isReading?  "#66E07A" : "#D0D0D2")
                                    property bool isReading: false
                                    property bool hovered: false

                                    // 滑块
                                    Rectangle {
                                        width:  layoutHeight * 0.9
                                        height: layoutHeight * 0.9
                                        radius: layoutHeight * 0.45
                                        anchors.verticalCenter: parent.verticalCenter
                                        x: readControl.isReading ? parent.width-width-2 : 2
                                        color: "#FAFAFA"
                                        scale: mouse2Area2.pressed ? 0.9 : 1.0

                                        Behavior on x {
                                            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                                        }
                                        Behavior on scale {
                                            NumberAnimation { duration: 100 }
                                        }
                                    }

                                    Text {
                                        anchors {
                                            left: parent.left
                                            leftMargin: 5
                                            verticalCenter: parent.verticalCenter
                                        }
                                        text: qsTr("Read")
                                        font.pixelSize: iconSize * 0.8
                                        font.bold: true
                                        visible: readControl.isReading
                                        Behavior on opacity { NumberAnimation { duration: 150 } }
                                    }

                                    Text {
                                        anchors {
                                            right: parent.right
                                            rightMargin: 5
                                            verticalCenter: parent.verticalCenter
                                        }
                                        text: qsTr("Pause")
                                        font.pixelSize: iconSize * 0.8
                                        font.bold: true
                                        visible: !readControl.isReading
                                        Behavior on opacity { NumberAnimation { duration: 150 } }
                                    }

                                    MouseArea {
                                        id: mouse2Area2
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onClicked: {
                                            readControl.isReading = !readControl.isReading
                                            BleManager.setDataReading(readControl.isReading)
                                        }
                                        onEntered: readControl.hovered = true
                                        onExited:  readControl.hovered = false
                                    }

                                    Behavior on color {
                                        ColorAnimation { duration: 200 }
                                    }
                                }
                            }

                        }
                    }

                }


                Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 3; color: "#7f8c8d" }


                ColumnLayout
                {
                    anchors.margins: 15
                    spacing: 1

                    Text {
                        anchors.margins: 5
                        font.pixelSize: iconSize * 0.9
                        text: qsTr("Toslon BLE Devices List:")
                        verticalAlignment: Text.AlignVCenter
                    }

                    // 设备列表
                    GroupBox {
                        Layout.fillWidth: true
                        Layout.preferredHeight: layoutHeight * 3
                        background: Rectangle {
                            color: "#F5F5F5"
                            radius: layoutHeight * 0.3
                            border.color: "#bdc3c7"
                        }

                        ListView {
                            id: deviceList
                            anchors.fill: parent
                            anchors.margins: 5
                            model: BleManager ? BleManager.devices : ""
                            clip: true

                            delegate: Rectangle
                            {
                                width: deviceList.width
                                height: layoutHeight * 0.8
                                radius: 4

                                readonly property bool noDevices: (modelData === qsTr("No Devices Found"))

                                border.width: noDevices ? 1 : (switchControl.isOn ? 2 : 1)
                                border.color: noDevices ? "#ecf0f1" : (switchControl.isOn ? "#3498db" : "#ecf0f1")
                                color: noDevices ? "white" : (switchControl.isOn ? "#d6eaf8" : (mouseArea2.containsMouse ? "#d6e6ff" : "white"))

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 15
                                    anchors.rightMargin: 15

                                    Text {
                                        Layout.fillWidth: true
                                        text: modelData
                                        color: "#2c3e50"
                                        font.pixelSize: iconSize * 0.9
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    Text {
                                        visible: !noDevices
                                        text: switchControl.isOn ? qsTr("Connected") : qsTr("Disconnected")
                                        color: switchControl.isOn ? "#36D85A" : "#7f8c8d"
                                        font.pixelSize: iconSize * 0.6
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }

                                MouseArea {
                                    id: mouseArea2
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked:{
                                        if(!noDevices && BleManager){
                                            BleManager.connectToDevice(index)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

        }




        Item {
            id: wifiPage

            visible: currentCommPage === 1

            anchors {
                top: bluetoothTitleBar.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: iconSize
                spacing: iconSize * 0.8

                Text {
                    text: qsTr("Tip: Ensure connected to WiFi \"FishFind\"")
                    font.pixelSize: iconSize * 0.9
                }

                RowLayout {
                    Layout.fillWidth: true

                    Text {
                        text: qsTr("Local IP:")
                        font.pixelSize: iconSize
                        font.bold: true
                        Layout.preferredWidth: iconSize * 6
                    }

                    TextField {
                        id: localIpEdit
                        Layout.fillWidth: true
                        placeholderText: "127.0.0.1"
                        readOnly: true
                        selectByMouse: false
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    Text {
                        text: qsTr("Remote IP:")
                        font.pixelSize: iconSize
                        font.bold: true
                        Layout.preferredWidth: iconSize * 6
                    }

                    TextField {
                        id: targetIpEdit
                        Layout.fillWidth: true
                        // text:
                        placeholderText: ".  .  .  ."
                        selectByMouse: true

                        onTextChanged: UdpManager.remoteIp = text
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    Text {
                        text: qsTr("Port:")
                        font.pixelSize: iconSize
                        font.bold: true
                        Layout.preferredWidth: iconSize * 6
                    }

                    TextField {
                        id: targetPortEdit
                        Layout.fillWidth: true
                        // text: udpManager.remotePort
                        validator: IntValidator {
                            bottom: 1
                            top: 65535
                        }
                        onTextChanged: UdpManager.remotePort = text
                    }

                }


                RowLayout {
                    id: udpControl
                    Layout.alignment: Qt.AlignHCenter
                    spacing: iconSize * 3


                    Rectangle {
                        id: onOffControl
                        width:  layoutHeight * 2.2
                        height: layoutHeight
                        radius: layoutHeight * 0.3
                        color:  hovered ? (onOffControl.isOn ? "#36D85A" : "#D6E6FF")
                                        : (onOffControl.isOn?  "#66E07A" : "#D0D0D2")
                        property bool isOn: false
                        property bool hovered: false

                        // 滑块
                        Rectangle {
                            width:  layoutHeight * 0.9
                            height: layoutHeight * 0.9
                            radius: layoutHeight * 0.45
                            anchors.verticalCenter: parent.verticalCenter
                            x: onOffControl.isOn ? parent.width-width-2 : 2
                            color: "#FAFAFA"

                            Behavior on x {
                                NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                            }

                        }

                        Text {
                            anchors {
                                left: parent.left
                                leftMargin: 10
                                verticalCenter: parent.verticalCenter
                            }
                            text: qsTr("ON")
                            font.pixelSize: iconSize * 0.8
                            font.bold: true
                            visible: onOffControl.isOn
                            Behavior on opacity { NumberAnimation { duration: 150 } }
                        }

                        Text {
                            anchors {
                                right: parent.right
                                rightMargin: 10
                                verticalCenter: parent.verticalCenter
                            }
                            text: qsTr("OFF")
                            font.pixelSize: iconSize * 0.8
                            font.bold: true
                            visible: !onOffControl.isOn
                            Behavior on opacity { NumberAnimation { duration: 150 } }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                UdpManager.openUdp(!onOffControl.isOn)
                            }
                            onEntered: onOffControl.hovered = true
                            onExited:  onOffControl.hovered = false
                        }

                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }

                    Rectangle {
                        id: readControl2
                        width:  layoutHeight * 2.2
                        height: layoutHeight
                        radius: layoutHeight * 0.3
                        color:  hovered ? (readControl2.isReading ? "#36D85A" : "#D6E6FF")
                                        : (readControl2.isReading?  "#66E07A" : "#D0D0D2")
                        property bool isReading: true
                        property bool hovered: true

                        // 滑块
                        Rectangle {
                            width:  layoutHeight * 0.9
                            height: layoutHeight * 0.9
                            radius: layoutHeight * 0.45
                            anchors.verticalCenter: parent.verticalCenter
                            x: readControl2.isReading ? parent.width-width-2 : 2
                            color: "#FAFAFA"

                            Behavior on x {
                                NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                            }
                        }

                        Text {
                            anchors {
                                left: parent.left
                                leftMargin: 5
                                verticalCenter: parent.verticalCenter
                            }
                            text: qsTr("Read")
                            font.pixelSize: iconSize * 0.8
                            font.bold: true
                            visible: readControl2.isReading
                            Behavior on opacity { NumberAnimation { duration: 150 } }
                        }

                        Text {
                            anchors {
                                right: parent.right
                                rightMargin: 5
                                verticalCenter: parent.verticalCenter
                            }
                            text: qsTr("Pause")
                            font.pixelSize: iconSize * 0.8
                            font.bold: true
                            visible: !readControl2.isReading
                            Behavior on opacity { NumberAnimation { duration: 150 } }
                        }

                        MouseArea {
                            id: readPauseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                readControl2.isReading = !readControl2.isReading
                                UdpManager.setDataReading(readControl2.isReading)
                            }
                            onEntered: readControl2.hovered = true
                            onExited:  readControl2.hovered = false
                        }

                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }
                }


            }
        }






        Item {
            id: serialPortPage
            visible: currentCommPage === 2
            anchors.top: bluetoothTitleBar.bottom
            anchors.left:         parent.left
            anchors.right:        parent.right
            anchors.bottom:       parent.bottom
            anchors.margins: iconSize * 0.5
            ColumnLayout
            {
                anchors.fill: parent

                spacing: 10

                RowLayout
                {
                    anchors.margins: 10
                    spacing: 10

                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.preferredWidth: 2
                        Layout.minimumWidth: 2

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 4

                            Rectangle {
                                id: keepBoatView2
                                Layout.fillWidth: true
                                Layout.preferredHeight: layoutHeight
                                radius: layoutHeight * 0.2
                                color: "#f9f9fb"
                                border.color: "#b0b3b8"
                                border.width: 2

                                property bool checked: true

                                SequentialAnimation {
                                   id: flashAnim112
                                   running: false
                                   loops: 1

                                   ColorAnimation {
                                       target: keepBoatView
                                       property: "color"
                                       to: "#9ecbff"
                                       duration: 100
                                   }
                                   ColorAnimation {
                                       target: keepBoatView
                                       property: "color"
                                       to: "#d6e6ff"
                                       duration: 100
                                   }
                                }

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: iconSize * 0.5
                                    anchors.rightMargin: iconSize * 0.5
                                    spacing: iconSize * 0.5
                                    anchors.verticalCenter: parent.verticalCenter

                                    Rectangle {
                                        width: iconSize * 1.1
                                        height: iconSize * 1.1
                                        radius: 5
                                        border.color: "#b0b3b8"
                                        border.width: 1
                                        anchors.verticalCenter: parent.verticalCenter

                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: parent.width * 0.8
                                            height: parent.height * 0.8
                                            radius: parent.height * 0.4
                                            color: "#66E07A"
                                        }
                                    }

                                    Text {
                                        text: qsTr("Keep Boat in View")
                                        font.pixelSize: iconSize
                                        color: "black"
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true

                                    onClicked: {
                                        flashAnim11.restart()
                                        if(switchControl.isOn) {
                                            core.location(1)
                                        }

                                    }

                                    onEntered: parent.color = "#d6e6ff"
                                    onExited: parent.color = "#f9f9fb"
                                }
                            }


                            Rectangle {
                                id: showDataPanel2
                                Layout.fillWidth: true
                                Layout.preferredHeight: layoutHeight
                                radius: layoutHeight * 0.2
                                color: "#f9f9fb"
                                border.color: "#b0b3b8"
                                border.width: 2

                                property bool checked: true

                                SequentialAnimation {
                                   id: flashAnim222
                                   running: false
                                   loops: 1

                                   ColorAnimation {
                                       target: showDataPanel
                                       property: "color"
                                       to: "#9ecbff"
                                       duration: 100
                                   }
                                   ColorAnimation {
                                       target: showDataPanel
                                       property: "color"
                                       to: "#d6e6ff"
                                       duration: 100
                                   }
                                }

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: iconSize * 0.5
                                    anchors.rightMargin: iconSize * 0.5
                                    spacing: iconSize * 0.6
                                    anchors.verticalCenter: parent.verticalCenter

                                    Rectangle {
                                        width: iconSize * 1.1
                                        height: iconSize * 1.1
                                        radius: 5
                                        border.color: "#b0b3b8"
                                        border.width: 1
                                        anchors.verticalCenter: parent.verticalCenter

                                        Rectangle {
                                            anchors.centerIn: parent
                                            width: parent.width * 0.8
                                            height: parent.height * 0.8
                                            radius: parent.height * 0.4
                                            color: "#66E07A"
                                            visible: isShowDataPanel
                                        }
                                    }

                                    Text {
                                        text: qsTr("Show Data Panel")
                                        font.pixelSize: iconSize
                                        color: "black"
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true

                                    onClicked: {
                                        flashAnim22.restart()
                                        isShowDataPanel = !isShowDataPanel
                                    }

                                    onEntered: parent.color = "#d6e6ff"
                                    onExited:  parent.color = "#f9f9fb"
                                }
                            }

                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Rectangle {
                            id: centerRect2
                            color: "transparent"
                            width: layoutHeight * 2
                            anchors.centerIn: parent

                            ColumnLayout {
                                id: mainCol2
                                anchors.centerIn: parent
                                spacing: 20

                                Rectangle {
                                    id: switchControl2
                                    width:  layoutHeight * 2.2
                                    height: layoutHeight
                                    radius: layoutHeight * 0.3
                                    color:  hovered ? (switchControl2.isOn ? "#36D85A" : "#D6E6FF")
                                                    : (switchControl2.isOn?  "#66E07A" : "#D0D0D2")
                                    property bool isOn: false
                                    property bool hovered: false

                                    // 滑块
                                    Rectangle {
                                        width:  layoutHeight * 0.9
                                        height: layoutHeight * 0.9
                                        radius: layoutHeight * 0.45
                                        anchors.verticalCenter: parent.verticalCenter
                                        x: switchControl2.isOn ? parent.width-width-2 : 2
                                        color: "#FAFAFA"
                                        scale: mouse1Area22.pressed ? 0.9 : 1.0

                                        Behavior on x {
                                            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                                        }
                                        Behavior on scale {
                                            NumberAnimation { duration: 100 }
                                        }
                                    }

                                    Text {
                                        anchors {
                                            left: parent.left
                                            leftMargin: 10
                                            verticalCenter: parent.verticalCenter
                                        }
                                        text: qsTr("ON")
                                        font.pixelSize: iconSize * 0.8
                                        font.bold: true
                                        visible: switchControl2.isOn
                                        Behavior on opacity { NumberAnimation { duration: 150 } }
                                    }

                                    Text {
                                        anchors {
                                            right: parent.right
                                            rightMargin: 10
                                            verticalCenter: parent.verticalCenter
                                        }
                                        text: qsTr("OFF")
                                        font.pixelSize: iconSize * 0.8
                                        font.bold: true
                                        visible: !switchControl2.isOn
                                        Behavior on opacity { NumberAnimation { duration: 150 } }
                                    }

                                    MouseArea {
                                        id: mouse1Area22
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onClicked: {
                                            if(switchControl2.isOn) {
                                                switchControl2.isOn = false;
                                                BleManager.operateBleOnOff(false)
                                            } else {
                                                BleManager.operateBleOnOff(true)
                                            }
                                        }
                                        onEntered: switchControl2.hovered = true
                                        onExited:  switchControl2.hovered = false
                                    }

                                    Behavior on color {
                                        ColorAnimation { duration: 200 }
                                    }
                                }

                                Rectangle {
                                    id: readControl22
                                    width:  layoutHeight * 2.2
                                    height: layoutHeight
                                    radius: layoutHeight * 0.3
                                    color:  hovered ? (readControl22.isReading ? "#36D85A" : "#D6E6FF")
                                                    : (readControl22.isReading?  "#66E07A" : "#D0D0D2")
                                    property bool isReading: false
                                    property bool hovered: false

                                    // 滑块
                                    Rectangle {
                                        width:  layoutHeight * 0.9
                                        height: layoutHeight * 0.9
                                        radius: layoutHeight * 0.45
                                        anchors.verticalCenter: parent.verticalCenter
                                        x: readControl22.isReading ? parent.width-width-2 : 2
                                        color: "#FAFAFA"
                                        scale: mouse2Area22.pressed ? 0.9 : 1.0

                                        Behavior on x {
                                            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                                        }
                                        Behavior on scale {
                                            NumberAnimation { duration: 100 }
                                        }
                                    }

                                    Text {
                                        anchors {
                                            left: parent.left
                                            leftMargin: 5
                                            verticalCenter: parent.verticalCenter
                                        }
                                        text: qsTr("Read")
                                        font.pixelSize: iconSize * 0.8
                                        font.bold: true
                                        visible: readControl2.isReading
                                        Behavior on opacity { NumberAnimation { duration: 150 } }
                                    }

                                    Text {
                                        anchors {
                                            right: parent.right
                                            rightMargin: 5
                                            verticalCenter: parent.verticalCenter
                                        }
                                        text: qsTr("Pause")
                                        font.pixelSize: iconSize * 0.8
                                        font.bold: true
                                        visible: !readControl2.isReading
                                        Behavior on opacity { NumberAnimation { duration: 150 } }
                                    }

                                    MouseArea {
                                        id: mouse2Area22
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onClicked: {
                                            readControl2.isReading = !readControl2.isReading
                                            BleManager.setDataReading(readControl2.isReading)
                                        }
                                        onEntered: readControl2.hovered = true
                                        onExited:  readControl2.hovered = false
                                    }

                                    Behavior on color {
                                        ColorAnimation { duration: 200 }
                                    }
                                }
                            }

                        }
                    }

                }


                Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 3; color: "#7f8c8d" }


                ColumnLayout
                {
                    anchors.margins: 15
                    spacing: 1

                    Text {
                        anchors.margins: 5
                        font.pixelSize: iconSize * 0.9
                        text: qsTr("Toslon BLE Devices List:")
                        verticalAlignment: Text.AlignVCenter
                    }

                    // 设备列表
                    GroupBox {
                        Layout.fillWidth: true
                        Layout.preferredHeight: layoutHeight * 3
                        background: Rectangle {
                            color: "#F5F5F5"
                            radius: layoutHeight * 0.3
                            border.color: "#bdc3c7"
                        }

                        ListView {
                            id: deviceList2
                            anchors.fill: parent
                            anchors.margins: 5
                            model: BleManager ? BleManager.devices : ""
                            clip: true

                            delegate: Rectangle
                            {
                                width: deviceList2.width
                                height: layoutHeight * 0.8
                                radius: 4

                                readonly property bool noDevices: (modelData === qsTr("No Devices Found"))

                                border.width: noDevices ? 1 : (switchControl2.isOn ? 2 : 1)
                                border.color: noDevices ? "#ecf0f1" : (switchControl2.isOn ? "#3498db" : "#ecf0f1")
                                color: noDevices ? "white" : (switchControl2.isOn ? "#d6eaf8" : (mouseArea22.containsMouse ? "#d6e6ff" : "white"))

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 15
                                    anchors.rightMargin: 15

                                    Text {
                                        Layout.fillWidth: true
                                        text: modelData
                                        color: "#2c3e50"
                                        font.pixelSize: iconSize * 0.9
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    Text {
                                        visible: !noDevices
                                        text: switchControl2.isOn ? qsTr("Connected") : qsTr("Disconnected")
                                        color: switchControl2.isOn ? "#36D85A" : "#7f8c8d"
                                        font.pixelSize: iconSize * 0.6
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }

                                MouseArea {
                                    id: mouseArea22
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked:{
                                        if(!noDevices && BleManager){
                                            BleManager.connectToDevice(index)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

        }

    }

    */
}
