import QtQuick 2.15

Rectangle {
    id: selectionRect
    visible: renderer.screetShot.isSelectionRectVisible

    // 添加坐标转换
    property real mouseX: 0
    property real mouseY: 0

    // 使用绝对坐标
    x: renderer.screetShot.selectionRect.x
    y: renderer.screetShot.selectionRect.y
    width: renderer.screetShot.selectionRect.width
    height: renderer.screetShot.selectionRect.height


    property int iconSize: theme.iconSize


    color: "transparent"
    border.width: 3
    border.color: Qt.rgba(0, 1, 1, 1.0)
    z: 1000


    Text {
       id: topLabel
       text: renderer.screetShot.screetWidth
       color: "yellow"
       font.pixelSize: iconSize
       font.bold: true

       anchors.horizontalCenter: parent.horizontalCenter
       anchors.bottom: parent.top
       anchors.bottomMargin: 6
    }


    Text {
       id: rightLabel
       text: renderer.screetShot.screetHeight
       color: "yellow"
       font.pixelSize: iconSize
       font.bold: true

       anchors.verticalCenter: parent.verticalCenter
       anchors.left: parent.right
       anchors.leftMargin: 6
    }



    property int handleSize: iconSize * 0.4
    property int toolSize: iconSize * 1.4

       // 左上角
       Rectangle {
           width: handleSize
           height: handleSize
           color: "#ff3030"
           border.color: "#ff3030"
           border.width: 2
           x: -handleSize/2
           y: -handleSize/2
       }

       // 右上角
       Rectangle {
           width: handleSize
           height: handleSize
           color: "#ff3030"
           border.color: "#ff3030"
           border.width: 2
           x: parent.width - handleSize/2
           y: -handleSize/2
       }

       // 左下角
       Rectangle {
           width:  handleSize
           height: handleSize
           color: "#ff3030"
           border.color: "#ff3030"
           border.width: 2
           x: -handleSize/2
           y: parent.height - handleSize/2
       }

       // 右下角
       Rectangle {
           width: handleSize
           height: handleSize
           color: "#ff3030"
           border.color: "#ff3030"
           border.width: 2
           x: parent.width - handleSize/2
           y: parent.height - handleSize/2
       }


       /*---- 右下角截图工具栏 ----*/
       Rectangle {
           id: screenToolBar
           visible: renderer.screetShot.screetToolBar

           width: toolSize * 6.2
           height: toolSize * 1.1

           color: "#f0f8ff"
           border.color: "#4f4f4f"
           border.width: 1

           anchors.left: parent.right
           anchors.top:  parent.bottom
           anchors.leftMargin: -width
           anchors.topMargin: 3

           Row {
               anchors.centerIn: parent
               spacing: toolSize * 0.2

               Rectangle {
                   width:  toolSize
                   height: toolSize
                   radius: 4
                   color: penMouse.containsMouse ? "#d6e6ff" : "transparent"

                   Image {
                      anchors.centerIn: parent
                      width:  toolSize * 0.9
                      height: toolSize * 0.9
                      source: "qrc:/XR/wordlog3232.png"
                   }

                   CMouseArea {
                      id: penMouse
                      onEntered: renderer.screetShot.setToArrowCursor()
                      toolTipText: qsTr("Edit")
                   }

               }

               Rectangle {
                   width:  toolSize
                   height: toolSize
                   radius: 4
                   color: textMouse.containsMouse ? "#d6e6ff" : "transparent"

                   Image {
                      anchors.centerIn: parent
                      width:  toolSize * 0.9
                      height: toolSize * 0.9
                      source: "qrc:/XR/undo.svg"
                   }

                   CMouseArea {
                      id: textMouse
                      onEntered: renderer.screetShot.setToArrowCursor()
                      toolTipText: qsTr("Revoke Edit")
                   }
               }

               Rectangle {
                   width:  toolSize
                   height: toolSize
                   radius: 4
                   color: okMouse.containsMouse ? "#d6e6ff" : "transparent"

                   Image {
                      anchors.centerIn: parent
                      width:  toolSize * 0.9
                      height: toolSize * 0.9
                      source: "qrc:/XR/wirelessTrans3232.png"
                   }

                   CMouseArea {
                      id: okMouse
                      onEntered: renderer.screetShot.setToArrowCursor()
                      toolTipText: qsTr("Wireless Trans")
                   }
               }

               Rectangle {
                   width:  toolSize
                   height: toolSize
                   radius: 4
                   color: cancelMouse.containsMouse ? "#d6e6ff" : "transparent"

                   Image {
                      anchors.centerIn: parent
                      width:  toolSize * 0.9
                      height: toolSize * 0.9
                      source: "qrc:/XR/cancelshot.png"
                   }

                   CMouseArea {
                      id: cancelMouse
                      onEntered: renderer.screetShot.setToArrowCursor()
                      toolTipText: qsTr("Cancel Shot")
                      onClicked: {
                          renderer.screetShot.setCancelShot()
                      }
                   }
               }


               Rectangle {
                   width:  toolSize
                   height: toolSize
                   radius: 4
                   color: save.containsMouse ? "#d6e6ff" : "transparent"

                   Image {
                      anchors.centerIn: parent
                      width:  toolSize * 0.9
                      height: toolSize * 0.9
                      source: "qrc:/XR/save.png"
                   }

                   CMouseArea {
                      id: save
                      onEntered: renderer.screetShot.setToArrowCursor()
                      toolTipText: qsTr("Save")
                      onClicked: {
                          renderer.screetShot.saveScreetShot()
                      }
                   }
               }
           }


       }



}
