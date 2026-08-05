import QtQuick 2.15
import QtQuick.Controls 2.15


Popup {
    id: mainMenuPopup
    width:  menuSize
    height: contentItem.childrenRect.height + topPadding + bottomPadding
    x: start
    y: start
    padding: 10
    closePolicy: Popup.CloseOnPressOutside | Popup.CloseOnEscape

    onClosed: {
        fileSubMenu.close()
        settingsSubMenu.close()
        helpSubMenu.close()
        langSubsubMenu.close()
        mapSubsubMenu.close()
        unitsSubsubMenu.close()
    }


    background: Rectangle {
        color: menuBackColor
        radius: 5
    }

    Component {
        id: menuBackground
        MenuItem {
            background: Rectangle {
              color: menuBackColor
              radius: 5
            }
        }
    }


    property alias menuVisible:  mainMenuPopup.visible
    property int   menuSize:   theme.screenSize * 0.2
    property int   itemFontSize: menuSize * 0.16
    property int   itemHeight:   menuSize * 0.2
    property int   start: 2
    property bool  layoutHorizontal: true

    property color menuBackColor: "#d6e6ff"
    property color menuPressColor: "#bfefff"

    function receiveMapCheck(value) {
        mapSubsubMenu.checkIndex = value
    }

    function closeMenu() {
        fileSubMenu.close()
        settingsSubMenu.close()
        helpSubMenu.close()
        langSubsubMenu.close()
        mapSubsubMenu.close()
        unitsSubsubMenu.close()
        mainMenuPopup.close()
    }

    contentItem: Column {
        width: parent.availableWidth
        spacing: 2

        // 菜单项 1------------------------------------------
        MenuItem {
            id: file
            text: qsTr("File")
            width: parent.width
            height: itemHeight
            font.pixelSize: itemFontSize
            background: Rectangle {
                color: file.pressed ? menuPressColor : menuBackColor
            }
            onClicked: fileSubMenu.open()


            Image {
                source: "qrc:/XR/triangle.svg"
                rotation: 90
                width:  itemFontSize * 0.5
                height: itemFontSize * 0.5
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 5
            }

            Menu {
                id: fileSubMenu
                width: parent.width * 0.8
                x: parent.width + start

                MenuItem {
                    id: open
                    text: qsTr("Open")
                    font.pixelSize: itemFontSize
                    background: Rectangle {
                        color: open.pressed ? menuPressColor : menuBackColor
                    }
                    onClicked: core.openFileFromMenu()

                    Image {
                        source: "qrc:/icons/ui/file_import.svg"
                        width: itemFontSize * 0.75
                        height: itemFontSize * 0.75
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                    }
                }

                MenuItem {
                    id: save
                    text: qsTr("Save")
                    font.pixelSize: itemFontSize
                    background: Rectangle {
                        color: save.pressed ? menuPressColor : menuBackColor
                    }

                    Image {
                        source: "qrc:/icons/ui/file_export.svg"
                        width: itemFontSize * 0.75
                        height: itemFontSize * 0.75
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                    }
                }


                MenuItem {
                    id: exportAsData
                    text: qsTr("Export")
                    font.pixelSize: itemFontSize
                    background: Rectangle {
                        color: exportAsData.pressed ? menuPressColor : menuBackColor
                    }

                    onClicked: theme.exportAsDataVisible = true

                    Image {
                        source: "qrc:/icons/ui/file_export.svg"
                        width: itemFontSize * 0.75
                        height: itemFontSize * 0.75
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                    }
                }

                MenuItem {
                    id: clear
                    text: qsTr("Clear")
                    font.pixelSize: itemFontSize
                    background: Rectangle {
                        color: clear.pressed ? menuPressColor : menuBackColor
                    }

                    onClicked: core.clearAll()

                    Image {
                        source: "qrc:/icons/ui/erase.svg"
                        width: itemFontSize * 0.75
                        height: itemFontSize * 0.75
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                    }
                }

            }
        }


        // 菜单项 2-------------------------------------------------------
        MenuItem {
            id: settings
            text: qsTr("Settings")
            width: parent.width
            height: itemHeight
            font.pixelSize: itemFontSize
            background: Rectangle {
                color: settings.pressed ? menuPressColor : menuBackColor
            }
            onClicked: settingsSubMenu.open()

            Image {
                source: "qrc:/XR/triangle.svg"
                rotation: 90
                width: itemFontSize/2
                height: itemFontSize/2
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 5
            }

            Menu {
                id: settingsSubMenu
                width: parent.width
                x: parent.width + start

                MenuItem {
                    id: lan
                    text: qsTr("Language")
                    font.pixelSize: itemFontSize
                    background: Rectangle {
                        color: lan.pressed ? menuPressColor : menuBackColor
                    }
                    onClicked: langSubsubMenu.open()

                    Image {
                        source: "qrc:/XR/triangle.svg"
                        rotation: 90
                        width: itemFontSize/2
                        height: itemFontSize/2
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                    }

                    Menu {
                        id: langSubsubMenu
                        width: parent.width
                        x: 0

                        property int checkIndex: theme.currentLanguage === 0 ? 0 : 1;

                        MenuItem {
                            id: english
                            text: qsTr("English")
                            font.pixelSize: itemFontSize
                            background: Rectangle {
                                color: english.pressed ? menuPressColor : menuBackColor
                            }
                            onClicked: {
                                langSubsubMenu.checkIndex = 0
                                theme.currentLanguage = 0
                                mapSubsubMenu.checkIndex = 2
                                core.switchMapType(2)
                                theme.currentMaptype = 2
                            }

                            Image {
                                source: "qrc:/XR/check.svg"
                                visible: langSubsubMenu.checkIndex === 0
                                width: itemFontSize
                                height: itemFontSize
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                anchors.rightMargin: 5
                            }
                        }

                        MenuItem {
                            id: chinese
                            text: qsTr("Chinese")
                            font.pixelSize: itemFontSize
                            background: Rectangle {
                                color: chinese.pressed ? menuPressColor : menuBackColor
                            }
                            onClicked: {
                                langSubsubMenu.checkIndex = 1
                                theme.currentLanguage = 1
                                mapSubsubMenu.checkIndex = 3
                                core.switchMapType(3)
                                theme.currentMaptype = 3
                            }

                            Image {
                                source: "qrc:/XR/check.svg"
                                visible: langSubsubMenu.checkIndex === 1
                                width: itemFontSize
                                height: itemFontSize
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                anchors.rightMargin: 5
                            }
                        }
                    }

                }

                MenuItem {
                    id: map
                    text: qsTr("Map")
                    font.pixelSize: itemFontSize
                    background: Rectangle {
                        color: map.pressed ? menuPressColor : menuBackColor
                    }
                    onClicked: mapSubsubMenu.open()

                    Image {
                        source: "qrc:/XR/triangle.svg"
                        rotation: 90
                        width: itemFontSize/2
                        height: itemFontSize/2
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                    }

                    Menu {
                        id: mapSubsubMenu
                        width: parent.width * 1.2
                        x: 0

                        property int checkIndex: theme.currentMaptype

                        MenuItem {
                            id: openstreet
                            text: qsTr("OpenStreet")
                            font.pixelSize: itemFontSize
                            background: Rectangle {
                                color: openstreet.pressed ? menuPressColor : menuBackColor
                            }
                            onClicked: {
                                mapSubsubMenu.checkIndex = 2
                                core.switchMapType(2)
                                theme.currentMaptype = 2
                            }

                            Image {
                                source: "qrc:/XR/check.svg"
                                visible: mapSubsubMenu.checkIndex === 2
                                width: itemFontSize
                                height: itemFontSize
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                anchors.rightMargin: 5
                            }
                        }


                        MenuItem {
                            id: genvisEarth
                            text: qsTr("GeovisEarth")
                            font.pixelSize: itemFontSize
                            background: Rectangle {
                                color: genvisEarth.pressed ? menuPressColor : menuBackColor
                            }
                            visible: theme.currentLanguage === 1
                            height: visible ? implicitHeight : 0
                            onClicked: {
                                mapSubsubMenu.checkIndex = 3
                                core.switchMapType(3)
                                theme.currentMaptype = 3
                            }

                            Image {
                                source: "qrc:/XR/check.svg"
                                visible: mapSubsubMenu.checkIndex === 3
                                width: itemFontSize
                                height: itemFontSize
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                anchors.rightMargin: 5
                            }
                        }



                        MenuItem {
                            id: amap
                            text: qsTr("Amap")
                            font.pixelSize: itemFontSize
                            background: Rectangle {
                                color: amap.pressed ? menuPressColor : menuBackColor
                            }
                            visible: theme.currentLanguage === 1
                            height: visible ? implicitHeight : 0
                            onClicked: {
                                mapSubsubMenu.checkIndex = 1
                                core.switchMapType(1)
                                theme.currentMaptype = 1
                            }

                            Image {
                                source: "qrc:/XR/check.svg"
                                visible: mapSubsubMenu.checkIndex === 1
                                width: itemFontSize
                                height: itemFontSize
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                anchors.rightMargin: 5
                            }
                        }



                        MenuItem {
                            id: google
                            text: qsTr("Google")
                            font.pixelSize: itemFontSize
                            background: Rectangle {
                                color: google.pressed ? menuPressColor : menuBackColor
                            }
                            visible: theme.googleExist
                            height: visible ? implicitHeight : 0
                            onClicked: {
                                mapSubsubMenu.checkIndex = 0
                                core.switchMapType(0)
                                theme.currentMaptype = 0
                            }

                            Image {
                                source: "qrc:/XR/check.svg"
                                visible: mapSubsubMenu.checkIndex === 0
                                width: itemFontSize
                                height: itemFontSize
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                anchors.rightMargin: 5
                            }
                        }

                        MenuItem {
                            id: user
                            text: qsTr("User Defined")
                            font.pixelSize: itemFontSize
                            background: Rectangle {
                                color: user.pressed ? menuPressColor : menuBackColor
                            }
                            onClicked: theme.mapSourceLoadVisible = true
                        }
                    }
                }

                MenuItem {
                    id: units
                    text: qsTr("Units")
                    font.pixelSize: itemFontSize
                    background: Rectangle {
                        color: units.pressed ? menuPressColor : menuBackColor
                    }
                    onClicked: unitsSubsubMenu.open()

                    Image {
                        source: "qrc:/XR/triangle.svg"
                        rotation: 90
                        width: itemFontSize/2
                        height: itemFontSize/2
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                    }

                    Menu {
                        id: unitsSubsubMenu
                        width: parent.width
                        x: 0

                        property int checkIndex: 0

                        MenuItem {
                            id: metres
                            text: qsTr("Metres")
                            font.pixelSize: itemFontSize
                            background: Rectangle {
                                color: metres.pressed ? menuPressColor : menuBackColor
                            }
                            onClicked: unitsSubsubMenu.checkIndex = 0

                            Image {
                                source: "qrc:/XR/check.svg"
                                visible: unitsSubsubMenu.checkIndex === 0
                                width: itemFontSize
                                height: itemFontSize
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                anchors.rightMargin: 5
                            }
                        }

                        MenuItem {
                            id: feet
                            text: qsTr("Feet")
                            font.pixelSize: itemFontSize
                            background: Rectangle {
                                color: feet.pressed ? menuPressColor : menuBackColor
                            }
                            onClicked: unitsSubsubMenu.checkIndex = 1

                            Image {
                                source: "qrc:/XR/check.svg"
                                visible: unitsSubsubMenu.checkIndex === 1
                                width: itemFontSize
                                height: itemFontSize
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                anchors.rightMargin: 5
                            }
                        }
                    }
                }


                MenuItem {
                    id: sonarLayout
                    text: qsTr("Vertical")
                    font.pixelSize: itemFontSize
                    background: Rectangle {
                        color: sonarLayout.pressed ? menuPressColor : menuBackColor
                    }
                    onClicked: {
                        layoutHorizontal = !layoutHorizontal
                    }

                    Image {
                        source: "qrc:/XR/check.svg"
                        visible: layoutHorizontal === false
                        width: itemFontSize
                        height: itemFontSize
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                    }
                }

                MenuItem {
                    id: liveData
                    text: qsTr("Live Data")
                    font.pixelSize: itemFontSize
                    background: Rectangle {
                        color: about.pressed ? menuPressColor : menuBackColor
                    }
                    onClicked: {
                        mainMenuPopup.closeMenu()
                        if(theme.liveDataVisible === false) {
                            BleManager.setBleLiveScanningVisible(true)
                            theme.liveDataVisible = true
                        }
                        else {
                            BleManager.setBleLiveScanningVisible(false)
                            theme.liveDataVisible = false
                        }
                    }

                    Image {
                        source: "qrc:/XR/check.svg"
                        visible: theme.liveDataVisible === true
                        width: itemFontSize
                        height: itemFontSize
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                    }
                }




            }

        }


        // 菜单项 3----------------------------------------
        MenuItem {
            id: help
            text: qsTr("Help")
            width: parent.width
            height: itemHeight
            font.pixelSize: itemFontSize
            background: Rectangle {
                color: help.pressed ? menuPressColor : menuBackColor
            }
            onClicked: helpSubMenu.open()

            Image {
                source: "qrc:/XR/triangle.svg"
                rotation: 90
                width: itemFontSize/2
                height: itemFontSize/2
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 5
            }

            Menu {
                id: helpSubMenu
                width: parent.width
                x: parent.width + start

                MenuItem {
                    id: registration
                    text: qsTr("Registration")
                    font.pixelSize: itemFontSize
                    background: Rectangle {
                        color: registration.pressed ? menuPressColor : menuBackColor
                    }
                    onClicked: {
                        mainMenuPopup.closeMenu()
                    }
                }

                MenuItem {
                    id: about
                    text: qsTr("About")
                    font.pixelSize: itemFontSize
                    background: Rectangle {
                        color: about.pressed ? menuPressColor : menuBackColor
                    }

                    onClicked: {
                        mainMenuPopup.closeMenu()
                    }
                }

            }
        }

        MenuItem {
            id: exit
            text: qsTr("Exit")
            width: parent.width
            height: itemHeight
            font.pixelSize: itemFontSize
            background: Rectangle {
                color: exit.pressed ? menuPressColor : menuBackColor
            }
            onClicked: {
                core.exitApp()
                mainMenuPopup.closeMenu()
            }
        }

    }

}
