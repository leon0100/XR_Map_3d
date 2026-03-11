import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

Popup {
    id: mainMenuPopup

    width: menuSize
    height: menuSize * 0.75
    x: start
    y: start
    padding: 10


    background: Rectangle {
        color: "#ffffff"
        border.color: "#d0d0d0"
        radius: 4
        layer.enabled: true
    }

    property int menuSize: Math.min(Screen.width, Screen.height) * 0.2
    property int itemFontSize: menuSize / 6
    property int start: 2

    // 内容区域：使用 Column 布局
    contentItem: Column {
        width: parent.availableWidth
        spacing: 2

        // 菜单项 1------------------------------------------
        MenuItem {
            text: qsTr("File")
            width: parent.width
            font.pixelSize: itemFontSize

            onClicked: fileSubMenu.open()

            Menu {
                id: fileSubMenu
                width: parent.width
                x: parent.width + start
                y: 0

                MenuItem {
                    text: qsTr("Open")
                    font.pixelSize: itemFontSize
                    onClicked: core.openFileFromMenu()
                }

                MenuItem {
                    text: qsTr("Save")
                    font.pixelSize: itemFontSize
                }

                MenuItem {
                    text: qsTr("Exit")
                    font.pixelSize: itemFontSize
                    onClicked: Qt.quit()
                }
            }
        }

        MenuSeparator {
            width: parent.width;
            contentItem: Rectangle {
                implicitHeight: 1;
                color: "#c0c0c0"
            }
        }



        // 菜单项 2--------------------------------------------------------
        MenuItem {
            text: qsTr("Settings")
            width: parent.width + start
            font.pixelSize: itemFontSize
            onClicked: settingsSubMenu.open()

            Menu {
                id: settingsSubMenu
                width: parent.width
                x: parent.width + start
                y: 0

                MenuItem {
                    text: qsTr("Language")
                    font.pixelSize: itemFontSize
                    onClicked: langSubsubMenu.open()

                    Menu {
                        id: langSubsubMenu
                        width: parent.width
                        x: 0
                        // y: 0

                        MenuItem {
                            text: qsTr("English")
                            font.pixelSize: itemFontSize
                        }

                        MenuItem {
                            text: qsTr("Chinese")
                            font.pixelSize: itemFontSize
                        }
                    }

                }

                MenuItem {
                    text: qsTr("Map")
                    font.pixelSize: itemFontSize
                    onClicked: mapSubsubMenu.open()


                    Menu {
                        id: mapSubsubMenu
                        width: parent.width
                        x: 0

                        MenuItem {
                            text: qsTr("OpenStreet")
                            font.pixelSize: itemFontSize
                        }

                        MenuItem {
                            text: qsTr("Google")
                            font.pixelSize: itemFontSize
                        }

                        MenuItem {
                            text: qsTr("User Defined")
                            font.pixelSize: itemFontSize
                        }
                    }
                }

                MenuItem {
                    text: qsTr("Units")
                    font.pixelSize: itemFontSize
                    onClicked: unitsSubsubMenu.open()

                    Menu {
                        id: unitsSubsubMenu
                        width: parent.width
                        x: 0

                        MenuItem {
                            text: qsTr("Metres")
                            font.pixelSize: itemFontSize
                        }

                        MenuItem {
                            text: qsTr("Feet")
                            font.pixelSize: itemFontSize
                        }
                    }
                }
            }

        }

        MenuSeparator {
            width: parent.width
            contentItem: Rectangle {
                implicitHeight: 1
                color: "#c0c0c0"
            }
        }





        // 菜单项 3---------------------------------------------------
        MenuItem {
            text: qsTr("Help")
            width: parent.width
            font.pixelSize: itemFontSize
            onClicked: helpSubMenu.open()

            Menu {
                id: helpSubMenu
                width: parent.width
                x: parent.width + start
                y: 0

                MenuItem {
                    text: qsTr("Registration")
                    font.pixelSize: itemFontSize
                }

                MenuItem {
                    text: qsTr("About")
                    font.pixelSize: itemFontSize
                }

            }
        }





    }

}
