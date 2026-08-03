import QtQuick 2.15
import QtQuick.Controls 2.15



ComboBox {
    id: control

    property var schemes: [
        [ "#ff5400", "#ff8c00", "#ffc300", "#8a7f70", "#3d3b3c", "#1a1a1a" ],

        [ "#1e3a8a", "#3b82f6", "#93c5fd", "#ffb5a7", "#ff8fab", "#ff4d6d" ],

        [ "#000000", "#2b2b2b", "#555555", "#888888", "#cccccc", "#ffffff" ],

        [ "#ffea00", "#ffaa00", "#ff7b00", "#ff0000", "#d00000", "#800020" ],

        [ "#2ec4b6", "#00a896", "#028090", "#05668d", "#023e8a", "#03045e" ]
    ]


    property int   schemeIconSize: theme.iconSize * 1.2
    property int   schemeLength:   6
    property int   schemeCount:    5


    implicitWidth:  schemeIconSize * 7.5
    implicitHeight: schemeIconSize * 1.2

    model: control.schemeCount


    delegate: ItemDelegate {
        id: itemDelegate

        width:  control.width
        height: schemeIconSize + 10

        property int indexModel: index

        Row {
           anchors.left: parent.left
           anchors.leftMargin: 5
           anchors.verticalCenter: parent.verticalCenter
           spacing: 0

           Repeater {
               model: control.schemeLength
               Rectangle {
                  width:  schemeIconSize
                  height: schemeIconSize
                  color: control.schemes[indexModel][index]
               }
           }
        }

        background: Rectangle {
            color: itemDelegate.highlighted ? "#707070" : "transparent"
            border.width: itemDelegate.highlighted ? 1 : 0
        }

        highlighted: control.highlightedIndex === index
    }


    // 当前选中显示
    contentItem: Row {
        anchors.left: parent.left
        anchors.leftMargin: 5

        Repeater {
            model: control.schemeLength
            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: schemeIconSize
                height: schemeIconSize
                color: control.schemes[control.currentIndex][index]
            }
        }
    }


    // 下拉箭头
    indicator: Canvas {
        id: canvas
        width:  schemeIconSize * 0.8
        height: schemeIconSize * 0.8
        x: control.width - schemeIconSize
        y: control.height * 0.5 - height * 0.5
        contextType: "2d"

        Connections {
            target: theme
            function onThemeIDChanged() {
                canvas.requestPaint()
            }
        }

        onPaint: {
            context.reset();
            if(control.popup.visible) {
                context.moveTo(width * 0.5, 0);
                context.lineTo(width, height);
                context.lineTo(0, height);
            }
            else {
                context.moveTo(0, 0);
                context.lineTo(width, 0);
                context.lineTo(width/2, height);
            }

            context.closePath();
            context.fillStyle = "#696969";
            context.fill();
        }

    }


    // 背景
    background: Rectangle {
        implicitWidth: 100
        implicitHeight: schemeIconSize
        color: "transparent"
        border.width: 1
    }


    popup: Popup {
        x: 0
        y: schemeIconSize + 2
        width:  schemeIconSize * 7
        height: schemeIconSize * 7.3
        implicitHeight: contentItem.implicitHeight
        padding: 0

        onVisibleChanged: {
            canvas.requestPaint()
        }

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            highlightFollowsCurrentItem: false
            focus:true
            ScrollIndicator.vertical: ScrollIndicator {}
        }

    }

}
