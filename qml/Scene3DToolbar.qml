import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3
import Qt.labs.settings 1.1



//下方中间位置的工具栏
Item  {
    id: toolbarRoot
    anchors.left: parent.left
    anchors.leftMargin: 8

    width:  rowButtons.implicitWidth
    height: rowButtons.implicitHeight

    signal updateBottomTrack()

    // opacity
    property bool isFitViewCheckButtonHovered:     false
    property bool isBoatTrackCheckButtonHovered:   false
    property bool isBottomTrackCheckButtonHovered: false

    property bool toolbarHovered: Qt.platform.os === "android" ?
                (setCameraIsometricView.down|| boatTrackCheckButton.down) :
                (isBoatTrackCheckButtonHovered || isFitViewCheckButtonHovered)

    property bool menuOpened: settings3DSettings.visible || locationSettings.visible

    opacity: (toolbarHovered || menuOpened) ? 1.0 : 0.5
    Behavior on opacity { NumberAnimation { duration: 120 } }


    // buttons
    ColumnLayout {
        id: rowButtons
        spacing: 10
        Layout.alignment: Qt.AlignHCenter

        // CheckButton {
        //     id: setCameraIsometricView
        //     iconSource: "./fit-in-view.svg"
        //     backColor: theme.controlBackColor
        //     borderColor: theme.controlBackColor
        //     checkedBorderColor: theme.controlBorderColor
        //     checkable: false
        //     checked: false
        //     implicitHeight: theme.menuWidth
        //     implicitWidth: theme.menuWidth

        //     CMouseOpacityArea {
        //         toolTipText: qsTr("Reset camera")
        //         popupPosition: "topRight"
        //     }

        //     hoverEnabled: true
        //     onHoveredChanged: {
        //         toolbarRoot.isFitViewCheckButtonHovered = hovered
        //     }

        //     onClicked: {
        //         Scene3dToolBarController.onSetCameraMapViewButtonClicked()
        //     }
        // }


        // Item {
        //     //visible: false
        //     id:     locationWrapper
        //     width : locationCheckButton.implicitWidth
        //     height: locationCheckButton.implicitHeight

        //     CheckButton {
        //         id: locationCheckButton
        //         iconSource: "qrc:/icons/ui/location.svg"
        //         backColor:          theme.controlBackColor
        //         borderColor:        theme.controlBackColor
        //         checkedBorderColor: theme.controlBorderColor
        //         checked:            false
        //         implicitHeight:     theme.menuWidth
        //         implicitWidth:      theme.menuWidth

        //         onCheckedChanged: {
        //             Scene3dToolBarController.onTrackLastDataCheckButtonCheckedChanged(checked)
        //         }

        //         Component.onCompleted: {
        //             Scene3dToolBarController.onTrackLastDataCheckButtonCheckedChanged(checked)
        //         }

        //         property bool locationLongPressTriggered: false

        //         MouseArea {
        //             id: locationTouchArea
        //             anchors.fill: parent
        //             enabled: Qt.platform.os === "android"

        //             onPressed: {
        //                 if (enabled) {
        //                     locationLongPressTimer.start()
        //                     locationCheckButton.locationLongPressTriggered = false
        //                 }
        //             }

        //             onReleased: {
        //                 if (enabled) {
        //                     if (!locationCheckButton.locationLongPressTriggered) {
        //                         locationCheckButton.checked = !locationCheckButton.checked
        //                     }
        //                     locationLongPressTimer.stop()
        //                 }
        //             }

        //             onCanceled: {
        //                 if (enabled) {
        //                     locationLongPressTimer.stop()
        //                 }
        //             }
        //         }

        //         Timer {
        //             id: locationLongPressTimer
        //             interval: 100 // ms
        //             repeat: false
        //             running : false
        //             onTriggered: {
        //                 locationCheckButton.locationLongPressTriggered = true;
        //             }
        //         }

        //         Settings {
        //             property alias locationCheckButton: locationCheckButton.checked
        //         }
        //     }

        //     LocationExtraSettings {
        //         id: locationSettings
        //         locationCheckButton:      locationCheckButton
        //         anchors.bottom:           locationCheckButton.top
        //         anchors.horizontalCenter: locationCheckButton.horizontalCenter
        //         z: 2
        //     }
        // }


        //Settings3DExtraSettings.qml
        Item {
            id: settings3DWrapper
            width : settings3DCheckButton.implicitWidth
            height: settings3DCheckButton.implicitHeight

            CheckButton {
                id: settings3DCheckButton
                iconSource: "qrc:/icons/ui/settings.svg"
                backColor:   theme.controlBackColor
                borderColor: theme.controlBackColor
                checkedBorderColor: theme.controlBorderColor
                checkable: false
                implicitHeight: theme.menuWidth
                implicitWidth:  theme.menuWidth

                property bool settingsPressTriggered: false

                onClicked: settings3DCheckButton.settingsPressTriggered =
                             !settings3DCheckButton.settingsPressTriggered
            }

            Settings3DExtraSettings {
                id: settings3DSettings
                settings3DCheckButton:  settings3DCheckButton
                anchors.left:           settings3DCheckButton.right
                anchors.verticalCenter: settings3DCheckButton.verticalCenter
                z: 2
            }
        }



        CheckButton {
            id: boatTrackCheckButton
            iconSource: "qrc:/icons/ui/route.svg"
            backColor: theme.controlBackColor
            borderColor: theme.controlBackColor
            checkedBorderColor: theme.controlBorderColor
            checked: true
            implicitHeight: theme.menuWidth
            implicitWidth: theme.menuWidth

            hoverEnabled: true
            onHoveredChanged: {
                toolbarRoot.isBoatTrackCheckButtonHovered = hovered
            }

            CMouseOpacityArea {
                toolTipText: qsTr("Boat track")
                popupPosition: "topRight"
            }

            onCheckedChanged: {
                BoatTrackControlMenuController.onVisibilityCheckBoxCheckedChanged(checked)
            }

            Component.onCompleted: {
                BoatTrackControlMenuController.onVisibilityCheckBoxCheckedChanged(checked)
            }

            Settings {
                property alias boatTrackCheckButton: boatTrackCheckButton.checked
            }
        }



        // CheckButton {
        //     id:                  bottomTrackCheckButton
        //     iconSource:          "qrc:/icons/ui/double_route.svg"
        //     backColor:           theme.controlBackColor
        //     borderColor:         theme.controlBackColor
        //     checkedBorderColor:  theme.controlBorderColor
        //     checked:             false
        //     implicitHeight:      theme.menuWidth
        //     implicitWidth:       theme.menuWidth

        //     hoverEnabled:        true

        //     property bool pulse: core.dataProcessorState === 1

        //     CMouseOpacityArea {
        //         toolTipText: qsTr("Bottom track")
        //         popupPosition: "topRight"
        //     }

        //     SequentialAnimation {
        //         id: pulseBottomTrackAnimation
        //         running: bottomTrackCheckButton.pulse
        //         loops: Animation.Infinite
        //         NumberAnimation { target: bottomTrackCheckButton; property: "opacity"; to: 0.2; duration: 500 }
        //         NumberAnimation { target: bottomTrackCheckButton; property: "opacity"; to: 1.0; duration: 500 }
        //     }

        //     onPulseChanged: {
        //         if (!pulse) {
        //            bottomTrackCheckButton.opacity = 1.0;
        //         }
        //     }

        //     onHoveredChanged: {
        //         toolbarRoot.isBottomTrackCheckButtonHovered = hovered
        //     }

        //     onCheckedChanged: {
        //         Scene3dToolBarController.onUpdateBottomTrackCheckButtonCheckedChanged(checked)
        //         BottomTrackControlMenuController.onVisibilityCheckBoxCheckedChanged(checked)

        //         if (checked) {
        //             toolbarRoot.updateBottomTrack()
        //         }
        //     }

        //     Component.onCompleted: {
        //         Scene3dToolBarController.onUpdateBottomTrackCheckButtonCheckedChanged(checked)
        //         BottomTrackControlMenuController.onVisibilityCheckBoxCheckedChanged(checked)
        //     }
        // }


        Item
        {
            id:     isobathsWrapper
            width : isobathsCheckButton.implicitWidth
            height: isobathsCheckButton.implicitHeight

            CheckButton {
                id: isobathsCheckButton
                iconSource: "qrc:/XR/contour.png"
                backColor:          theme.controlBackColor
                borderColor:        theme.controlBackColor
                checkedBorderColor: theme.controlBorderColor
                checked: false
                implicitHeight:     theme.menuWidth
                implicitWidth:      theme.menuWidth

                property bool pulse: core.dataProcessorState === 2

                onPulseChanged: {
                    if (!pulse) {
                        isobathsCheckButton.opacity = 1.0;
                    }
                }

                onCheckedChanged: {
                    IsobathsViewControlMenuController.onProcessStateChanged(checked);
                    IsobathsViewControlMenuController.onIsobathsVisibilityCheckBoxCheckedChanged(checked)
                }

                Component.onCompleted: {
                    IsobathsViewControlMenuController.onProcessStateChanged(checked);
                    IsobathsViewControlMenuController.onIsobathsVisibilityCheckBoxCheckedChanged(checked)
                }

            }

        }



        // erase route
        Item {
            id: eraseViewWrapper
            width : eraseRouteButton.implicitWidth
            height: eraseRouteButton.implicitHeight

            CheckButton {
                id: eraseRouteButton
                iconSource: "qrc:/icons/ui/erase.svg"
                backColor: theme.controlBackColor
                borderColor: theme.controlBackColor
                checkedBorderColor: theme.controlBorderColor
                checkable: false
                implicitHeight: theme.menuWidth
                implicitWidth: theme.menuWidth

                property bool pulse: core.dataProcessorState === 3

                SequentialAnimation {
                    running: eraseRouteButton.pulse
                    loops: Animation.Infinite
                    NumberAnimation { target: eraseRouteButton; property: "opacity"; to: 0.2; duration: 500 }
                    NumberAnimation { target: eraseRouteButton; property: "opacity"; to: 1.0; duration: 500 }
                }

                onPulseChanged: {
                    if (!pulse) {
                        eraseRouteButton.opacity = 1.0;
                    }
                }

                onClicked: core.clearRouteData()
            }

        }


        ButtonGroup {
            property bool buttonChangeFlag : false
            id: buttonGroup
            onCheckedButtonChanged: buttonChangeFlag = true
            onClicked: {
                if (!buttonChangeFlag) {
                    checkedButton = null
                }

                buttonChangeFlag = false;
            }
        }


    }

}
