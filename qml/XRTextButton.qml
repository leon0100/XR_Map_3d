import QtQuick 2.15
import QtQuick.Layouts 1.15


Rectangle {
    id: xrTxtBtn
    width:  recSize * 4
    height: width * 0.2
    radius: height * 0.2

    property  int  recSize: theme.iconSize * 2.5
    property  int  recIconSize: height * 0.8
    property  int  recTextSize: height * 0.7

    property  string  iconSource: ""
    property  string  buttonText: ""

    property  bool    hovered: false
    property  bool    pressed: false

    property  bool    checkable: true
    property  bool    checked: false

    property  var     clickAction: null

    property  color   normalColor:  "#879fc6"
    property  color   hoverColor:   "#b9cceb"
    property  color   checkedColor: "#63b8ff"
    property  color   borderColor0: "#828282"
    property  color   borderColor1: "#363636"


    Behavior on color { ColorAnimation { duration: 150 } }
    Behavior on opacity { NumberAnimation { duration: 150 } }
    Behavior on scale { NumberAnimation { duration: 100; easing.type: Easing.OutQuad } }


    RowLayout {
        anchors.fill: parent

        spacing: 2

        Image {
            source: xrTxtBtn.iconSource
            Layout.preferredWidth:  recIconSize
            Layout.preferredHeight: recIconSize
            Layout.alignment: Qt.AlignVCenter
            fillMode: Image.PreserveAspectFit
        }

        Item {
            Layout.fillWidth: true
            Text {
                anchors.centerIn: parent
                text: xrTxtBtn.buttonText
                color: "white"
                font.pixelSize: recTextSize
            }
        }
    }


    states: [
        // 默认状态
        State {
            name: "normal"
            when: !xrTxtBtn.checked && !xrTxtBtn.hovered && !xrTxtBtn.pressed
            PropertyChanges {
                target: xrTxtBtn
                color: xrTxtBtn.normalColor
                border.color: borderColor0
                border.width: 1
                opacity: 0.8
            }
        },


        // 鼠标悬停
        State {
            name: "hover"
            when: !xrTxtBtn.checked && xrTxtBtn.hovered && !xrTxtBtn.pressed
            PropertyChanges {
                target: xrTxtBtn
                color: xrTxtBtn.hoverColor
                border.color: borderColor1
                opacity: 0.9
            }
        },


        // 鼠标按下
        State {
            name: "pressed"
            when: xrTxtBtn.pressed
            PropertyChanges {
                target: xrTxtBtn
                scale: 0.95
                opacity: 1.0
            }
        },


        // checked状态
        State {
            name: "checked"
            when: xrTxtBtn.checked && !xrTxtBtn.hovered
            PropertyChanges {
                target: xrTxtBtn
                color: xrTxtBtn.checkedColor
                border.color: borderColor1
                border.width: 2
                opacity: 0.9
            }
        },


        // checked + hover
        State {
            name: "checkedHover"
            when: xrTxtBtn.checked && xrTxtBtn.hovered
            PropertyChanges {
                target: xrTxtBtn
                color: xrTxtBtn.checkedColor
                border.color: borderColor1
                border.width: 2
                opacity: 1.0
            }
        },


        // checked + pressed
        State {
            name: "checkedPressed"
            when: xrTxtBtn.checked && xrTxtBtn.pressed
            PropertyChanges {
                target: xrTxtBtn
                color: Qt.darker(xrTxtBtn.checkedColor, 1.15)
                scale: 0.95
                border.color: borderColor1
                border.width: 2
            }
        }

    ]



    MouseArea {
        anchors.fill: parent

        hoverEnabled: true
        onEntered: xrTxtBtn.hovered = true
        onExited: xrTxtBtn.hovered = false

        onPressed: xrTxtBtn.pressed = true
        onReleased: xrTxtBtn.pressed = false

        onClicked: {
            if(xrTxtBtn.checkable) {
                xrTxtBtn.checked=!xrTxtBtn.checked
            }
            if(xrTxtBtn.clickAction) {
                xrTxtBtn.clickAction()
            }
        }

    }



}
