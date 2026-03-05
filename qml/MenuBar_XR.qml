import QtQuick 2.15
import QtQuick.Controls 2.15

MenuBar {
    id: root
    height: 52

    // ================== 全局参数 ==================
    readonly property int menuBarHeight: 52
    readonly property int menuItemHeight: 50
    readonly property int menuItemWidth: 150

    signal openClicked()
    signal fullScreenToggled(bool isFull)


    delegate: MenuBarItem {
        id: barItem
        implicitHeight: menuBarHeight
        contentItem: Text {
            text: barItem.text
            font.pixelSize: 25
            font.bold: true
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
        background: Rectangle {
            color: barItem.highlighted ? "#87ceff" : "transparent"
        }
    }

    Component {
        id: myMenuItemDelegate
        MenuItem {
            id: mItem
            implicitWidth: menuItemWidth
            implicitHeight: menuItemHeight

            arrow: Text {
                x: mItem.width - width - 10
                y: (mItem.height - height) / 2
                text: "›"
            }

            contentItem: Text {
                text: mItem.text
                verticalAlignment: Text.AlignVCenter
                leftPadding: 10
            }
            background: Rectangle {
                color: mItem.highlighted ? "#87ceff" : "transparent"
            }
        }
    }




/*----------------------------------------------------------------------*/
    Menu {
        title: qsTr("File")
        delegate: myMenuItemDelegate

        MenuItem {
            text: qsTr("Open")
            onTriggered:core.openFileFromMenu()
        }

        Menu {
            title: qsTr("Save")
            delegate: myMenuItemDelegate
            MenuItem { text: qsTr("Chinese") }
            MenuItem { text: qsTr("English") }
        }

        MenuSeparator {
            contentItem: Rectangle { implicitHeight: 1; color: "#e0e0e0" }
        }

        MenuItem {
            text: qsTr("Exit")
            onTriggered: Qt.quit()
        }
    }

    Menu {
        title: qsTr("Setting")
        delegate: myMenuItemDelegate

        Menu {
            title: qsTr("Language")
            delegate: myMenuItemDelegate
            MenuItem { text: qsTr("English") }
            MenuItem { text: qsTr("Chinese") }
        }

        Menu {
            title: qsTr("Map")
            delegate: myMenuItemDelegate

            MenuItem { text: qsTr("Openstreet") }
            MenuItem { text: qsTr("User Defined") }
        }


        Menu {
            title: qsTr("Units")
            delegate: myMenuItemDelegate

            MenuItem { text: qsTr("Metres") }
            MenuItem { text: qsTr("Feet") }
        }
    }

    Menu {
        title: qsTr("Help")
        delegate: myMenuItemDelegate

        Menu {
            title: qsTr("Registration")
        }

        Menu {
            title: qsTr("About")
        }

    }
}


