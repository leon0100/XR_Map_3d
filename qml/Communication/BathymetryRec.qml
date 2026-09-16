import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts  1.12

import "../"
import AppXr 1.0

// ----------------- Bathymetry Config抽屉面板------------------
XRRectangle {
    id: bathymetry
    width:  iconSize * 24
    height: iconSize * 11

    color: "#dbe3f2"
    border.color: "#d8e0ef"
    border.width: 1
    radius: iconSize * 0.25


    property var  targetPlot: null
    property int  iconSize:   theme.iconSize * 1.4


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
            font.bold: true
        }
    }


    ColumnLayout {
        anchors.top: bathymetryConfigTitle.bottom
        anchors.topMargin: iconSize * 0.5
        anchors.left: parent.left
        anchors.leftMargin: iconSize * 0.2
        spacing: iconSize * 0.5

        RowLayout {
            Layout.fillWidth: true
            spacing: iconSize * 0.2

            Text {
                text: qsTr("Sound Velocity")
                font.pixelSize: iconSize
                verticalAlignment: Text.AlignVCenter
                // width: 200 //布局中，width不起效果xxx
                // implicitWidth: 200  //Text在布局中implicitWidth仅读不能赋值xxx
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

            XRButton {
                id: applyBtn
                width: iconSize * 3.3
                height: iconSize * 1.4
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
            Layout.preferredHeight: 2
            color: "#555555"
        }


        // 第二行:
        RowLayout {
            Layout.fillWidth: true
            spacing: iconSize * 0.3

            CCheck {
                id: depthFilterCheck
                checked: false
                rightPadding: 0
                onCheckedChanged: targetPlot.setDepthFilterVisible(checked, depthFilterXRSlider.value)
            }
            XRSlider {
                id: depthFilterXRSlider
                title: qsTr("Depth Filter")
                Layout.preferredWidth: iconSize * 6
                Layout.alignment: Qt.AlignVCenter
                fontSize: iconSize
                spacing:  iconSize * 0.3
                sliderLen: iconSize * 8
                from: 1
                to: 4
                value: 1
                onValueChanged: {
                    if(targetPlot) {
                        targetPlot.setDepthFilterVisible(depthFilterCheck.checked, depthFilterXRSlider.value)
                    }
                }
            }

        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 2
            color: "#555555"
        }

        // 第三行:
        RowLayout {
            Layout.fillWidth: true
            spacing: iconSize * 0.3

            XRSlider {
                id: keelOffset
                // title: "Outline Correct"
                title: qsTr("Keel Offset")
                Layout.preferredWidth: iconSize * 8
                Layout.alignment: Qt.AlignVCenter
                fontSize: iconSize
                spacing:  iconSize * 0.1
                sliderLen: iconSize * 9
                textLen: iconSize * 3
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
            Layout.preferredHeight: 2
            color: "#555555"
        }

        //第四行
        RowLayout {
            Layout.fillWidth: true
            spacing: iconSize * 0.3

            CCheck {
                id: attitudeCorrectCheck
                text: qsTr("Attitude Correction")
                font.pixelSize: iconSize
                checked: false
                enabled: false
                opacity: 0.7
                onCheckedChanged: {

                }
            }

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 2
                color: "#888888"
            }

            CCheck {
                id: batchCorrectionCheck
                text: qsTr("Batch Correct")
                font.pixelSize: iconSize
                checked: false
                onCheckedChanged: {
                    targetPlot.batchCorrect = checked
                }
            }

        }

    }

}
