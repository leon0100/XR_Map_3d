import QtQuick 2.15
import QtQuick.Controls 2.15



CheckBox {
     id: control
     implicitHeight: iconSize * 1.2
     implicitWidth: text.length > 0 ? indicator.width + spacing + contentItem.implicitWidth : indicator.width

     property int iconSize: theme.iconSize * 1.4


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
             x: backRect.width * 0.25
             y: backRect.height * 0.25
             radius: 1
             color: theme.textColor
             visible: control.checked
         }
     }

     contentItem: CText {
         text: control.text
         font.pixelSize: iconSize
         verticalAlignment: Text.AlignVCenter
         leftPadding: control.text.length > 0 ? control.indicator.width + control.spacing : 0
         visible: control.text.length > 0
     }
 }
