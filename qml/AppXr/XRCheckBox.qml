import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15


Item {
    id: control
    width: row.width
    height: row.height

    property int   loadSize: Math.min(Screen.width, Screen.height) * 0.4
    property int   iconSize: loadSize * 0.04
    property int   boxSize:  iconSize * 1.2
    property alias text:     label.text
    property bool  checked:  false

    property string group: ""
    property bool exclusive: true

    signal clicked()


    Row {
        id: row
        anchors.verticalCenter: parent.verticalCenter
        spacing: 6

        Rectangle {
            id: checkBox
            width: boxSize
            height: boxSize
            radius: 2
            border.width: 1
            border.color: "#666666"
            color: "transparent"

            Text {
                anchors.centerIn: parent
                text: "✓"
                visible: control.checked
                font.pixelSize: boxSize
                color: "#0066cc"
                font.bold: true
            }
        }

        Text {
            id: label
            anchors.verticalCenter: parent.verticalCenter
            font.pixelSize: control.iconSize
            color: "#333333"
        }
    }


    MouseArea {
        anchors.fill: parent

        onClicked: {
            if(control.exclusive && control.group !== "") {
                //寻找同级控件
                var items = control.parent.children
                for(var i = 0; i < items.length; i++) {
                    if(items[i] !== control && items[i].group === control.group) {
                        items[i].checked=false
                    }
                }

                control.checked = true
            }
            else {
                control.checked = !control.checked
            }

            control.clicked()
        }

    }

}
