import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

Popup {
    id: mainMenuPopup

    width:  menuSize
    height: contentItem.childrenRect.height + topPadding + bottomPadding
    x: start
    y: start
    padding: 10

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


    property int menuSize: Math.min(Screen.width, Screen.height) * 0.2
    property int itemFontSize: menuSize * 0.16
    property int itemHeight: menuSize * 0.2
    property int start: 2

    property color menuBackColor: "#d6e6ff"
    property color menuPressColor: "#bfefff"


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
                width: itemFontSize/2
                height: itemFontSize/2
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                anchors.rightMargin: 5
            }

            Menu {
                id: fileSubMenu
                background: menuBackground.createObject(fileSubMenu)
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

                // MenuItem {
                //     id: exit
                //     text: qsTr("Exit")
                //     font.pixelSize: itemFontSize
                //     background: Rectangle {
                //         color: exit.pressed ? menuPressColor : menuBackColor
                //     }
                //     onClicked: Qt.quit()
                // }
            }
        }


        // 菜单项 2--------------------------------------------------------
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
                background: menuBackground.createObject(fileSubMenu)
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
                        background: menuBackground.createObject(fileSubMenu)
                        width: parent.width
                        x: 0

                        // property int checkIndex: 0
                        property int checkIndex: {
                            if (typeof theme !== 'undefined' && theme.currentLanguage === 0) {
                                return 0;   // English
                            }
                            else if (typeof theme !== 'undefined' && theme.currentLanguage === 1) {
                                return 1;   // Chinese
                            }
                            return -1;
                        }

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
                        background: menuBackground.createObject(fileSubMenu)
                        width: parent.width * 1.2
                        x: 0

                        property int checkIndex: 0

                        MenuItem {
                            id: openstreet
                            text: qsTr("OpenStreet")
                            font.pixelSize: itemFontSize
                            background: Rectangle {
                                color: openstreet.pressed ? menuPressColor : menuBackColor
                            }
                            onClicked: mapSubsubMenu.checkIndex = 0

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
                            id: google
                            text: qsTr("Google")
                            font.pixelSize: itemFontSize
                            background: Rectangle {
                                color: google.pressed ? menuPressColor : menuBackColor
                            }
                            onClicked: mapSubsubMenu.checkIndex = 1

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
                            id: user
                            text: qsTr("User Defined")
                            font.pixelSize: itemFontSize
                            background: Rectangle {
                                color: user.pressed ? menuPressColor : menuBackColor
                            }
                            onClicked: mapSubsubMenu.checkIndex = 2

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
                        background: menuBackground.createObject(fileSubMenu)
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
            }

        }


        // 菜单项 3---------------------------------------------------
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
                background: menuBackground.createObject(fileSubMenu)
                width: parent.width
                x: parent.width + start

                MenuItem {
                    id: registration
                    text: qsTr("Registration")
                    font.pixelSize: itemFontSize
                    background: Rectangle {
                        color: registration.pressed ? menuPressColor : menuBackColor
                    }
                }

                MenuItem {
                    id: about
                    text: qsTr("About")
                    font.pixelSize: itemFontSize
                    background: Rectangle {
                        color: about.pressed ? menuPressColor : menuBackColor
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
            onClicked: core.exitApp()
        }


    }

}
