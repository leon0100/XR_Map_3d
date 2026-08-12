import QtQuick 2.15
import QtQuick.Layouts 1.15


Rectangle {
    id: xrBtn
    width:  recSize * 2
    // height: width * 0.5
    radius: width * 0.1

    property  int  recSize: theme.iconSize * 2
    property  int  recIconSize: height * 0.8
    property  int  recTextSize: height * 0.6

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

    property  color  normalColor:  "#b9cceb"
    property  color  hoverColor:   "#a9bde8"
    property  color  checkedColor: "#9DB5F2"
    property  color  borderColor0: "#828282"
    property  color  borderColor1: "#363636"


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
            text: xrBtn.buttonText
            color: "black"
            font.pixelSize: xrBtn.recTextSize
            Layout.alignment: Qt.AlignVCenter
        }

    }

    states: [
        // 默认状态
        State {
            name: "normal"
            when: !xrBtn.checked && !xrBtn.hovered && !xrBtn.pressed
            PropertyChanges {
                target: xrBtn
                color: xrBtn.normalColor
                border.color: borderColor0
                border.width: 1
                opacity: 0.8
            }
        },


        // 鼠标悬停
        State {
            name: "hover"
            when: !xrBtn.checked && xrBtn.hovered && !xrBtn.pressed
            PropertyChanges {
                target: xrBtn
                color: xrBtn.hoverColor
                border.color: borderColor1
                opacity: 0.9
            }
        },


        // 鼠标按下
        State {
            name: "pressed"
            when: xrBtn.pressed
            PropertyChanges {
                target: xrBtn
                scale: 0.95
                opacity: 1.0
            }
        },


        // checked状态
        State {
            name: "checked"
            when: xrBtn.checked && !xrBtn.hovered
            PropertyChanges {
                target: xrBtn
                color: xrBtn.checkedColor
                border.color: borderColor1
                border.width: 2
                opacity: 0.9
            }
        },


        // checked + hover
        State {
            name: "checkedHover"
            when: xrBtn.checked && xrBtn.hovered
            PropertyChanges {
                target: xrBtn
                color: xrBtn.checkedColor
                border.color: borderColor1
                border.width: 2
                opacity: 1.0
            }
        },


        // checked + pressed
        State {
            name: "checkedPressed"
            when: xrBtn.checked && xrBtn.pressed
            PropertyChanges {
                target: xrBtn
                color: Qt.darker(xrBtn.checkedColor, 1.15)
                scale: 0.95
                border.color: borderColor1
                border.width: 2
            }
        }

    ]

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
        }

    }

}


