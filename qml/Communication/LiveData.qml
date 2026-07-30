import QtQuick 2.15
import QtQuick.Controls 2.12
import QtQuick.Layouts  1.12



// ----------------- Live Data 面板 ------------------
Rectangle {
    id: liveDataContent
    width:  liveDataSize
    height: liveDataSize * 0.8
    x: width * 0.3
    y: 2
    z: 9999

    color: "#f0f0f0"
    border.color: "#3498db"
    border.width: 1
    radius: 5
    visible: theme.liveDataVisible


    property int  currentCommPage: 0
    property bool bluetoothDrawOpen: false
    property int  liveDataSize:  theme.screenSize * 0.35
    property int  layoutHeight: liveDataSize * 0.1
    property var  targetPlot:   null
    property int  iconSize:     liveDataSize * 0.05
    property bool isShowDataPanel: false


    Connections {
        target: BleManager
        function onConnectedChanged(connected) {
            switchControl.isOn = connected
            readControl.isReading  = connected
            liveDataContent.isShowDataPanel = true
        }
    }


    // 拦截鼠标事件，防止点击穿透到地图
    MouseArea {
        anchors.fill: parent
        enabled: bluetoothDrawOpen
        preventStealing: true
    }

    Rectangle {
        anchors.fill: parent
        color: "#dbe3f2"
    }

    Behavior on anchors.rightMargin {
        NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
    }


    Rectangle {
        id: bluetoothTitleBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        height: iconSize * 1.5
        color: "#9fb6cd"

        RowLayout {
            anchors.fill: parent
            spacing: 1

            Repeater {
                model: [ qsTr("Bluetooth"), qsTr("WiFi"), qsTr("SerialPort") ]

                delegate: Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    color: currentCommPage === index ? "#ffffff" : "#9fb6cd"

                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        font.pixelSize: iconSize
                        font.bold: currentCommPage === index
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            currentCommPage = index
                            if(index === 0) {
                                BleManager.setBleLiveScanningVisible(true)
                            }
                            else if(index === 2) {
                                SerialPort.scanPorts()
                            }

                        }
                    }
                }
            }
        }
    }

    Item {
        id: bluetoothPage
        visible: currentCommPage === 0
        anchors.top: bluetoothTitleBar.bottom
        anchors.left:         parent.left
        anchors.right:        parent.right
        anchors.bottom:       parent.bottom
        anchors.margins: iconSize * 0.5
        ColumnLayout
        {
            anchors.fill: parent

            spacing: 10

            RowLayout
            {
                anchors.margins: 10
                spacing: 10

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.preferredWidth: 2
                    Layout.minimumWidth: 2

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 4

                        Rectangle {
                            id: keepBoatView
                            Layout.fillWidth: true
                            Layout.preferredHeight: layoutHeight
                            radius: layoutHeight * 0.2
                            color: "#f9f9fb"
                            border.color: "#b0b3b8"
                            border.width: 2

                            property bool checked: true

                            SequentialAnimation {
                               id: flashAnim11
                               running: false
                               loops: 1

                               ColorAnimation {
                                   target: keepBoatView
                                   property: "color"
                                   to: "#9ecbff"
                                   duration: 100
                               }
                               ColorAnimation {
                                   target: keepBoatView
                                   property: "color"
                                   to: "#d6e6ff"
                                   duration: 100
                               }
                            }

                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: iconSize * 0.5
                                anchors.rightMargin: iconSize * 0.5
                                spacing: iconSize * 0.5
                                anchors.verticalCenter: parent.verticalCenter

                                Rectangle {
                                    width: iconSize * 1.1
                                    height: iconSize * 1.1
                                    radius: 5
                                    border.color: "#b0b3b8"
                                    border.width: 1
                                    anchors.verticalCenter: parent.verticalCenter

                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: parent.width * 0.8
                                        height: parent.height * 0.8
                                        radius: parent.height * 0.4
                                        color: "#66E07A"
                                    }
                                }

                                Text {
                                    text: qsTr("Keep Boat in View")
                                    font.pixelSize: iconSize
                                    color: "black"
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true

                                onClicked: {
                                    flashAnim11.restart()
                                    if(switchControl.isOn) {
                                        core.location(1)
                                    }

                                }

                                onEntered: parent.color = "#d6e6ff"
                                onExited: parent.color = "#f9f9fb"
                            }
                        }


                        Rectangle {
                            id: showDataPanel
                            Layout.fillWidth: true
                            Layout.preferredHeight: layoutHeight
                            radius: layoutHeight * 0.2
                            color: "#f9f9fb"
                            border.color: "#b0b3b8"
                            border.width: 2

                            property bool checked: true

                            SequentialAnimation {
                               id: flashAnim22
                               running: false
                               loops: 1

                               ColorAnimation {
                                   target: showDataPanel
                                   property: "color"
                                   to: "#9ecbff"
                                   duration: 100
                               }
                               ColorAnimation {
                                   target: showDataPanel
                                   property: "color"
                                   to: "#d6e6ff"
                                   duration: 100
                               }
                            }

                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: iconSize * 0.5
                                anchors.rightMargin: iconSize * 0.5
                                spacing: iconSize * 0.6
                                anchors.verticalCenter: parent.verticalCenter

                                Rectangle {
                                    width: iconSize * 1.1
                                    height: iconSize * 1.1
                                    radius: 5
                                    border.color: "#b0b3b8"
                                    border.width: 1
                                    anchors.verticalCenter: parent.verticalCenter

                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: parent.width * 0.8
                                        height: parent.height * 0.8
                                        radius: parent.height * 0.4
                                        color: "#66E07A"
                                        visible: isShowDataPanel
                                    }
                                }

                                Text {
                                    text: qsTr("Show Data Panel")
                                    font.pixelSize: iconSize
                                    color: "black"
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true

                                onClicked: {
                                    flashAnim22.restart()
                                    isShowDataPanel = !isShowDataPanel
                                }

                                onEntered: parent.color = "#d6e6ff"
                                onExited:  parent.color = "#f9f9fb"
                            }
                        }

                    }
                }

                Item {
                    id: rootItem
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Rectangle {
                        id: centerRect
                        color: "transparent"
                        width: layoutHeight * 2
                        anchors.centerIn: parent

                        ColumnLayout {
                            id: mainCol
                            anchors.centerIn: parent
                            spacing: 20

                            Rectangle {
                                id: switchControl
                                width:  layoutHeight * 2.2
                                height: layoutHeight
                                radius: layoutHeight * 0.3
                                color:  hovered ? (switchControl.isOn ? "#36D85A" : "#D6E6FF")
                                                : (switchControl.isOn?  "#66E07A" : "#D0D0D2")
                                property bool isOn: false
                                property bool hovered: false

                                // 滑块
                                Rectangle {
                                    width:  layoutHeight * 0.9
                                    height: layoutHeight * 0.9
                                    radius: layoutHeight * 0.45
                                    anchors.verticalCenter: parent.verticalCenter
                                    x: switchControl.isOn ? parent.width-width-2 : 2
                                    color: "#FAFAFA"
                                    scale: mouse1Area2.pressed ? 0.9 : 1.0

                                    Behavior on x {
                                        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                                    }
                                    Behavior on scale {
                                        NumberAnimation { duration: 100 }
                                    }
                                }

                                Text {
                                    anchors {
                                        left: parent.left
                                        leftMargin: 10
                                        verticalCenter: parent.verticalCenter
                                    }
                                    text: qsTr("ON")
                                    font.pixelSize: iconSize * 0.8
                                    font.bold: true
                                    visible: switchControl.isOn
                                    Behavior on opacity { NumberAnimation { duration: 150 } }
                                }

                                Text {
                                    anchors {
                                        right: parent.right
                                        rightMargin: 10
                                        verticalCenter: parent.verticalCenter
                                    }
                                    text: qsTr("OFF")
                                    font.pixelSize: iconSize * 0.8
                                    font.bold: true
                                    visible: !switchControl.isOn
                                    Behavior on opacity { NumberAnimation { duration: 150 } }
                                }

                                MouseArea {
                                    id: mouse1Area2
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        if(switchControl.isOn) {
                                            switchControl.isOn = false;
                                            BleManager.operateBleOnOff(false)
                                        } else {
                                            BleManager.operateBleOnOff(true)
                                        }
                                    }
                                    onEntered: switchControl.hovered = true
                                    onExited:  switchControl.hovered = false
                                }

                                Behavior on color {
                                    ColorAnimation { duration: 200 }
                                }
                            }

                            Rectangle {
                                id: readControl
                                width:  layoutHeight * 2.2
                                height: layoutHeight
                                radius: layoutHeight * 0.3
                                color:  hovered ? (readControl.isReading ? "#36D85A" : "#D6E6FF")
                                                : (readControl.isReading?  "#66E07A" : "#D0D0D2")
                                property bool isReading: false
                                property bool hovered: false

                                // 滑块
                                Rectangle {
                                    width:  layoutHeight * 0.9
                                    height: layoutHeight * 0.9
                                    radius: layoutHeight * 0.45
                                    anchors.verticalCenter: parent.verticalCenter
                                    x: readControl.isReading ? parent.width-width-2 : 2
                                    color: "#FAFAFA"
                                    scale: mouse2Area2.pressed ? 0.9 : 1.0

                                    Behavior on x {
                                        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                                    }
                                    Behavior on scale {
                                        NumberAnimation { duration: 100 }
                                    }
                                }

                                Text {
                                    anchors {
                                        left: parent.left
                                        leftMargin: 5
                                        verticalCenter: parent.verticalCenter
                                    }
                                    text: qsTr("Read")
                                    font.pixelSize: iconSize * 0.8
                                    font.bold: true
                                    visible: readControl.isReading
                                    Behavior on opacity { NumberAnimation { duration: 150 } }
                                }

                                Text {
                                    anchors {
                                        right: parent.right
                                        rightMargin: 5
                                        verticalCenter: parent.verticalCenter
                                    }
                                    text: qsTr("Pause")
                                    font.pixelSize: iconSize * 0.8
                                    font.bold: true
                                    visible: !readControl.isReading
                                    Behavior on opacity { NumberAnimation { duration: 150 } }
                                }

                                MouseArea {
                                    id: mouse2Area2
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        readControl.isReading = !readControl.isReading
                                        BleManager.dataReading = readControl.isReading
                                    }
                                    onEntered: readControl.hovered = true
                                    onExited:  readControl.hovered = false
                                }

                                Behavior on color {
                                    ColorAnimation { duration: 200 }
                                }
                            }
                        }

                    }
                }

            }


            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 3; color: "#7f8c8d" }


            ColumnLayout
            {
                anchors.margins: 15
                spacing: 1

                Text {
                    anchors.margins: 5
                    font.pixelSize: iconSize * 0.9
                    text: qsTr("Toslon BLE Devices List:")
                    verticalAlignment: Text.AlignVCenter
                }

                // 设备列表
                GroupBox {
                    Layout.fillWidth: true
                    Layout.preferredHeight: layoutHeight * 3
                    background: Rectangle {
                        color: "#F5F5F5"
                        radius: layoutHeight * 0.3
                        border.color: "#bdc3c7"
                    }

                    ListView {
                        id: deviceList
                        anchors.fill: parent
                        anchors.margins: 5
                        model: BleManager ? BleManager.devices : ""
                        clip: true

                        delegate: Rectangle
                        {
                            width: deviceList.width
                            height: layoutHeight * 0.8
                            radius: 4

                            readonly property bool noDevices: (modelData === qsTr("No Devices Found"))

                            border.width: noDevices ? 1 : (switchControl.isOn ? 2 : 1)
                            border.color: noDevices ? "#ecf0f1" : (switchControl.isOn ? "#3498db" : "#ecf0f1")
                            color: noDevices ? "white" : (switchControl.isOn ? "#d6eaf8" : (mouseArea2.containsMouse ? "#d6e6ff" : "white"))

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 15
                                anchors.rightMargin: 15

                                Text {
                                    Layout.fillWidth: true
                                    text: modelData
                                    color: "#2c3e50"
                                    font.pixelSize: iconSize * 0.9
                                    verticalAlignment: Text.AlignVCenter
                                }
                                Text {
                                    visible: !noDevices
                                    text: switchControl.isOn ? qsTr("Connected") : qsTr("Disconnected")
                                    color: switchControl.isOn ? "#36D85A" : "#7f8c8d"
                                    font.pixelSize: iconSize * 0.6
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }

                            MouseArea {
                                id: mouseArea2
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked:{
                                    if(!noDevices && BleManager){
                                        BleManager.connectToDevice(index)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    }




    Item {
        id: wifiPage

        visible: currentCommPage === 1

        anchors {
            top: bluetoothTitleBar.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: iconSize
            spacing: iconSize * 0.8

            Text {
                text: qsTr("Tip: Ensure connected to WiFi \"FishFind\"")
                font.pixelSize: iconSize * 0.9
            }

            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: qsTr("Local IP:")
                    font.pixelSize: iconSize
                    font.bold: true
                    Layout.preferredWidth: iconSize * 6
                }

                TextField {
                    id: localIpEdit
                    Layout.fillWidth: true
                    placeholderText: "127.0.0.1"
                    readOnly: true
                    selectByMouse: false
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: qsTr("Remote IP:")
                    font.pixelSize: iconSize
                    font.bold: true
                    Layout.preferredWidth: iconSize * 6
                }

                TextField {
                    id: targetIpEdit
                    Layout.fillWidth: true
                    // text:
                    placeholderText: ".  .  .  ."
                    selectByMouse: true

                    onTextChanged: UdpManager.remoteIp = text
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: qsTr("Port:")
                    font.pixelSize: iconSize
                    font.bold: true
                    Layout.preferredWidth: iconSize * 6
                }

                TextField {
                    id: targetPortEdit
                    Layout.fillWidth: true
                    validator: IntValidator {
                        bottom: 1
                        top: 65535
                    }
                    onTextChanged: UdpManager.remotePort = text
                }

            }


            RowLayout {
                id: udpControl
                Layout.alignment: Qt.AlignHCenter
                spacing: iconSize * 3


                Rectangle {
                    id: onOffControl
                    width:  layoutHeight * 2.2
                    height: layoutHeight
                    radius: layoutHeight * 0.3
                    color:  hovered ? (onOffControl.isOn ? "#36D85A" : "#D6E6FF")
                                    : (onOffControl.isOn?  "#66E07A" : "#D0D0D2")
                    property bool isOn: false
                    property bool hovered: false

                    // 滑块wifi
                    Rectangle {
                        width:  layoutHeight * 0.9
                        height: layoutHeight * 0.9
                        radius: layoutHeight * 0.45
                        anchors.verticalCenter: parent.verticalCenter
                        x: onOffControl.isOn ? parent.width-width-2 : 2
                        color: "#FAFAFA"

                        Behavior on x {
                            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                        }

                    }

                    Text {
                        anchors {
                            left: parent.left
                            leftMargin: 10
                            verticalCenter: parent.verticalCenter
                        }
                        text: qsTr("ON")
                        font.pixelSize: iconSize * 0.8
                        font.bold: true
                        visible: onOffControl.isOn
                        Behavior on opacity { NumberAnimation { duration: 150 } }
                    }

                    Text {
                        anchors {
                            right: parent.right
                            rightMargin: 10
                            verticalCenter: parent.verticalCenter
                        }
                        text: qsTr("OFF")
                        font.pixelSize: iconSize * 0.8
                        font.bold: true
                        visible: !onOffControl.isOn
                        Behavior on opacity { NumberAnimation { duration: 150 } }
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            UdpManager.openUdp(!onOffControl.isOn)
                        }
                        onEntered: onOffControl.hovered = true
                        onExited:  onOffControl.hovered = false
                    }

                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }


                Rectangle {
                    id: readControl2
                    width:  layoutHeight * 2.2
                    height: layoutHeight
                    radius: layoutHeight * 0.3
                    color:  hovered ? (readControl2.isReading ? "#36D85A" : "#D6E6FF")
                                    : (readControl2.isReading?  "#66E07A" : "#D0D0D2")
                    property bool isReading: false
                    property bool hovered:   false

                    // 滑块wifi
                    Rectangle {
                        width:  layoutHeight * 0.9
                        height: layoutHeight * 0.9
                        radius: layoutHeight * 0.45
                        anchors.verticalCenter: parent.verticalCenter
                        x: readControl2.isReading ? parent.width-width-2 : 2
                        color: "#FAFAFA"

                        Behavior on x {
                            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                        }
                    }

                    Text {
                        anchors {
                            left: parent.left
                            leftMargin: 5
                            verticalCenter: parent.verticalCenter
                        }
                        text: qsTr("Read")
                        font.pixelSize: iconSize * 0.8
                        font.bold: true
                        visible: readControl2.isReading
                        Behavior on opacity { NumberAnimation { duration: 150 } }
                    }

                    Text {
                        anchors {
                            right: parent.right
                            rightMargin: 5
                            verticalCenter: parent.verticalCenter
                        }
                        text: qsTr("Pause")
                        font.pixelSize: iconSize * 0.8
                        font.bold: true
                        visible: !readControl2.isReading
                        Behavior on opacity { NumberAnimation { duration: 150 } }
                    }

                    MouseArea {
                        id: readPauseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            readControl2.isReading = !readControl2.isReading
                            UdpManager.setDataReading(readControl2.isReading)
                        }
                        onEntered: readControl2.hovered = true
                        onExited:  readControl2.hovered = false
                    }

                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }
            }


        }
    }






    Item {
        id: serialPortPage
        visible: currentCommPage === 2
        anchors.top: bluetoothTitleBar.bottom
        anchors.left:    parent.left
        anchors.right:   parent.right
        anchors.bottom:  parent.bottom
        anchors.margins: iconSize * 0.5
        ColumnLayout
        {
            anchors.fill: parent
            spacing: 10

            RowLayout {
                id: rowSettings
                Layout.fillWidth: true
                spacing: iconSize
                RowLayout {
                    id: rowPort
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        text: qsTr("Port")
                        color: "black"
                        font.pixelSize: iconSize
                        Layout.minimumWidth: iconSize
                    }
                    ComboBox {
                        id: portCombo
                        model: SerialPort.availablePorts
                        Layout.preferredWidth: iconSize * 6
                        font.pixelSize: iconSize
                        contentItem: Text {
                            text: portCombo.displayText
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            color: "#333333"
                        }

                        delegate: ItemDelegate {
                            width: portCombo.width
                            height: portCombo.height
                            contentItem: Text {
                                text: modelData
                                color: "#333333"
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                            }
                            highlighted: portCombo.highlightedIndex === index
                        }
                    }

                }

                RowLayout {
                    id: rowBaud
                    Layout.fillWidth: true
                    spacing: 2

                    Label {
                        text: qsTr("Baud Rate")
                        color: "black"
                        font.pixelSize: iconSize
                        Layout.minimumWidth: iconSize * 2
                    }

                    ComboBox {
                        id: baudCombo
                        model: [ "230400", "19200", "921600", "57600", "115200", "38400"]
                        font.pixelSize: iconSize
                        Layout.preferredWidth: iconSize * 6
                        contentItem: Text {
                            text: baudCombo.displayText
                            color: "#333333"
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                        }

                        delegate: ItemDelegate {
                            width: baudCombo.width
                            height: baudCombo.height
                            contentItem: Text {
                                text: modelData
                                color: "#333333"
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                            }
                            highlighted: baudCombo.highlightedIndex === index
                        }
                    }
                }
            }


            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: layoutHeight * 2

                Rectangle {
                    id: switchControl2
                    width:  layoutHeight * 2.2
                    height: layoutHeight
                    radius: layoutHeight * 0.3
                    color:  hovered ? (SerialPort.connected ? "#36D85A" : "#D6E6FF")
                                    : (SerialPort.connected?  "#66E07A" : "#D0D0D2")
                    property bool hovered: false

                    // 滑块serialPort
                    Rectangle {
                        width:  layoutHeight * 0.9
                        height: layoutHeight * 0.9
                        radius: layoutHeight * 0.45
                        anchors.verticalCenter: parent.verticalCenter
                        x: SerialPort.connected
                        color: "#FAFAFA"
                        scale: mouse1Area22.pressed ? 0.9 : 1.0

                        Behavior on x {
                            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                        }
                        Behavior on scale {
                            NumberAnimation { duration: 100 }
                        }
                    }

                    Text {
                        anchors {
                            left: parent.left
                            leftMargin: 10
                            verticalCenter: parent.verticalCenter
                        }
                        text: qsTr("ON")
                        font.pixelSize: iconSize * 0.8
                        font.bold: true
                        visible: SerialPort.connected
                        Behavior on opacity { NumberAnimation { duration: 150 } }
                    }

                    Text {
                        anchors {
                            right: parent.right
                            rightMargin: 10
                            verticalCenter: parent.verticalCenter
                        }
                        text: qsTr("OFF")
                        font.pixelSize: iconSize * 0.8
                        font.bold: true
                        visible: !SerialPort.connected
                        Behavior on opacity { NumberAnimation { duration: 150 } }
                    }

                    MouseArea {
                        id: mouse1Area22
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            SerialPort.toggleConnection(portCombo.currentText, baudCombo.currentText)
                        }
                        onEntered: switchControl2.hovered = true
                        onExited:  switchControl2.hovered = false
                    }

                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }

                Rectangle {
                    id: readControl22
                    width:  layoutHeight * 2.2
                    height: layoutHeight
                    radius: layoutHeight * 0.3
                    color:  hovered ? (readControl22.isReading ? "#36D85A" : "#D6E6FF")
                                    : (readControl22.isReading?  "#66E07A" : "#D0D0D2")
                    property bool isReading: false
                    property bool hovered: false

                    // 滑块serialPort
                    Rectangle {
                        width:  layoutHeight * 0.9
                        height: layoutHeight * 0.9
                        radius: layoutHeight * 0.45
                        anchors.verticalCenter: parent.verticalCenter
                        x: readControl22.isReading ? parent.width-width-2 : 2
                        color: "#FAFAFA"
                        scale: mouse2Area22.pressed ? 0.9 : 1.0

                        Behavior on x {
                            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                        }
                        Behavior on scale {
                            NumberAnimation { duration: 100 }
                        }
                    }

                    Text {
                        anchors {
                            left: parent.left
                            leftMargin: 5
                            verticalCenter: parent.verticalCenter
                        }
                        text: qsTr("Read")
                        font.pixelSize: iconSize * 0.8
                        font.bold: true
                        visible: readControl22.isReading
                        Behavior on opacity { NumberAnimation { duration: 150 } }
                    }

                    Text {
                        anchors {
                            right: parent.right
                            rightMargin: 5
                            verticalCenter: parent.verticalCenter
                        }
                        text: qsTr("Pause")
                        font.pixelSize: iconSize * 0.8
                        font.bold: true
                        visible: !readControl22.isReading
                        Behavior on opacity { NumberAnimation { duration: 150 } }
                    }

                    MouseArea {
                        id: mouse2Area22
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            readControl2.isReading = !readControl2.isReading
                            SerialPort.dataReading = readControl2.isReading
                        }
                        onEntered: readControl2.hovered = true
                        onExited:  readControl2.hovered = false
                    }

                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }


            }


            Rectangle {
                id: keepBoatView2
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: layoutHeight * 6.4
                Layout.preferredHeight:layoutHeight
                radius: layoutHeight * 0.2
                color: "#f9f9fb"
                border.color: "#b0b3b8"
                border.width: 2

                property bool checked: true

                SequentialAnimation {
                   id: flashAnim112
                   running: false
                   loops: 1

                   ColorAnimation {
                       target: keepBoatView
                       property: "color"
                       to: "#9ecbff"
                       duration: 100
                   }
                   ColorAnimation {
                       target: keepBoatView
                       property: "color"
                       to: "#d6e6ff"
                       duration: 100
                   }
                }

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: iconSize * 0.5
                    anchors.rightMargin: iconSize * 0.5
                    spacing: iconSize * 0.5
                    anchors.verticalCenter: parent.verticalCenter

                    Rectangle {
                        width: iconSize * 1.1
                        height: iconSize * 1.1
                        radius: 5
                        border.color: "#b0b3b8"
                        border.width: 1
                        anchors.verticalCenter: parent.verticalCenter

                        Rectangle {
                            anchors.centerIn: parent
                            width: parent.width * 0.8
                            height: parent.height * 0.8
                            radius: parent.height * 0.4
                            color: "#66E07A"
                        }
                    }

                    Text {
                        text: qsTr("Keep Boat in View")
                        font.pixelSize: iconSize
                        color: "black"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        flashAnim11.restart()
                        if(SerialPort.connected) {
                            core.location(1)
                        }
                    }
                    onEntered: parent.color = "#d6e6ff"
                    onExited: parent.color = "#f9f9fb"
                }
            }

            Rectangle {
                id: showDataPanel2
                // Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: layoutHeight * 6.4
                Layout.preferredHeight: layoutHeight
                radius: layoutHeight * 0.2
                color: "#f9f9fb"
                border.color: "#b0b3b8"
                border.width: 2

                property bool checked: true

                SequentialAnimation {
                   id: flashAnim222
                   running: false
                   loops: 1

                   ColorAnimation {
                       target: showDataPanel
                       property: "color"
                       to: "#9ecbff"
                       duration: 100
                   }
                   ColorAnimation {
                       target: showDataPanel
                       property: "color"
                       to: "#d6e6ff"
                       duration: 100
                   }
                }

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: iconSize * 0.5
                    anchors.rightMargin: iconSize * 0.5
                    spacing: iconSize * 0.6
                    anchors.verticalCenter: parent.verticalCenter

                    Rectangle {
                        width: iconSize * 1.1
                        height: iconSize * 1.1
                        radius: 5
                        border.color: "#b0b3b8"
                        border.width: 1
                        anchors.verticalCenter: parent.verticalCenter

                        Rectangle {
                            anchors.centerIn: parent
                            width: parent.width * 0.8
                            height: parent.height * 0.8
                            radius: parent.height * 0.4
                            color: "#66E07A"
                            visible: isShowDataPanel
                        }
                    }

                    Text {
                        text: qsTr("Show Data Panel")
                        font.pixelSize: iconSize
                        color: "black"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked: {
                        flashAnim22.restart()
                        isShowDataPanel = !isShowDataPanel
                    }

                    onEntered: parent.color = "#d6e6ff"
                    onExited:  parent.color = "#f9f9fb"
                }
            }



        }

    }



}
