import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import QtQuick.Window 2.15

Rectangle {
    id: loadMap
    width: loadSize * 1.3
    height: loadSize * 1.6
    x: Screen.width * 0.5 - width * 0.5
    y: Screen.height * 0.5 - height * 0.8
    z: 99
    visible: theme.mapSourceLoadVisible

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#f5f7fa" }
            GradientStop { position: 1.0; color: "#c3cfe2" }
        }
    }


    property int  loadSize:         Math.min(Screen.width, Screen.height) * 0.35
    property int  layoutHeight:    loadSize * 0.1
    property int  iconSize:        loadSize * 0.06

    property bool googleMapExists: false
    property var menuBar: null

    signal mapLoadConfirm(bool hasGoogle)
    signal openGoogleHelpDocument()

    // 主布局
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        anchors.leftMargin: 40
        anchors.rightMargin: 40
        spacing: 10

        Label { text: qsTr("Available Map Source") }

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

                delegate: ItemDelegate {
                    width: listView.width
                    height: model.isSeparator ? 20 : (model.isLine ? 5 : 35)
                    enabled: model.isSelectable
                    highlighted: ListView.isCurrentItem

                    // 内容区
                    contentItem: Item {
                        anchors.fill: parent

                        // 正常文本
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: 10
                            text: model.name
                            visible: !model.isSeparator && !model.isLine && !model.isCenterText
                        }

                        // 居中文本 (对应 User Defined)
                        Text {
                            anchors.centerIn: parent
                            text: model.name
                            visible: model.isCenterText
                            color: "gray"
                        }

                        // 分割线
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
                            onTriggered: {
                                mapModel.remove(index)
                                googleMapExists = false
                            }
                        }
                    }
                }
            }
        }

        // User Config 区域
        RowLayout {
            Layout.fillWidth: true
            Label {
                id: configTileLabel
                text: qsTr("User Config")
            }
            Item { Layout.fillWidth: true }
            ToolButton {
                id: helpBtn
                icon.source: "qrc:/XR/question_mark.svg"
                implicitWidth: 24
                implicitHeight: 24
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Help Document")
                onClicked: theme.openGoogleHelpDocument();
            }
        }

        // 添加 URL 区域
        Item {
            id: urlInputContainer
            Layout.fillWidth: true
            height: 40

            // 当前输入内容
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
                    radius: 4
                    border.color: "#999"
                    border.width: 1
                    color: "white"

                    Text {
                        id: inputTextItem
                        anchors.centerIn: parent
                        text: urlInputContainer.inputText === ""
                              ? qsTr("Enter URL here...")
                              : urlInputContainer.inputText
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
                    implicitWidth: 24
                    implicitHeight: 24
                    onClicked: {
                        if (!googleMapExists) {
                            if ((urlInputContainer.inputText === urlInputContainer.googleUrlEn)
                                    || (urlInputContainer.inputText === urlInputContainer.googoleUrlCh)) {

                                mapModel.append({
                                    "name": "Google Map",
                                    "isSelectable": true,
                                    "canDelete": true,
                                    "isSeparator": false,
                                    "isLine": false,
                                    "isCenterText": false
                                })
                                googleMapExists = true
                                urlInputContainer.inputText = ""
                                GetInterface.showDialogInfo(0, qsTr("Map Add Success!"))
                            } else {
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
                width: 50
                height: 28
                radius: 4
                color: "#00ee76"
                visible: false
                z: 10

                Text {
                    anchors.centerIn: parent
                    text: qsTr("Paste")
                    color: "white"
                    font.pixelSize: 13
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


        Item { Layout.preferredHeight: 2 }

        // 底部按钮
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 100

            Button {
                text: qsTr("OK")
                implicitWidth: 80
                onClicked: {
                    if (!googleMapExists) {
                        if ((urlInputContainer.inputText === urlInputContainer.googleUrlEn)
                                || (urlInputContainer.inputText === urlInputContainer.googoleUrlCh)) {
                            mapModel.append({
                                "name": "Google Map",
                                "isSelectable": true,
                                "canDelete": true,
                                "isSeparator": false,
                                "isLine": false,
                                "isCenterText": false
                            })
                            googleMapExists = true
                            urlInputContainer.inputText = ""
                            GetInterface.showDialogInfo(0, qsTr("Map Add Success!"))
                            if(loadMap.menuBar.hasGoogleMap) {
                                urlInputContainer.inputText = ""
                                loadMap.visible = false
                                GetInterface.showDialogInfo(0, qsTr("Map is Exist!"))
                            }
                            else {
                                theme.mapLoadConfirm(googleMapExists)
                                loadMap.visible = false
                            }
                        } else {
                            urlInputContainer.inputText = ""
                            GetInterface.showDialogInfo(0, qsTr("Map Url is Error!"))
                        }
                    }
                    else {
                        urlInputContainer.inputText = ""
                        if(loadMap.menuBar.hasGoogleMap) {
                            urlInputContainer.inputText = ""
                            loadMap.visible = false
                            GetInterface.showDialogInfo(0, qsTr("Map is Exist!"))
                        }
                        else {
                            theme.mapLoadConfirm(googleMapExists)
                            loadMap.visible = false
                        }

                    }

                }
            }
            Button {
                text: qsTr("Cancel")
                implicitWidth: 80
                onClicked: loadMap.visible = false
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
        ListElement { name: ""; isSelectable: false; canDelete: false; isSeparator: false; isLine: true; isCenterText: false } // 横线
        ListElement { name: qsTr("User Defined"); isSelectable: false; canDelete: false; isSeparator: false; isLine: false; isCenterText: true }
    }


}
