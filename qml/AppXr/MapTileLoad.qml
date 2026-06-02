import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import QtQuick.Window 2.15


Rectangle {
    id: loadMap
    width:  loadSize * 1.5
    height: loadSize * 1.6
    x: Screen.width * 0.5  - width * 0.5
    y: Screen.height * 0.5 - height * 0.8
    z: 99
    visible: theme.mapSourceLoadVisible

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0;  color: "#f5f7fa" }
            GradientStop { position: 1.0;  color: "#c3cfe2" }
        }
    }


    property int  loadSize:  Math.min(Screen.width, Screen.height) * 0.35
    property int  iconSize:  loadSize * 0.06
    property bool googleMapExists_inLoadMap: theme.googleExist

    signal updateMapCheck(int value)


    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        anchors.leftMargin: 40
        anchors.rightMargin: 40
        anchors.bottomMargin: loadSize * 0.05
        spacing: 10

        Label { text: qsTr("Available Map Source");  font.pixelSize: iconSize * 0.8; }

        // 列表区域
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            border.color: "lightgray"

            ListView {
                id: listView
                anchors.fill: parent
                anchors.margins: 2
                clip: true
                model: mapModel
                currentIndex: -1
                spacing: 25

                delegate: ItemDelegate
                {
                    width: parent.width
                    height: model.isSeparator ? 20 : (model.isLine ? 5 : iconSize)
                    visible: !(model.isGoogle && !googleMapExists_inLoadMap)
                    enabled: model.isSelectable
                    highlighted: ListView.isCurrentItem

                    contentItem: Item {
                        anchors.fill: parent

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            text: model.name
                            font.pixelSize: iconSize
                            visible: !model.isSeparator && !model.isLine && !model.isCenterText
                        }

                        // 居中文本 (对应 User Defined)
                        Text {
                            anchors.centerIn: parent
                            text: model.name
                            font.pixelSize: iconSize
                            visible: model.isCenterText
                            color: "gray"
                        }

                        Rectangle {
                            anchors.centerIn: parent
                            width: parent.width - 20
                            height: 1
                            color: "gray"
                            visible: model.isLine
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onClicked: (mouse) => {
                            if (model.isSelectable) {
                                listView.currentIndex = index;
                            }
                            if(model.canDelete) {
                                if((mouse.button === Qt.RightButton) || (mouse.button === Qt.LeftButton)) {
                                    contextMenu.popup();
                                }
                            }

                        }
                    }

                    Menu {
                        id: contextMenu
                        MenuItem {
                            text: qsTr("Delete")
                            font.pixelSize: iconSize
                            implicitWidth: iconSize * 6
                            onTriggered: {
                                googleMapExists_inLoadMap = false
                                theme.googleExist = false
                                if(theme.currentLanguage === 1) {
                                    core.switchMapType(3)
                                    theme.currentMaptype = 3
                                    updateMapCheck(3)
                                }
                                else if(theme.currentLanguage === 0) {
                                    core.switchMapType(2)
                                    theme.currentMaptype = 2
                                    updateMapCheck(2)
                                }
                            }
                        }
                    }

                }

            }
        }


        Loader {
            id: helpLoader
            active: false
            source: "HelpDialog.qml"

            onLoaded: {
                if (item && item.open) {
                    item.open()
                }
            }
        }

        // User Config 区域
        RowLayout {
            Layout.fillWidth: true
            Label {
                id: configTileLabel
                text: qsTr("User Config")
                font.pixelSize: iconSize
            }
            Item { Layout.fillWidth: true }
            ToolButton {
                id: helpBtn
                icon.source: "qrc:/XR/question_mark.svg"
                icon.width: iconSize * 1.2
                icon.height: iconSize * 1.2
                implicitWidth:  iconSize * 1.2
                implicitHeight: iconSize * 1.2
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Help Document")
                onClicked: {
                    if(Qt.platform.os === "android") {
                        helpLoader.active = true
                        helpLoader.item.open()
                    }
                    else {
                        theme.openGoogleHelpDocument()
                    }
                }
            }
        }

        Item { Layout.preferredHeight: 10 }

        // 添加 URL 区域
        Item {
            id: urlInputContainer
            Layout.fillWidth: true
            height: iconSize * 1.5

            property string inputText: ""
            readonly property string googleUrlEn: "http://mt2.google.com/vt/lyrs=y&hl=en&x=%1&y=%2&z=%3"
            readonly property string googoleUrlCh: "http://mt2.google.com/vt/lyrs=y&hl=zh-CN&x=%1&y=%2&z=%3"

            TextEdit { id: clipboardBridge;   visible: false }

            RowLayout {
                id: urlInputRow
                anchors.fill: parent
                spacing: 8

                Rectangle {
                    id: fakeInput
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 5
                    border.color: "#999"
                    border.width: 1
                    color: "white"

                    Text {
                        id: inputTextItem
                        anchors.centerIn: parent
                        text: urlInputContainer.inputText === ""
                              ? qsTr("Enter URL here...")
                              : urlInputContainer.inputText
                        font.pixelSize: iconSize
                        color: urlInputContainer.inputText === "" ? "#999" : "#000"
                        elide: Text.ElideRight
                        width: parent.width - 20
                    }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton

                        onClicked: (mouse) => {
                            // 1. 让中转站执行粘贴动作
                            clipboardBridge.paste()

                            // 2. 检查粘贴后中转站里是否有内容
                            if (clipboardBridge.text !== "") {
                                pasteHint.visible = true
                                pasteHint.x = mouse.x > (parent.width - pasteHint.width) ?
                                               (parent.width - pasteHint.width) : mouse.x
                                pasteHint.y = -pasteHint.height - 5
                            }
                        }
                    }
                }

                // 右侧功能图标（加号按钮）
                ToolButton {
                    id: addUrlBtn
                    icon.source: "qrc:/XR/plus.svg"
                    icon.width: iconSize * 1.2
                    icon.height: iconSize * 1.2
                    implicitWidth: iconSize * 1.5
                    implicitHeight: iconSize * 1.5
                    onClicked: {
                        if (!googleMapExists_inLoadMap) {
                            if ((urlInputContainer.inputText === urlInputContainer.googleUrlEn)
                            || (urlInputContainer.inputText === urlInputContainer.googoleUrlCh)) {
                                googleMapExists_inLoadMap = true
                                urlInputContainer.inputText = ""
                            }
                            else {
                                urlInputContainer.inputText = ""
                                GetInterface.showDialogInfo(0, qsTr("Map Url is Error!"))
                            }
                        }
                        else {
                            urlInputContainer.inputText = ""
                            GetInterface.showDialogInfo(0, qsTr("Map is Exist!"))
                        }
                    }
                }

            }


            MouseArea {
                id: outsideClickDismiss
                width: 5000
                height: 5000
                x: -2500
                y: -2500
                z: 9
                enabled: pasteHint.visible
                onPressed: pasteHint.visible = false
            }

            // 粘贴提示按钮
            Rectangle {
                id: pasteHint
                width: iconSize * 3
                height: iconSize
                radius: 4
                visible: false
                z: 10

                Text {
                    anchors.centerIn: parent
                    text: qsTr("Paste")
                    font.pixelSize: iconSize
                    font.bold: true
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        urlInputContainer.inputText = clipboardBridge.text
                        pasteHint.visible = false
                        clipboardBridge.text = ""
                    }
                }
            }
        }


        Item { Layout.preferredHeight: loadSize * 0.05 }

        // 底部按钮
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: loadSize * 0.5

            Button {
                text: qsTr("OK")
                font.pixelSize: iconSize
                implicitWidth: iconSize * 5
                implicitHeight: iconSize
                onClicked: {
                    if (!googleMapExists_inLoadMap) {
                        if ((urlInputContainer.inputText === urlInputContainer.googleUrlEn)
                        || (urlInputContainer.inputText === urlInputContainer.googoleUrlCh)) {
                            googleMapExists_inLoadMap = true
                            urlInputContainer.inputText = ""

                            if(theme.googleExist === true) {
                                theme.mapSourceLoadVisible = false
                                GetInterface.showDialogInfo(0, qsTr("Map is Exist!"))
                            }
                            else {
                                theme.googleExist = true
                                theme.mapSourceLoadVisible = false
                                GetInterface.showDialogInfo(0, qsTr("Map Add Success!"))
                            }
                        } else if(urlInputContainer.inputText !== "") {
                            urlInputContainer.inputText = ""
                            GetInterface.showDialogInfo(0, qsTr("Map Url is Error!"))
                        }
                    }
                    else {
                        urlInputContainer.inputText = ""
                        if(theme.googleExist === true) {
                            theme.mapSourceLoadVisible = false
                        }
                        else {
                            theme.googleExist = true
                            theme.mapSourceLoadVisible = false
                            GetInterface.showDialogInfo(0, qsTr("Map Add Success!"))
                        }

                    }

                }
            }
            Button {
                text: qsTr("Cancel")
                font.pixelSize: iconSize
                implicitWidth: iconSize * 5
                implicitHeight: iconSize
                onClicked: theme.mapSourceLoadVisible = false
            }
        }
    }


    // 内部数据模型
    ListModel {
        id: mapModel
        ListElement { name: "OpenStreetMap"; isSelectable: true; canDelete: false; isSeparator: false; isLine: false; isCenterText: false }
        ListElement { name: "GeoVisEarthMap"; isSelectable: true; canDelete: false; isSeparator: false; isLine: false; isCenterText: false }
        ListElement { name: "Amap(高德地图)"; isSelectable: true; canDelete: false; isSeparator: false; isLine: false; isCenterText: false }
        ListElement { name: ""; isSelectable: false; canDelete: false; isSeparator: true; isLine: false; isCenterText: false }  // 空白占位
        ListElement { name: ""; isSelectable: false; canDelete: false; isSeparator: false; isLine: true; isCenterText: false }  // 横线
        ListElement { name: qsTr("User Defined"); isSelectable: false; canDelete: false; isSeparator: false; isLine: false; isCenterText: true }
        ListElement {
            name: "Google Map"
            isSelectable: true
            canDelete: true
            isSeparator: false
            isLine: false
            isCenterText: false
            isGoogle: true
        }
    }


}
