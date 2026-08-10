import QtQuick 2.15
import QtQuick.Layouts 1.15


Rectangle {
    id: xrBtn
    width: recSize * 0.4
    height: recSize * 0.08
    radius: recSize * 0.015

    property  int  recSize: theme.screenSize * 0.35
    property  int  recIconSize: theme.iconSize * 1.5

    property  string  iconSource: ""
    property  string  buttonText: ""
    property  bool    showText: buttonText !== ""
    property  bool    showIcon: iconSource !== ""
    property  var     clickAction: null  //点击回调

    // toggle模式
    property bool checkable: false
    property bool checked: false

    property bool hovered: false
    property bool pressed: false

    property  color   normalColor:  "#879fc6"
    property  color   hoverColor:   "#63b8ff"
    property  color   checkedColor: "#b9cceb"

    color: checked ? checkedColor : hovered ? hoverColor : normalColor
    opacity: hovered ? 0.9 : 0.8
    border.width: hovered ? 2 : 1
    border.color: hovered ? "#ffffff" : "#879fdd"
    scale: pressed ? 0.95 : 1.0

    Behavior on color { ColorAnimation { duration: 150 } }
    Behavior on scale { NumberAnimation { duration: 100; easing.type: Easing.OutQuad } }

    RowLayout {
        anchors.centerIn: parent
        spacing: 8

        Image {
            visible: xrBtn.showIcon
            source: xrBtn.iconSource
            Layout.preferredWidth: xrBtn.recIconSize
            Layout.preferredHeight: xrBtn.recIconSize
            fillMode: Image.PreserveAspectFit
        }

        Text {
            visible: xrBtn.showText
            // anchors.centerIn: parent
            text: xrBtn.buttonText
            color: "white"
            font.pixelSize: recSize * 0.056
            Layout.alignment: Qt.AlignVCenter
        }

    }

    MouseArea {
        anchors.fill: parent

        hoverEnabled: true

        onEntered: xrBtn.hovered = true

        onExited:  xrBtn.hovered = false

        onPressed: xrBtn.pressed = true

        onReleased: xrBtn.pressed = false

        onClicked: {
            if(xrBtn.checkable) {
                xrBtn.checked=!xrBtn.checked
            }

            if(xrBtn.clickAction) {
                xrBtn.clickAction()
            }
            if(xrBtn.checked) {
                color = xrBtn.hoverColor
                border.color = "#ffffff"
            }
            else {
                color = xrBtn.normalColor
                border.color = "#879fdd"
            }
        }

    }

}


