import QtQuick 2.15
import QtQuick.Controls 2.15



CheckBox {
     id: control
     text: qsTr("Check")
     implicitHeight: iconSize * 1.2

     property int iconSize: theme.iconSize * 1.5

     indicator: Rectangle {
         id: backRect
         implicitHeight: control.iconSize
         implicitWidth:  control.iconSize
         radius: 1
         x: control.leftPadding
         y: control.height * 0.5 - height * 0.5
         color: theme.controlBackColor
         border.color: theme.textColor
         border.width: 1

         Rectangle {
             width:  backRect.width * 0.5
             height: backRect.height * 0.5
             x: backRect.width * 0.5
             y: backRect.height * 0.25
             radius: 1
             color: theme.textColor
             visible: control.checked
         }
     }

     contentItem: CText {
         text: control.text
         font.pixelSize: theme.iconSize
         verticalAlignment: Text.AlignVCenter
         leftPadding: control.indicator.width + control.spacing
     }
 }
