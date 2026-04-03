import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3
import Qt.labs.settings 1.1



// isobaths extra settings
MenuFrame {
    id: isobathsSettings
    width:  isobathSize * 1.28
    height: isobathSize * 0.6
    z: 9999


    property int isobathSize: theme.screenSize * 0.35
    property CheckButton isobathsCheckButton
    property var targetPlot: null
    property int iconSize: isobathSize * 0.06

    property bool outlineMode: false

    onIsHoveredChanged: {
        if (Qt.platform.os === "android") {
            if (isHovered) {
                isHovered = false
            }
        }
    }

    onVisibleChanged: {
        if (visible) {
            focus = true;
        }
    }

    onFocusChanged: {
        if (Qt.platform.os === "android" && !focus) {
            isobathsCheckButton.isobathsLongPressTriggered = false
        }
    }


    ColumnLayout {
        spacing: 16

        RowLayout {
            spacing: 16

            Rectangle {
                Layout.fillWidth: true
                height: 2
                color: "#808080"
            }

            Text {
                text: qsTr("Isobaths Settings")
                color: "black"
                font.pixelSize: iconSize * 0.9
            }

            Rectangle {
                Layout.fillWidth: true
                height: 2
                color: "#808080"
            }
        }


        RowLayout {
            spacing: 32

            Button {
                id: outlineButton
                text: qsTr("Draw Outline")
                font.pixelSize: iconSize * 1.1
                implicitWidth:  isobathSize * 0.5
                // Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: iconSize * 1.1
                palette.button: outlineMode ? "#66E07A" : "#b9c6db"

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
                font.pixelSize: iconSize * 1.1
                implicitWidth:  isobathSize * 0.5
                // Layout.alignment: Qt.AlignCenter
                Layout.preferredHeight: iconSize * 1.1
                palette.button: "#b9c6db"

                onClicked: {
                    if (targetPlot) {
                            targetPlot.doDistProcessing(
                                0,     // preset
                                1,     // window_size
                                0,     // vertical_gap
                                0,     // range_min
                                1000,  // range_max
                                1,     // gain_slope
                                0,     // threshold
                                0,     // offsetx
                                0,     // offsety
                                0,     // offsetz
                                false  // manual
                            )
                    }
                }
            }

        }

        // RowLayout {
        //     CText {
        //         text: qsTr("Theme:")
        //     }
        //     Item {
        //         Layout.fillWidth: true
        //     }
        //     CCombo  {
        //         id: isobathsTheme
        //         Layout.preferredWidth: 200
        //         model: [qsTr("Standard"), qsTr("Midnight"), qsTr("Default"), qsTr("Blue"), qsTr("Sepia"), qsTr("WRGBD"), qsTr("WhiteBlack")]
        //         currentIndex: 0
        //         onCurrentIndexChanged: {
        //             IsobathsViewControlMenuController.onThemeChanged(currentIndex)
        //         }

        //         onFocusChanged: {
        //             if (Qt.platform.os === 'android') {
        //                 isobathsSettings.focus = true
        //             }
        //         }

        //         Component.onCompleted: {
        //             IsobathsViewControlMenuController.onThemeChanged(currentIndex)
        //         }

        //         Settings {
        //             property alias isobathsTheme: isobathsTheme.currentIndex
        //         }
        //     }
        // }


        RowLayout {
            spacing: 10

            Text {
                // text: qsTr("Edge limit(m):")
                color: "black"
                text: qsTr("Render Span") //定义了Delaunay三角剖分中的三角形边长的最大允许值
                font.pixelSize: iconSize
                Layout.fillWidth: true
            }

            Rectangle {
                id: renderSpanControl
                width:  iconSize * 3.2
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
                id: isobathsEdgeLimitSpinBox
                implicitWidth: isobathSize * 0.5
                from: 10
                to: 1000
                stepSize: 5
                value: 100
                editable: false
                Layout.rightMargin: 10
                enabled: !renderSpanControl.isOn

                property int decimals: 1

                onFocusChanged: isobathsSettings.focus = true

                Component.onCompleted: {
                    IsobathsViewControlMenuController.onEdgeLimitChanged(isobathsEdgeLimitSpinBox.value)
                }

                onValueChanged: {
                    IsobathsViewControlMenuController.onEdgeLimitChanged(isobathsEdgeLimitSpinBox.value)
                }

                Connections {
                    target: IsobathsViewControlMenuController
                    function onEdgeLimitChanged(val) {
                        isobathsEdgeLimitSpinBox.value = val
                    }
                }
            }

        }


        RowLayout {
            Text {
                text: qsTr("Contour Interval") //控制等值线的步长，即相邻两条等值线之间的高度差。该参数直接影响等值线的密度和详细程度
                font.pixelSize: iconSize
                Layout.fillWidth: true
                color: "black"
            }
            SpinBoxCustom {
                id: isobathsSurfaceLineStepSizeSpinBox
                implicitWidth: isobathSize * 0.5
                from: 1
                to: 200
                stepSize: 1
                value: 10
                editable: false
                Layout.rightMargin: 10

                property int decimals: 1
                property real realValue: value / 10

                validator: DoubleValidator {
                    bottom: Math.min(isobathsSurfaceLineStepSizeSpinBox.from, isobathsSurfaceLineStepSizeSpinBox.to)
                    top:  Math.max(isobathsSurfaceLineStepSizeSpinBox.from, isobathsSurfaceLineStepSizeSpinBox.to)
                }

                textFromValue: function(value, locale) {
                    return Number(value / 10).toLocaleString(locale, 'f', decimals)
                }

                valueFromText: function(text, locale) {
                    return Number.fromLocaleString(locale, text) * 10
                }

                onFocusChanged:  isobathsSettings.focus = true

                Component.onCompleted: {
                    IsobathsViewControlMenuController.onSetSurfaceLineStepSize(isobathsSurfaceLineStepSizeSpinBox.realValue)
                }

                onRealValueChanged: {
                    IsobathsViewControlMenuController.onSetSurfaceLineStepSize(isobathsSurfaceLineStepSizeSpinBox.realValue)
                }

                Settings {
                    property alias isobathsSurfaceLineStepSizeSpinBox: isobathsSurfaceLineStepSizeSpinBox.value
                }
            }
        }


        // RowLayout {
        //     Text {
        //         text: qsTr("Extra Width:")
        //         font.pixelSize: iconSize
        //         Layout.fillWidth: true
        //         color: "black"
        //     }
        //     Item {
        //         Layout.fillWidth: true
        //     }
        //     SpinBoxCustom {
        //         id: extraWidthSpinBox
        //         implicitWidth: isobathSize * 0.5
        //         from: 5
        //         to: 100
        //         stepSize: 5
        //         value: 5
        //         editable: false

        //         onFocusChanged: {
        //             isobathsSettings.focus = true
        //         }

        //         onValueChanged: {
        //             IsobathsViewControlMenuController.onSetExtraWidth(value)
        //         }

        //         Component.onCompleted: {
        //             IsobathsViewControlMenuController.onSetExtraWidth(value)
        //         }

        //         Settings {
        //             property alias extraWidthSpinBox: extraWidthSpinBox.value
        //         }
        //     }
        // }

        Rectangle {
            width: parent.width
            height: 2
            color: "#9c9c9c"
            Layout.fillWidth: true
            Layout.topMargin: 5
            Layout.bottomMargin: 5
        }

        RowLayout {
            spacing: 20
            Text {
                text: qsTr("Vertical Scale")
                color: "black"
                Layout.fillWidth: true
                font.pixelSize: iconSize
            }

            Slider {
                id: verticalScaleSlider
                Layout.fillWidth: true
                Layout.preferredWidth: isobathSize * 0.8
                from: 0.5
                to:   10.0
                value: IsobathsViewControlMenuController.verticalScale()
                stepSize: 0.1

                background: Rectangle {
                    x: verticalScaleSlider.leftPadding
                    y: verticalScaleSlider.topPadding + verticalScaleSlider.availableHeight/2 - height/2
                    implicitWidth: isobathSize * 0.8
                    implicitHeight: iconSize * 0.5
                    width: verticalScaleSlider.availableWidth
                    height: isobathSize * 0.02
                    radius: 5
                    color: "#828282"
                }

                handle: Rectangle {
                    x: verticalScaleSlider.leftPadding + verticalScaleSlider.visualPosition
                       * (verticalScaleSlider.availableWidth - width)
                    y: verticalScaleSlider.topPadding + verticalScaleSlider.availableHeight/2 - height/2
                    implicitWidth: iconSize
                    implicitHeight: iconSize
                    radius: iconSize * 0.7
                    color: verticalScaleSlider.pressed ? "#f0f0f0" : "#f6f6f6"
                }

                onValueChanged: {
                    IsobathsViewControlMenuController.onVerticalScaleSliderValueChanged(value)
                }
            }
        }

        // RowLayout {
        //     CTextField {
        //         id: exportSurfacePathText
        //         hoverEnabled: true
        //         Layout.maximumWidth: 200
        //         Layout.fillWidth: true
        //         placeholderText: qsTr("Enter path")
        //     }

        //     CButton {
        //         text: "..."
        //         Layout.fillWidth: false

        //         onClicked: {
        //             exportSurfaceFileDialog.open()
        //         }
        //     }

        //     FileDialog  {
        //         id: exportSurfaceFileDialog
        //         title: qsTr("Select folder and set .csv file name")

        //         // currentFolder: StandardPaths.writableLocation(StandardPaths.HomeLocation)

        //         // fileMode: FileDialog.SaveFile

        //         nameFilters: ["CSV Files (*.csv)", "All Files (*)"]
        //         defaultSuffix: "csv"

        //         onAccepted: {
        //             var url = selectedFile.toString()
        //             if (!url.toLowerCase().endsWith(".csv")) {
        //                 url += ".csv"
        //             }

        //             exportSurfacePathText.text = url
        //         }
        //     }

        //     CButton {
        //         text: qsTr("Export to CSV")
        //         Layout.fillWidth: true
        //         onClicked: Scene3DControlMenuController.onExportToCSVButtonClicked(exportSurfacePathText.text)
        //     }

        //     // Settings {
        //     //     property alias exportSurfaceFolder: exportSurfaceFileDialog.currentFolder
        //     // }

        //     Settings {
        //         property alias exportSurfaceFolderText: exportSurfacePathText.text
        //     }
        // }

        // RowLayout {
        //     visible: !isobathsDebugModeCheckButton.checked

        //     CText {
        //         text: qsTr("Label step, m:")
        //         Layout.fillWidth: true
        //     }
        //     SpinBoxCustom {
        //         id: isobathsLabelStepSpinBox
        //         implicitWidth: 200
        //         from: 10
        //         to: 1000
        //         stepSize: 5
        //         value: 100
        //         editable: false

        //         property int decimals: 1

        //         onFocusChanged: {
        //             isobathsSettings.focus = true
        //         }

        //         Component.onCompleted: {
        //             IsobathsViewControlMenuController.onSetLabelStepSize(isobathsLabelStepSpinBox.value)
        //         }

        //         onValueChanged: {
        //             IsobathsViewControlMenuController.onSetLabelStepSize(isobathsLabelStepSpinBox.value)
        //         }

        //         Settings {
        //             property alias isobathsLabelStepSpinBox: isobathsLabelStepSpinBox.value
        //         }
        //     }
        // }

        // CheckButton {
        //     text: qsTr("Triangles")
        //     Layout.fillWidth: true
        //     checked: true
        //     visible: isobathsDebugModeCheckButton.checked

        //     onCheckedChanged: {
        //         IsobathsViewControlMenuController.onTrianglesVisible(checked);
        //     }

        //     onFocusChanged: {
        //         isobathsSettings.focus = true
        //     }
        // }

        // CheckButton {
        //     text: qsTr("Edges")
        //     Layout.fillWidth: true
        //     checked: true
        //     visible: isobathsDebugModeCheckButton.checked

        //     onCheckedChanged: {
        //         IsobathsViewControlMenuController.onEdgesVisible(checked);
        //     }

        //     onFocusChanged: {
        //         isobathsSettings.focus = true
        //     }
        // }

        // CheckButton {
        //     id: isobathsDebugModeCheckButton
        //     text: qsTr("Debug mode")
        //     Layout.fillWidth: true
        //     checked: false

        //     onCheckedChanged: {
        //         IsobathsViewControlMenuController.onDebugModeView(checked);
        //     }

        //     onFocusChanged: {
        //         isobathsSettings.focus = true
        //     }
        // }

        // CButton {
        //     id: resetIsobathsButton
        //     text: qsTr("Clear")
        //     Layout.fillWidth: true
        //     onClicked: {
        //         IsobathsViewControlMenuController.onResetIsobathsButtonClicked()
        //     }

        //     onFocusChanged: {
        //         isobathsSettings.focus = true
        //     }
        // }

        // CButton {
        //     id: updateIsobathsButton
        //     text: qsTr("Update")
        //     Layout.fillWidth: true

        //     onClicked: {
        //         //IsobathsViewControlMenuController.onResetIsobathsButtonClicked()
        //         IsobathsViewControlMenuController.onUpdateIsobathsButtonClicked()
        //     }

        //     onFocusChanged: {
        //         isobathsSettings.focus = true
        //     }
        // }
    }
}
