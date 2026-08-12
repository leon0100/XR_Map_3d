import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts  1.12

import "../"
import AppXr 1.0



// ---------------------------- isobaths抽屉 ----------------------------
XRRectangle {
    id: isobathsContent
    width:  isobathSize
    height: isobathSize * 1.25

    color: "#dbe3f2"
    border.color: "#d8e0ef"
    border.width: 1
    radius: iconSize * 0.3


    property int  isobathSize:  theme.screenSize * 0.35
    property int  layoutHeight: isobathSize * 0.1
    property var  targetPlot:   null

    property int  iconSize:     isobathSize * 0.05

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

    // 拦截鼠标事件，防止点击穿透到地图
    // MouseArea {
    //     anchors.fill: parent
    //     enabled: isobathsDrawOpen
    //     preventStealing: true
    // }

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

            XRButton {
                id: outlineButton
                buttonText: outlineMode ? qsTr("Clear Outline") : qsTr("Draw Outline")
                checkable: true
                implicitWidth: isobathSize * 0.4
                Layout.preferredHeight: iconSize * 1.5

                clickAction: function() {
                    var trackCnt = core.poolSize();
                    if(trackCnt < 3) {
                        checked = false
                        GetInterface.showDialogInfo(0, qsTr("No Track Data Found!"))
                        return
                    }
                    outlineMode = !outlineMode
                    if(targetPlot) {
                        targetPlot.drawPolygonOutline(outlineMode)
                    }
                }
            }

            XRButton {
                id: updateBottomTrackButton
                buttonText: qsTr("Draw Isobaths")
                checkable: false
                implicitWidth:  isobathSize * 0.4
                Layout.preferredHeight: iconSize * 1.5

                clickAction: function() {
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
            radius: iconSize * 0.3

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
                            width:  iconSize * 1.2
                            height: iconSize * 1.2
                            radius: 5
                            border.color: "#b0b3b8"
                            border.width: 1
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                source: "qrc:/XR/check.svg"
                                width: parent.width * 0.9
                                height: parent.width * 0.9
                                visible: isShowBoatTrack
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.rightMargin: 2
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
                            width:  iconSize * 1.2
                            height: iconSize * 1.2
                            radius: 5
                            border.color: "#b0b3b8"
                            border.width: 1
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                source: "qrc:/XR/check.svg"
                                width: parent.width * 0.9
                                height: parent.width * 0.9
                                visible: isShowOutline
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.rightMargin: 2
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
                            width:  iconSize * 1.2
                            height: iconSize * 1.2
                            radius: 5
                            border.color: "#b0b3b8"
                            border.width: 1
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                source: "qrc:/XR/check.svg"
                                width: parent.width * 0.9
                                height: parent.width * 0.9
                                visible: isContours
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.rightMargin: 2
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
                            width:  iconSize * 1.2
                            height: iconSize * 1.2
                            radius: 5
                            border.color: "#b0b3b8"
                            border.width: 1
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                source: "qrc:/XR/check.svg"
                                width: parent.width * 0.9
                                height: parent.width * 0.9
                                visible: isShowIsobaths
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.rightMargin: 2
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
                            width:  iconSize * 1.2
                            height: iconSize * 1.2
                            radius: 5
                            border.color: "#b0b3b8"
                            border.width: 1
                            anchors.verticalCenter: parent.verticalCenter

                            Image {
                                source: "qrc:/XR/check.svg"
                                width: parent.width * 0.9
                                height: parent.width * 0.9
                                visible: isShowBoat
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.rightMargin: 2
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
                width:  iconSize * 3.2
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
                    font.pixelSize: iconSize * 0.7
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

            /*----备选----*/
            // Rectangle {
            //     id: renderSpanControl
            //     width:  iconSize * 1.2
            //     height: iconSize * 1.2
            //     radius: 5
            //     border.color: "#b0b3b8"
            //     border.width: 1

            //     property bool isOn: true

            //     Image {
            //         source: "qrc:/XR/check.svg"
            //         width: parent.width * 0.9
            //         height: parent.width * 0.9
            //         visible: renderSpanControl.isOn
            //         anchors.verticalCenter: parent.verticalCenter
            //         anchors.horizontalCenter: parent.horizontalCenter
            //         anchors.rightMargin: 2
            //     }

            //     MouseArea {
            //         anchors.fill: parent
            //         hoverEnabled: true
            //         onPressed: {
            //             if(renderSpanControl.isOn) {
            //                 renderSpanControl.isOn = false;
            //                 core.setAutoRenderSpan(false);
            //             }
            //             else {
            //                 renderSpanControl.isOn = true
            //                 core.setAutoRenderSpan(true);
            //             }
            //         }
            //     }
            // }


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

                // Settings {
                //     property alias isobathsSurfaceLineStepSizeSpinBox: contourStep.value
                // }
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

