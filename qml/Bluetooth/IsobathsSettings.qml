import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts  1.12
import QtQuick.Dialogs  1.3
import Qt.labs.settings 1.1
import QtQuick.Window 2.15

import "../"


Item {
    id: isobathsSettings
    objectName: "isobathsSettings"

    width: drawerOpen ? content.width : toggleButton.width
    height: Math.max(content.height, toggleButton.height)
    z: 9999

    property bool drawerOpen: false
    property int isobathSize: theme.screenSize * 0.35
    property int  layoutHeight:  isobathSize * 0.1
    property CheckButton isobathsCheckButton
    property var targetPlot: null
    property int iconSize: isobathSize * 0.05
    property bool outlineMode: false

    property bool isShowBoatTrack: true
    property bool isShowOutline: true
    property bool isShowIsobaths: true
    property bool isShowBoat: true


    onVisibleChanged: if (visible) focus = true

    onFocusChanged: {
        if (Qt.platform.os === "android" && !focus) {
            isobathsCheckButton.isobathsLongPressTriggered = false
        }
    }

    // ================= 侧边按钮 =================
    Rectangle {
        id: toggleButton
        width: 25
        height: 200
        anchors.right: parent.right
        anchors.top: parent.top
        color: "#3498db"
        opacity: 0.75

        MouseArea {
            anchors.fill: parent
            onClicked: drawerOpen = !drawerOpen
        }

        Text {
            anchors.centerIn: parent
            text: qsTr("Isobaths Settings")
            rotation: 90
            color: "white"
            font.pixelSize: 14
        }
    }

    // ================= 抽屉 =================
    Rectangle {
        id: content
        width: isobathSize
        height: isobathSize
        anchors.top: parent.top
        anchors.right: toggleButton.left

        anchors.rightMargin: drawerOpen ? 0 : -(width + toggleButton.width)

        color: "#f0f0f0"
        border.color: "#3498db"
        border.width: 1
        radius: 8

        // 拦截鼠标事件，防止点击穿透到地图
       MouseArea {
           anchors.fill: parent
           enabled: drawerOpen
           preventStealing: true
       }

        Rectangle {
            anchors.fill: parent
            color: "#dbe3f2"
        }

        Behavior on anchors.rightMargin {
            NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 12


            Rectangle {
                id: boatTrack
                Layout.fillWidth: true
                Layout.preferredHeight: layoutHeight
                color: "#f9f9fb"
                border.color: "#b0b3b8"
                border.width: 1

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
                    onExited: parent.color = "#f9f9fb"
                }
            }


            Rectangle {
                id: showOutline
                Layout.fillWidth: true
                Layout.preferredHeight: layoutHeight
                color: "#f9f9fb"
                border.color: "#b0b3b8"
                border.width: 1

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
                        text: qsTr("Outline")
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
                    }

                    onEntered: parent.color = "#d6e6ff"
                    onExited: parent.color = "#f9f9fb"
                }
            }



            Rectangle {
                id: contour
                Layout.fillWidth: true
                Layout.preferredHeight: layoutHeight
                color: "#f9f9fb"
                border.color: "#b0b3b8"
                border.width: 1

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
                        font.pixelSize: 0
                        color: "black"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked: {
                        flashAnim3.restart()
                        isShowIsobaths = !isShowIsobaths
                        IsobathsViewControlMenuController.onProcessStateChanged(isShowIsobaths);
                        IsobathsViewControlMenuController.onIsobathsVisibilityCheckBoxCheckedChanged(isShowIsobaths)
                    }

                    onEntered: parent.color = "#d6e6ff"
                    onExited: parent.color = "#f9f9fb"
                }
            }


            Rectangle {
                id: showBoat
                Layout.fillWidth: true
                Layout.preferredHeight: layoutHeight
                color: "#f9f9fb"
                border.color: "#b0b3b8"
                border.width: 1

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
                        text: qsTr("Show Boat")
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
                    }

                    onEntered: parent.color = "#d6e6ff"
                    onExited: parent.color = "#f9f9fb"
                }
            }


            // ------------------- 分隔线 -----------------
            Rectangle {
                height: 2
                color: "#9c9c9c"
                Layout.fillWidth: true
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
                    width:  iconSize * 3.5
                    height: iconSize * 1.3
                    radius: iconSize * 0.35
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
                            } else {
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
                    id: edgeLimit
                    implicitWidth: isobathSize * 0.4
                    from: 10; to: 1000; stepSize: 5
                    value: 100
                    enabled: !renderSpanControl.isOn

                    onValueChanged:
                        IsobathsViewControlMenuController.onEdgeLimitChanged(value)
                }
            }


            // ================= Contour Interval =================
            RowLayout {
                Text {
                    text: qsTr("Contour Interval")
                    font.pixelSize: iconSize
                    Layout.fillWidth: true
                }

                SpinBoxCustom {
                    id: contourStep
                    implicitWidth: isobathSize * 0.4
                    from: 1; to: 200; stepSize: 1
                    value: 10

                    property real realValue: value / 10

                    onRealValueChanged:
                        IsobathsViewControlMenuController.onSetSurfaceLineStepSize(realValue)
                }
            }

            // ================= 分隔线 =================
            Rectangle {
                height: 2
                color: "#9c9c9c"
                Layout.fillWidth: true
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
}
