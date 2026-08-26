import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3
import Qt.labs.settings 1.1

import AppXr 1.0
import Communication 1.0


//地图下侧的工具栏
Item  {
    id: toolbarRoot

    width:  theme.menuWidth * 4
    height: theme.menuWidth

    property var  targetPlot2d:  null
    property var  expandBar:   null

    property bool isNorthModeBtnHovered: false

    property bool toolbarHovered: Qt.platform.os === "android" ? northModeBtn.down : isNorthModeBtnHovered

    Behavior on opacity { NumberAnimation { duration: 120 } }

    property string distance: "0m"
    Connections {
        target: renderer.screetShot

        function onSignalSreenBoxDist(dist) {
            if (dist >= 1000) {
                distance = (dist / 1000).toFixed(2) + " km"
            }
            else {
                distance = dist.toFixed(0) + " m"
            }
        }
    }



    function closeIsobathsOutside(globalX, globalY) {
        if(!mainviewIsobathsBtn.checked) {
            return;
        }
        var pos = mainIsobathsRec.mapFromItem(null, globalX, globalY)
        if(pos.x < 0 || pos.y < 0 || pos.x > mainIsobathsRec.width || pos.y > mainIsobathsRec.height) {
            var btnPos = mainviewIsobathsBtn.mapFromItem(null, globalX, globalY)
            if(btnPos.x < 0 || btnPos.y < 0 || btnPos.x > mainviewIsobathsBtn.width
                    || btnPos.y > mainviewIsobathsBtn.height) {
                mainviewIsobathsBtn.checked = false
            }
        }
    }





    RowLayout {
        id: rowButtons
        spacing: 8
        Layout.alignment: Qt.AlignHCenter


        Item {
            id: scaleBar

            width:  theme.menuWidth * 2.5
            height: theme.menuWidth

            property real lineWidth: theme.iconSize * 0.25
            property real barWidth:  theme.menuWidth * 2.2
            property real barHeight: theme.menuWidth * 0.2

            Column {
                anchors.centerIn: parent
                spacing: 4

                // 距离文字
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: distance
                    color: "white"
                    font.pixelSize: theme.iconSize
                    font.bold: true
                }

                // 比例尺
                Item {
                    width: scaleBar.barWidth
                    height: scaleBar.barHeight

                    // 底部横线
                    Rectangle {
                        x: 0
                        y: parent.height
                        width: parent.width
                        height: scaleBar.lineWidth
                        color: "white"
                    }

                    // 左侧竖线（向下）
                    Rectangle {
                        x: 0
                        y: 0
                        width: scaleBar.lineWidth
                        height: parent.height
                        color: "white"
                    }

                    // 右侧竖线（向下）
                    Rectangle {
                        x: parent.width - scaleBar.lineWidth
                        y: 0
                        width: scaleBar.lineWidth
                        height: parent.height
                        color: "white"
                    }
                }
            }
        }


        Button {
            id: zoomInBtn
            implicitHeight: theme.menuWidth
            implicitWidth:  theme.menuWidth

            CMouseOpacityArea {
                cursorShape: Qt.PointingHandCursor
                toolTipText: qsTr("Zoom In")
                popupPosition: "topRight"
            }

            contentItem: Item {
                Image {
                    source: "qrc:/XR/zoom_in.png"
                    anchors.centerIn: parent
                    width:  theme.menuWidth * 0.8
                    height: theme.menuWidth * 0.8
                }
            }

            background: Rectangle {
                radius: 3
                color: zoomInBtn.down ? "white" : theme.controlBackColor
            }

            onClicked: {
                renderer.zoomInOut(true)
                if(targetPlot2d) {
                    targetPlot2d.closeEchoBathyIsobathOutside(0, 0)
                }
                scene3DToolbar.closeIsobathsOutside(0, 0)
                expandBar.expanded = false
            }

        }


        Button {
            id: zoomOutBtn
            implicitHeight: theme.menuWidth
            implicitWidth:  theme.menuWidth

            CMouseOpacityArea {
                cursorShape: Qt.PointingHandCursor
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
                if(targetPlot2d) {
                    targetPlot2d.closeEchoBathyIsobathOutside(0, 0)
                }
                scene3DToolbar.closeIsobathsOutside(0, 0)
                expandBar.expanded = false
            }

        }


        CheckButton {
            id: northModeBtn
            iconSource: "qrc:/icons/ui/location_pin.svg"
            backColor: "#879fc6"
            checked:    true
            checkedColor: "white"
            checkedBackColor: "white"
            implicitHeight: theme.menuWidth
            implicitWidth: theme.menuWidth
            opacity: toolbarHovered ? 1.0 : 0.5

            CMouseOpacityArea {
                cursorShape: Qt.PointingHandCursor
                toolTipText: qsTr("Lock 2D")
                popupPosition: "topRight"
            }

            onCheckedChanged: {
                core.onIsNorthLocationButtonChanged(checked)
                if(targetPlot2d) {
                    targetPlot2d.closeEchoBathyIsobathOutside(0, 0)
                }
                scene3DToolbar.closeIsobathsOutside(0, 0)
                expandBar.expanded = false
            }

            Component.onCompleted: {
                core.onIsNorthLocationButtonChanged(checked)
            }

            Settings {
                property alias northModeBtn: northModeBtn.checked
            }
        }


        XRTextButton {
            id: mainviewIsobathsBtn
            buttonText: qsTr("Isobaths")
            iconSource: "qrc:/XR/contour.png"
            visible: visualisationLayout.splitMode === 2
            clickAction: function() {
                if(mainviewIsobathsBtn.checked) {
                    var btnPos = mainviewIsobathsBtn.mapToItem(mainview.contentItem, 0, 0)
                    mainIsobathsRec.x = btnPos.x - mainIsobathsRec.width * 0.32
                    mainIsobathsRec.y = btnPos.y - mainIsobathsRec.height * 1.1
                }
            }
        }

        IsobathsRec {
            id: mainIsobathsRec
            parent: mainview.contentItem
            expanded: mainviewIsobathsBtn.checked
            visible: mainviewIsobathsBtn.checked
            dragArea: targetPlot2d
            targetPlot: targetPlot2d
        }

    }

}


