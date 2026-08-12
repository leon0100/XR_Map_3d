import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts  1.12

import "../"
import AppXr 1.0

// ----------------- Bathymetry Config抽屉面板------------------
XRRectangle {
    id: bathymetry
    width:  bathymetrySize * 1.6
    height: iconSize * 12

    color: "#dbe3f2"
    border.color: "#d8e0ef"
    border.width: 1
    radius: iconSize * 0.25


    property var  targetPlot:    null
    property int  bathymetrySize:  theme.screenSize * 0.35
    property int  iconSize:      bathymetrySize * 0.05

    // 拦截鼠标事件，防止点击穿透到地图
    // MouseArea {
    //     anchors.fill: parent
    //     enabled: bluetoothDrawOpen
    //     preventStealing: true
    // }

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
        spacing: iconSize * 0.2

        RowLayout {
            Layout.fillWidth: true
            spacing: iconSize * 0.2

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

            XRButton{
                id: applyBtn
                width: iconSize * 3
                height: iconSize * 1.2
                buttonText: qsTr("Apply")
                recTextSize: iconSize
                checkable: false

                clickAction: function() {
                    let soundVelocity = parseInt(soundSpeedField.text)
                    let draftOffset   = parseInt(draftOffsetField.text)
                    if(targetPlot) {
                        targetPlot.setSoundVelocity(soundVelocity, draftOffset)
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
            spacing: iconSize * 0.3

            CheckBox {
                id: depthFilterCheck
                font.pixelSize: iconSize
                rightPadding: 0
                checked: false
                onCheckedChanged: targetPlot.setDepthFilterVisible(checked, depthFilterXRSlider.value)
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
                    if(targetPlot) {
                        targetPlot.setDepthFilterVisible(depthFilterCheck.checked, depthFilterXRSlider.value)
                    }
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
                unit: " cm"
                onValueChanged: {
                    targetPlot.setKeelOffsetValue(keelOffset.value)
                }
            }

        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#555555"
        }

        //第三行
        RowLayout {
            Layout.fillWidth: true
            spacing: iconSize * 0.3

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
                    targetPlot.batchCorrect = checked
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
