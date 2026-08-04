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
                    source: "qrc:/icons/ui/file_settings.svg"
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

                    onValueChanged: {
                        IsobathsViewControlMenuController.onVerticalScaleSliderValueChanged(value)
                    }

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



    // property bool isShowDataPanel: false

    // onVisibleChanged: {
    //     BleManager.setBleLiveScanningVisible(visible)
    // }

    // Connections {
    //     target: BleManager
    //     function onConnectedChanged(connected) {
    //         switchControl.isOn = connected
    //         readControl.isReading  = connected
    //         root.isShowDataPanel = true
    //         // root.visible = false
    //     }
    // }






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
                    font.bold: true
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
                    onCheckedChanged: core.setDepthFilterVisible(checked, depthFilterXRSlider.value)
                }
                XRSlider {
                    id: depthFilterXRSlider
                    title: qsTr("Depth Filter")
                    Layout.preferredWidth: iconSize * 6
                    Layout.alignment: Qt.AlignVCenter
                    fontSize: iconSize
                    spacing:  iconSize * 0.3
                    handleWidth: iconSize * 0.4
                    sliderLen: iconSize * 5
                    from: 1
                    to: 4
                    value: 1
                    onValueChanged: {
                        core.setDepthFilterVisible(depthFilterCheck.checked, depthFilterXRSlider.value)
                    }
                }

                Rectangle {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 2
                    color: "#888888"
                }

                XRSlider {
                    id: keelOffset
                    // title: "Outline Correct"
                    title: qsTr("Keel Offset")
                    Layout.preferredWidth: iconSize * 8
                    Layout.alignment: Qt.AlignVCenter
                    fontSize: iconSize
                    spacing:  iconSize * 0.1
                    handleWidth: iconSize * 0.4
                    sliderLen: iconSize * 7
                    from: -100
                    to: 100
                    value: 0
                    unit: "cm"
                    onValueChanged: {
                        core.setKeelOffsetValue(keelOffset.value)
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
                    text: qsTr("Attitude Correction")
                    font.pixelSize: iconSize
                    checked: false
                    enabled: false
                    opacity: 0.7
                    onCheckedChanged: {
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

                // Rectangle {
                //     Layout.fillHeight: true
                //     Layout.preferredWidth: 2
                //     color: "#888888"
                // }

                // CheckBox {
                //     id: depthCorrectionCheck
                //     text: qsTr("Depth Correct")
                //     font.pixelSize: iconSize
                //     checked: false
                //     onCheckedChanged: {
                //         core.depthCorrect = checked
                //     }
                // }

            }

        }

    }

}
