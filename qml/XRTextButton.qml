import QtQuick 2.15
import QtQuick.Layouts 1.15


Rectangle {
    id: xrTxtBtn
    width: recSize * 0.4
    height: recSize * 0.08
    radius: recSize * 0.02

    property  int  recSize: theme.screenSize * 0.35
    property  int  recIconSize: theme.iconSize * 1.5

    property  string  iconSource: ""
    property  string  buttonText: ""
    property  var     clickAction: null
    property  bool    showText: true
    property  color   normalColor:  "#879fc6"
    property  color   hoverColor:   "#63b8ff"
    property  color   visibleColor: "#b9cceb"
    property  bool    hovered: false
    property  bool    pressed: false
    property  bool    settingVisible: false

    color: settingVisible ? visibleColor : hovered ? hoverColor : normalColor
    opacity: hovered ? 0.9 : 0.75
    border.width: 1
    border.color: hovered ? "#ffffff" : "#879fdd"
    scale: pressed ? 0.95 : 1.0

    Behavior on color { ColorAnimation { duration: 150 } }
    Behavior on opacity { NumberAnimation { duration: 150 } }
    Behavior on scale { NumberAnimation { duration: 100; easing.type: Easing.OutQuad } }


    MouseArea {
        anchors.fill: parent

        hoverEnabled: true

        onEntered: {
            xrTxtBtn.hovered = true
        }

        onExited: {
            xrTxtBtn.hovered = false
        }

        onPressed: {
            xrTxtBtn.pressed = true
        }

        onReleased: {
            xrTxtBtn.pressed = false
        }

        onClicked: {
            if(xrTxtBtn.clickAction) {
                xrTxtBtn.clickAction()
            }
            xrTxtBtn.settingVisible = !xrTxtBtn.settingVisible
            if(xrTxtBtn.settingVisible) {
                color = xrTxtBtn.hoverColor
                border.color = "#ffffff"
            }
            else {
                color = xrTxtBtn.normalColor
                border.color = "#879fdd"
            }
        }

    }


    RowLayout {
        anchors.fill: parent

        spacing: 2

        Image {
            source: xrTxtBtn.iconSource
            Layout.preferredWidth:  recSize * 0.07
            Layout.preferredHeight: recSize * 0.07
            Layout.alignment: Qt.AlignVCenter
            fillMode: Image.PreserveAspectFit
        }

        Item {
            Layout.preferredWidth: recSize * 0.3
            Layout.fillWidth: true
            Text {
                anchors.centerIn: parent
                text: xrTxtBtn.buttonText
                color: "white"
                font.pixelSize: recSize * 0.056
            }
        }
    }
}
