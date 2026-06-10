import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3
import Qt.labs.settings 1.1

import AppXr 1.0


//地图下侧的工具栏
Item  {
    id: toolbarRoot

    width:  rowButtons.implicitWidth
    height: rowButtons.implicitHeight

    property bool isNorthModeCheckButtonHovered:   false

    property bool toolbarHovered: Qt.platform.os === "android" ?
                northModeCheckButton.down : isNorthModeCheckButtonHovered


    Behavior on opacity { NumberAnimation { duration: 120 } }


    property string  distance: "0m"
    Connections {
        target: renderer.screetShot

        function onSignalStartToEndDist(dist) {
            if (dist >= 1000) {
                distance = (dist / 1000).toFixed(2) + " km"
            }
            else {
                distance = dist.toFixed(0) + " m"
            }
        }
    }


    RowLayout {
        id: rowButtons
        spacing: 8
        Layout.alignment: Qt.AlignHCenter

        Item {
            id: scaleBar
            Layout.alignment: Qt.AlignVCenter
            width: 110
            height: 30

            Column {
                anchors.centerIn: parent
                spacing: 2

                Text {
                  anchors.horizontalCenter: parent.horizontalCenter
                  text: distance
                  color: "white"
                  font.pixelSize: theme.iconSize
                }

                Canvas {
                   width: 100
                   height: 12

                   onPaint: {
                      var ctx = getContext("2d");
                      ctx.clearRect(0, 0, width, height);
                      ctx.strokeStyle = "white";
                      ctx.lineWidth = 4;

                      ctx.beginPath();
                      ctx.moveTo(0, height * 0.6);
                      ctx.lineTo(width, height * 0.6);
                      ctx.moveTo(0, 0);
                      ctx.lineTo(0, height * 0.6);
                      ctx.moveTo(width, 0);
                      ctx.lineTo(width, height * 0.6);
                      ctx.stroke();
                   }
                }
            }
        }


        Button {
            id: zoomInBtn
            implicitHeight:  theme.menuWidth
            implicitWidth:   theme.menuWidth

            CMouseOpacityArea {
                toolTipText: qsTr("Zoom In")
                popupPosition: "topRight"
            }

            contentItem: Item {
                Image {
                    source: "qrc:/XR/zoom_in.png"
                    anchors.centerIn: parent
                    width:  theme.menuWidth * 0.85
                    height: theme.menuWidth * 0.85
                }
            }

            background: Rectangle {
                radius: 3
                color: zoomInBtn.down ? "white" : theme.controlBackColor
            }

            onClicked: {
                renderer.zoomInOut(true)
            }

        }


        Button {
            id: zoomOutBtn
            implicitHeight: theme.menuWidth
            implicitWidth:  theme.menuWidth

            CMouseOpacityArea {
                toolTipText: qsTr("Zoom Out")
                popupPosition: "topRight"
            }

            contentItem: Item {
                Image {
                    source: "qrc:/XR/zoom_out.png"
                    anchors.centerIn: parent
                    width:  theme.menuWidth * 0.85
                    height: theme.menuWidth * 0.85
                }
            }

            background: Rectangle {
                radius: 3
                color: zoomOutBtn.down ? "white" : theme.controlBackColor
            }

            onClicked: {
                renderer.zoomInOut(false)
            }

        }


        CheckButton {
            id: northModeCheckButton
            iconSource: "qrc:/icons/ui/location_pin.svg"
            backColor:  theme.controlBackColor
            borderColor: theme.controlBackColor
            checkedBorderColor: theme.controlBorderColor
            checked:    true
            implicitHeight: theme.menuWidth
            implicitWidth: theme.menuWidth
            opacity: toolbarHovered ? 1.0 : 0.5

            CMouseOpacityArea {
                toolTipText: qsTr("Lock 2D")
                popupPosition: "topRight"
            }

            onCheckedChanged: {
                Scene3dToolBarController.onIsNorthLocationButtonChanged(checked)
            }

            Component.onCompleted: {
                Scene3dToolBarController.onIsNorthLocationButtonChanged(checked)
            }

            Settings {
                property alias northModeCheckButton: northModeCheckButton.checked
            }
        }



        // CheckButton {
        //     id: mapCheckButton
        //     iconSource: "qrc:/icons/ui/map.svg"
        //     backColor:          theme.controlBackColor
        //     borderColor:        theme.controlBackColor
        //     checkedBorderColor: theme.controlBorderColor
        //     checked:            true
        //     implicitHeight:     theme.menuWidth * 0.8
        //     implicitWidth:      theme.menuWidth * 0.8

        //     hoverEnabled: true
        //     onHoveredChanged: {
        //         toolbarRoot.isMapCheckButtonHovered = hovered
        //         mapCheckButton.opacity = 1.0;
        //     }

        //     CMouseOpacityArea {
        //         toolTipText: qsTr("Map Visibility")
        //         popupPosition: "topRight"
        //     }

        //     onCheckedChanged: MapViewControlMenuController.onVisibilityChanged(checked)

        //     Component.onCompleted: MapViewControlMenuController.onVisibilityChanged(checked)

        // }

    }

}


