import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


import  "../"
import AppXr 1.0
import QmlCommon 1.0


XRRectangle {
    id: plotCheckRec
    width: plotIconSize * 24
    height: plotIconSize * 9


    property var  targetPlot:   null

    property int  plotIconSize: theme.iconSize * 1.5

    property bool  currentFrameChecked:  currentFrame.checked
    property bool  bottomLineChecked:    bottomLine.checked
    property bool  deleteFrameChecked:   deleteFrame.checked




    function deleteFrameFunc(globalX, globalY) {
        if(targetPlot) {
            var ok = targetPlot.onDoubleClick(globalX, globalY)
            if (ok) {
                var startIdx = targetPlot.getDeleteStartIdx()
                var endIdx   = targetPlot.getDeleteEndIdx()
                if (startIdx >= 0) {
                    fromValue.text     = startIdx
                    fromLonValue.text  = targetPlot.fromLonStr
                    fromLatiValue.text = targetPlot.fromLatiStr
                }
                if (endIdx >= 0) {
                    toValue.text     = endIdx
                    toLonValue.text  = targetPlot.toLonStr
                    toLatiValue.text = targetPlot.toLatiStr
                }
            }
        }

    }


    ColumnLayout {
        id: plotSettings
        Layout.alignment: Qt.AlignHCenter
        spacing: plotIconSize * 0.5

        RowLayout {
            Layout.fillWidth:  true
            spacing: plotIconSize * 2

            CCheck {
                id: echogramVisible
                checked: true
                text: qsTr("Color Scheme")
                height: plotIconSize
                onCheckedChanged: plotEchogramVisible(checked)
                Component.onCompleted: plotEchogramVisible(checked)
            }

            XRColorScheme {
                id: echoTheme
                currentIndex: 0
                onCurrentIndexChanged: {
                    plot.setColorScheme(currentIndex)
                }

                Component.onCompleted: {
                    plot.setColorScheme(currentIndex)
                }

            }

        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#555555"
        }

        RowLayout {
            Layout.fillWidth: true

            CText {
                text: qsTr("upper(m)")
                font.pixelSize: plotIconSize
                horizontalAlignment: Text.AlignRight
                Layout.alignment: Qt.AlignVCenter
            }
            TextField {
               id: upperMin
               Layout.preferredWidth: plotIconSize * 3
               horizontalAlignment: TextInput.AlignHCenter
               font.pixelSize: plotIconSize
               selectByMouse: true
               validator: IntValidator { bottom: 0; top: 511;}
               text: (plot.minUpRng / 100).toFixed(0)
               onEditingFinished: {
                    let value = parseInt(text, 10)
                    if(isNaN(value) || value < 0) { value = 0 }
                    if(value > 511) { value = 511 }
                    text = value.toString()
                }

                Connections {
                    target: plot
                    function onMinUpRngChanged() {
                        // if (!upperMin.activeFocus) {
                            upperMin.text = (plot.minUpRng / 100).toFixed(0)
                        // }
                    }
                }
            }

            Item {
                Layout.preferredWidth: plotIconSize * 0.75
            }

            CText {
                text: qsTr("lower(m)")
                font.pixelSize: plotIconSize
                horizontalAlignment: Text.AlignRight
                Layout.alignment: Qt.AlignVCenter
            }
            TextField {
                id: lowerMax
                Layout.preferredWidth: plotIconSize * 3
                horizontalAlignment: TextInput.AlignHCenter
                font.pixelSize: plotIconSize
                selectByMouse: true
                validator: IntValidator { bottom: 1; top: 512;}
                text: (plot.maxLoRng / 100).toFixed(0)
                onEditingFinished: {
                    let value = parseInt(text, 10)
                    if(isNaN(value) || value < 1) { value = 1 }
                    if(value > 512) { value = 512 }
                    text = value.toString()
                }

                Connections {
                    target: plot
                    function onMaxLoRngChanged() {
                        // if (!lowerMax.activeFocus) {
                            lowerMax.text = (plot.maxLoRng / 100).toFixed(0)
                        // }
                    }
                }
            }

            Item {
                Layout.preferredWidth: plotIconSize * 0.75
            }

            Rectangle {
                id: applyBtn
                width: plotIconSize * 4
                height: plotIconSize * 1.2
                radius: 4
                color: mouseArea.pressed ? "#888888" : "#555555"
                border.color: "#aaaaaa"

                Text {
                    anchors.centerIn: parent
                    text: qsTr("Apply")
                    color: "white"
                    font.pixelSize: plotIconSize
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    onClicked: {
                        let upperVal = parseInt(upperMin.text, 10)
                        let lowerVal = parseInt(lowerMax.text, 10)
                        if(isNaN(upperVal)) { upperVal = 0 }
                        if(isNaN(lowerVal)) { lowerVal = 1 }
                        if(upperVal < 0)    { upperVal = 0 }
                        if(upperVal > 511)  { upperVal = 511 }
                        if(lowerVal < 1)    { lowerVal = 1 }
                        if(lowerVal > 512)  { lowerVal = 512 }

                        if(upperVal >= lowerVal) {
                            upperVal = lowerVal - 1
                        }
                        plot.resetUpLoRng(upperVal, lowerVal)
                    }
                }
            }

        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#555555"
        }


        RowLayout {
            Layout.fillWidth:  true
            spacing: plotIconSize * 2

            XRSlider {
                title: "Sensitivity"
                Layout.preferredWidth: plotIconSize * 7
                Layout.alignment: Qt.AlignVCenter
                sliderLen: plotIconSize * 5
                from: 1
                to: 9
                value: 7
                onValueChanged: {
                    plot.setSensitivity(value)
                }
            }

            CCheck {
                id: bottomLine
                checked: false
                text: qsTr("Bottom Line")
                height: plotIconSize
                onCheckedChanged: {
                    bottomLineChecked = !bottomLineChecked
                    plot.setBottomLineVisible(bottomLineChecked)
                }
            }

        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "#555555"
        }

        RowLayout {
            Layout.fillWidth: true

            CCheck {
                id: currentFrame
                checked: false
                text: qsTr("Current Frame")
                height: plotIconSize
                onCheckedChanged: {
                    currentFrameChecked = !currentFrameChecked
                    if(!currentFrameChecked) {
                        plot.plotMousePosition(-1, -1)
                    }
                }
            }



            Rectangle {
                id: marksDrawer
                x: addMarks.x
                y: addMarks.y - height
                width: plot.width * 0.25
                height: 0
                color: "#dbe3f2"
                border.color: "#a8b3c5"
                border.width: 1
                radius: markIconSize * 0.2
                clip: true

                property bool opened: false
                property int  markIconSize: plotIconSize * 0.72
                property int  dist0Time1: 0
                property int  distInterval: 10
                property int  timeInterval: 60
                property bool isFrameVisible:  true;
                property bool isTimeVisible:   true;
                property bool isDepthVisible: true;
                property bool isCoordinateVisible: true;

                Behavior on height {
                    NumberAnimation {
                        duration: 250
                        easing.type: Easing.OutCubic
                    }
                }

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 2

                    Rectangle {
                        Layout.alignment: Qt.AlignLeft
                        height: marksDrawer.markIconSize * 1.5

                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 3
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Marks Format")
                            font.pixelSize: marksDrawer.markIconSize
                            font.bold: true
                        }
                    }

                    Rectangle {
                        Layout.preferredWidth: plot.width * 0.24
                        Layout.preferredHeight: marksDrawer.markIconSize * 5
                        Layout.alignment: Qt.AlignHCenter
                        Layout.topMargin: 4
                        Layout.bottomMargin: 4

                        border.color: "#7f8fa6"
                        border.width: 1
                        radius: marksDrawer.markIconSize * 0.2

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 5
                            spacing: 2

                            Rectangle {
                                id: frame
                                Layout.fillWidth: true
                                Layout.preferredHeight: marksDrawer.markIconSize
                                color: "#d6e6ff"

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: marksDrawer.markIconSize * 0.5
                                    anchors.rightMargin: marksDrawer.markIconSize * 0.5
                                    spacing: marksDrawer.markIconSize * 0.5
                                    anchors.verticalCenter: parent.verticalCenter

                                    Rectangle {
                                        width:  marksDrawer.markIconSize
                                        height: marksDrawer.markIconSize
                                        radius: 5
                                        border.color: "#b0b3b8"
                                        border.width: 1
                                        anchors.verticalCenter: parent.verticalCenter

                                        Image {
                                            source: "qrc:/XR/check.svg"
                                            width: parent.width * 0.9
                                            height: parent.width * 0.9
                                            visible: marksDrawer.isFrameVisible
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            anchors.rightMargin: 2
                                        }
                                    }

                                    Text {
                                        text: qsTr("Frame")
                                        font.pixelSize: marksDrawer.markIconSize
                                        color: "black"
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onPressed: marksDrawer.isFrameVisible = !marksDrawer.isFrameVisible
                                }
                            }


                            Rectangle {
                                id: time
                                Layout.fillWidth: true
                                Layout.preferredHeight: marksDrawer.markIconSize
                                color: "#d6e6ff"

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: marksDrawer.markIconSize * 0.5
                                    anchors.rightMargin: marksDrawer.markIconSize * 0.5
                                    spacing: marksDrawer.markIconSize * 0.5
                                    anchors.verticalCenter: parent.verticalCenter

                                    Rectangle {
                                        width:  marksDrawer.markIconSize
                                        height: marksDrawer.markIconSize
                                        radius: 5
                                        border.color: "#b0b3b8"
                                        border.width: 1
                                        anchors.verticalCenter: parent.verticalCenter

                                        Image {
                                            source: "qrc:/XR/check.svg"
                                            visible: marksDrawer.isTimeVisible
                                            width: parent.width * 0.9
                                            height: parent.width * 0.9
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            anchors.rightMargin: 2
                                        }
                                    }
                                    Text {
                                        text: qsTr("Time")
                                        font.pixelSize: marksDrawer.markIconSize
                                        color: "black"
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onPressed: marksDrawer.isTimeVisible = !marksDrawer.isTimeVisible
                                }
                            }



                            Rectangle {
                                id: depth
                                Layout.fillWidth: true
                                Layout.preferredHeight: marksDrawer.markIconSize
                                color: "#d6e6ff"

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: marksDrawer.markIconSize * 0.5
                                    anchors.rightMargin: marksDrawer.markIconSize * 0.5
                                    spacing: marksDrawer.markIconSize * 0.5
                                    anchors.verticalCenter: parent.verticalCenter

                                    Rectangle {
                                        width:  marksDrawer.markIconSize
                                        height: marksDrawer.markIconSize
                                        radius: 5
                                        border.color: "#b0b3b8"
                                        border.width: 1
                                        anchors.verticalCenter: parent.verticalCenter

                                        Image {
                                            source: "qrc:/XR/check.svg"
                                            visible: marksDrawer.isDepthVisible
                                            width: parent.width * 0.9
                                            height: parent.width * 0.9
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            anchors.rightMargin: 2
                                        }
                                    }

                                    Text {
                                        text: qsTr("Depth")
                                        font.pixelSize: marksDrawer.markIconSize
                                        color: "black"
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onPressed: marksDrawer.isDepthVisible = !marksDrawer.isDepthVisible

                                }
                            }

                            Rectangle {
                                id: coordinate
                                Layout.fillWidth: true
                                Layout.preferredHeight: marksDrawer.markIconSize
                                color: "#d6e6ff"

                                Row {
                                    anchors.fill: parent
                                    anchors.leftMargin: marksDrawer.markIconSize * 0.5
                                    anchors.rightMargin: marksDrawer.markIconSize * 0.5
                                    spacing: marksDrawer.markIconSize * 0.5
                                    anchors.verticalCenter: parent.verticalCenter

                                    Rectangle {
                                        width:  marksDrawer.markIconSize
                                        height: marksDrawer.markIconSize
                                        radius: 5
                                        border.color: "#b0b3b8"
                                        border.width: 1
                                        anchors.verticalCenter: parent.verticalCenter

                                        Image {
                                            source: "qrc:/XR/check.svg"
                                            visible: marksDrawer.isCoordinateVisible
                                            width: parent.width * 0.9
                                            height: parent.width * 0.9
                                            anchors.verticalCenter: parent.verticalCenter
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            anchors.rightMargin: 2
                                        }
                                    }


                                    Text {
                                        text: qsTr("Coordinate")
                                        font.pixelSize: marksDrawer.markIconSize
                                        color: "black"
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onPressed: marksDrawer.isCoordinateVisible = !marksDrawer.isCoordinateVisible
                                }
                            }

                        }

                    }


                    Rectangle {
                        id: markInterval
                        Layout.alignment: Qt.AlignLeft
                        height: marksDrawer.markIconSize * 1.5
                        color: "#3498db"

                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 3
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Marks Interval")
                            font.pixelSize: marksDrawer.markIconSize
                            font.bold: true
                        }
                    }

                    Rectangle {
                        Layout.preferredWidth: plot.width * 0.24
                        Layout.preferredHeight: marksDrawer.markIconSize * 3
                        Layout.alignment: Qt.AlignHCenter
                        Layout.bottomMargin: 5

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 2

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: marksDrawer.markIconSize * 0.2

                                Rectangle {
                                    Layout.preferredWidth: marksDrawer.markIconSize
                                    Layout.preferredHeight: marksDrawer.markIconSize
                                    radius: 5
                                    border.color: "#b0b3b8"
                                    border.width: 1
                                    Layout.alignment: Qt.AlignVCenter
                                    Image {
                                        source: "qrc:/XR/check.svg"
                                        width: parent.width * 0.9
                                        height: parent.width * 0.9
                                        visible: marksDrawer.dist0Time1 === 0
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.rightMargin: 2
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: marksDrawer.dist0Time1 = 0
                                    }
                                }

                                Text {
                                    text: qsTr("Distance")
                                    font.pixelSize: marksDrawer.markIconSize * 0.8
                                    Layout.fillWidth: true
                                    verticalAlignment: Text.AlignVCenter
                                }

                                TextField {
                                    id: distanceValue
                                    text: "10"
                                    font.pixelSize:         marksDrawer.markIconSize * 0.9
                                    Layout.preferredWidth:  marksDrawer.markIconSize * 3
                                    Layout.preferredHeight: marksDrawer.markIconSize * 1.2
                                    horizontalAlignment: TextInput.AlignHCenter
                                    topPadding: 0
                                    bottomPadding: 0
                                    selectByMouse: true
                                }

                                ComboBox {
                                    id: distanceUnitCombo
                                    model:[qsTr("m"), qsTr("km")]
                                    currentIndex: 0
                                    Layout.preferredWidth:  marksDrawer.markIconSize * 2.8
                                    Layout.preferredHeight: marksDrawer.markIconSize * 1.2
                                    font.pixelSize:         marksDrawer.markIconSize * 0.8

                                    contentItem: Text {
                                        text: distanceUnitCombo.displayText
                                        font.pixelSize: marksDrawer.markIconSize * 0.8
                                        color: "#333333"
                                    }

                                    delegate: ItemDelegate {
                                        width: distanceUnitCombo.width
                                        height: distanceUnitCombo.height
                                        contentItem: Text {
                                            text: modelData
                                            color: "#333333"
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                        }
                                        highlighted: distanceUnitCombo.highlightedIndex === index
                                    }
                                }
                            }

                            RowLayout {
                                Layout.fillWidth:true
                                spacing: marksDrawer.markIconSize * 0.2

                                Rectangle {
                                    Layout.preferredWidth: marksDrawer.markIconSize
                                    Layout.preferredHeight: marksDrawer.markIconSize
                                    radius: 5
                                    border.color: "#b0b3b8"
                                    border.width: 1
                                    Layout.alignment: Qt.AlignVCenter
                                    Image {
                                        source: "qrc:/XR/check.svg"
                                        width: parent.width * 0.9
                                        height: parent.width * 0.9
                                        visible: marksDrawer.dist0Time1 === 1
                                        anchors.verticalCenter: parent.verticalCenter
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        anchors.rightMargin: 2
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: marksDrawer.dist0Time1 = 1
                                    }
                                }

                                Text {
                                    text: qsTr("Time")
                                    font.pixelSize: marksDrawer.markIconSize * 0.8
                                    color: "black"
                                    Layout.fillWidth: true
                                    verticalAlignment: Text.AlignVCenter
                                }

                                TextField {
                                    id: timeValue
                                    text: "60"
                                    font.pixelSize:         marksDrawer.markIconSize * 0.9
                                    Layout.preferredWidth:  marksDrawer.markIconSize * 2.8
                                    Layout.preferredHeight: marksDrawer.markIconSize * 1.2
                                    horizontalAlignment: TextInput.AlignHCenter
                                    topPadding: 0
                                    bottomPadding: 0
                                    selectByMouse: true
                                }

                                ComboBox {
                                    id: timeUnitCombo
                                    model: [qsTr("Sec"), qsTr("Min")]
                                    currentIndex: 0
                                    Layout.preferredWidth:  marksDrawer.markIconSize * 2.8
                                    Layout.preferredHeight: marksDrawer.markIconSize * 1.2
                                    font.pixelSize: marksDrawer.markIconSize * 0.8

                                    contentItem: Text {
                                        text: timeUnitCombo.displayText
                                        font.pixelSize: marksDrawer.markIconSize * 0.8
                                        color: "#333333"
                                    }

                                    delegate: ItemDelegate {
                                        width: timeUnitCombo.width
                                        height: timeUnitCombo.height
                                        contentItem: Text {
                                            text: modelData
                                            color: "#333333"
                                            verticalAlignment: Text.AlignVCenter
                                            horizontalAlignment: Text.AlignHCenter
                                        }
                                        highlighted: timeUnitCombo.highlightedIndex === index
                                    }
                                }
                            }

                        }

                    }

                    Button {
                        text: qsTr("OK")
                        font.pixelSize: marksDrawer.markIconSize
                        Layout.preferredWidth:  marksDrawer.markIconSize * 3.6
                        Layout.preferredHeight: marksDrawer.markIconSize * 1.5
                        Layout.alignment: Qt.AlignHCenter
                        palette.button: "#b9c6db"
                        onClicked: {
                            marksDrawer.distInterval = parseInt(distanceValue.text)
                            if (distanceUnitCombo.currentIndex === 1) marksDrawer.distInterval *= 1000  // km→m
                            marksDrawer.timeInterval = parseInt(timeValue.text)
                            if (timeUnitCombo.currentIndex === 1) marksDrawer.timeInterval *= 60  // min→sec
                            plot.setMarkDistTimeVisible(addMarks.checked, marksDrawer.dist0Time1, marksDrawer.distInterval,
                                marksDrawer.timeInterval, marksDrawer.isFrameVisible,marksDrawer.isTimeVisible,
                                marksDrawer.isDepthVisible, marksDrawer.isCoordinateVisible)
                        }
                    }

                }

                function open() {
                    height = plotIconSize * 11
                    opened = true
                }

                function close() {
                    height = 0
                    opened = false
                }
            }


            ExpandCheckBox {
                id: addMarks
                Layout.preferredWidth: plot.width * 0.2
                text: qsTr("Add Marks")
                onTextClicked: {
                    if(addMarks.isTextClicked) {
                        marksDrawer.open()
                    }
                    else {
                        marksDrawer.close()
                    }
                }
                onCheckClicked: {
                    marksDrawer.distInterval = parseInt(distanceValue.text)
                    if (distanceUnitCombo.currentIndex === 1) marksDrawer.distInterval *= 1000
                    marksDrawer.timeInterval = parseInt(timeValue.text)
                    if (timeUnitCombo.currentIndex === 1) marksDrawer.timeInterval *= 60
                    plot.setMarkDistTimeVisible(addMarks.checked, marksDrawer.dist0Time1, marksDrawer.distInterval,
                             marksDrawer.timeInterval, marksDrawer.isFrameVisible,marksDrawer.isTimeVisible,
                             marksDrawer.isDepthVisible, marksDrawer.isCoordinateVisible )
                }
            }


            Rectangle {
                id: deleteFramePanel
                x: deleteFrame.x
                y: deleteFrame.y - height
                width: plot.width * 0.24
                height: 0
                color: "#dbe3f2"
                border.color: "#a8b3c5"
                border.width: 1
                radius: deleteIconSize * 0.2
                clip: true

                property bool opened: false
                property int  deleteIconSize: plotIconSize * 0.7

                Behavior on height {
                    NumberAnimation {
                        duration: 250
                        easing.type: Easing.OutCubic
                    }
                }

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 2

                    Rectangle {
                        Layout.alignment: Qt.AlignLeft
                        height: deleteFramePanel.deleteIconSize * 1.5

                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 3
                            anchors.verticalCenter: parent.verticalCenter
                            text: qsTr("Delete Frame")
                            font.bold: true
                            font.pixelSize: deleteFramePanel.deleteIconSize
                        }
                    }


                    Rectangle {
                        id: content
                        Layout.preferredWidth: plot.width * 0.24
                        Layout.preferredHeight: deleteFramePanel.height * 0.8
                        Layout.alignment: Qt.AlignHCenter
                        Layout.bottomMargin: 5
                        Layout.leftMargin: 4
                        Layout.rightMargin: 4
                        color: "#dbe3f2"

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 2

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: deleteFramePanel.deleteIconSize * 0.2

                                Text {
                                    text: qsTr("From:")
                                    font.pixelSize: deleteFramePanel.deleteIconSize
                                    Layout.preferredWidth: content.width * 0.25
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Text {
                                    text: qsTr("Frame")
                                    font.pixelSize: deleteFramePanel.deleteIconSize
                                    Layout.preferredWidth: content.width * 0.25
                                    verticalAlignment: Text.AlignVCenter
                                }

                                TextField {
                                    id: fromValue
                                    text: "10"
                                    font.pixelSize:         deleteFramePanel.deleteIconSize
                                    Layout.preferredWidth: content.width * 0.4
                                    Layout.preferredHeight: deleteFramePanel.deleteIconSize  * 1.25
                                    horizontalAlignment: TextInput.AlignHCenter
                                    topPadding: 1
                                    bottomPadding: 0
                                    selectByMouse: true
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: deleteFramePanel.deleteIconSize * 0.2

                                Text {
                                    text: qsTr("Lon:")
                                    color:"#9c9c9c"
                                    font.pixelSize: deleteFramePanel.deleteIconSize
                                    Layout.preferredWidth: content.width * 0.3
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Text {
                                    id: fromLonValue
                                    text: "000.000000"
                                    color:"#9c9c9c"
                                    font.pixelSize: deleteFramePanel.deleteIconSize
                                    Layout.preferredWidth: content.width * 0.5
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: deleteFramePanel.deleteIconSize * 0.2

                                Text {
                                    text: qsTr("Lati:")
                                    color:"#828282"
                                    font.pixelSize: deleteFramePanel.deleteIconSize
                                    Layout.preferredWidth: content.width * 0.3
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Text {
                                    id: fromLatiValue
                                    text: "000.000000"
                                    color:"#828282"
                                    font.pixelSize: deleteFramePanel.deleteIconSize
                                    Layout.preferredWidth: content.width * 0.5
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 1
                                Layout.topMargin: 5
                                Layout.bottomMargin: 5
                                color: "#555555"
                            }

                            RowLayout {
                                Layout.fillWidth:true
                                spacing: deleteFramePanel.deleteIconSize  * 0.2

                                Text {
                                    text: qsTr("To:")
                                    font.pixelSize: deleteFramePanel.deleteIconSize
                                    Layout.preferredWidth: content.width * 0.25
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Text {
                                    text: qsTr("Frame")
                                    font.pixelSize: deleteFramePanel.deleteIconSize
                                    Layout.preferredWidth: content.width * 0.25
                                    verticalAlignment: Text.AlignVCenter
                                }

                                TextField {
                                    id: toValue
                                    text: "60"
                                    font.pixelSize:         deleteFramePanel.deleteIconSize
                                    Layout.preferredWidth: content.width * 0.4
                                    Layout.preferredHeight: deleteFramePanel.deleteIconSize * 1.25
                                    horizontalAlignment: TextInput.AlignHCenter
                                    topPadding: 1
                                    bottomPadding: 0
                                    selectByMouse: true
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: deleteFramePanel.deleteIconSize * 0.2

                                Text {
                                    text: qsTr("Lon:")
                                    color: "#828282"
                                    font.pixelSize: deleteFramePanel.deleteIconSize
                                    Layout.preferredWidth: content.width * 0.3
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Text {
                                    id: toLonValue
                                    text: "000.000000"
                                    color: "#828282"
                                    font.pixelSize: deleteFramePanel.deleteIconSize
                                    Layout.preferredWidth: content.width * 0.5
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: deleteFramePanel.deleteIconSize * 0.2

                                Text {
                                    text: qsTr("Lati:")
                                    color: "#828282"
                                    font.pixelSize: deleteFramePanel.deleteIconSize
                                    Layout.preferredWidth: content.width * 0.3
                                    verticalAlignment: Text.AlignVCenter
                                }

                                Text {
                                    id: toLatiValue
                                    text: "000.000000"
                                    color:"#828282"
                                    font.pixelSize: deleteFramePanel.deleteIconSize
                                    Layout.preferredWidth: content.width * 0.5
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }

                        }

                    }


                    RowLayout {
                        spacing: marksDrawer.markIconSize
                        Layout.alignment: Qt.AlignHCenter

                        Button {
                            text: qsTr("OK")
                            font.pixelSize: marksDrawer.markIconSize * 0.85
                            Layout.preferredWidth:  marksDrawer.markIconSize * 3.6
                            Layout.preferredHeight: marksDrawer.markIconSize * 1.2
                            palette.button: "#b9c6db"
                            onClicked: {
                                var ok = plot.deleteSelectedFrames()
                                if(ok) {
                                    fromValue.text = "0"
                                    toValue.text   = "0"
                                    fromLonValue.text  = "000.000000"
                                    fromLatiValue.text = "000.000000"
                                    toLonValue.text  = "000.000000"
                                    toLatiValue.text = "000.000000"
                                    plot.clearDeleteFrame()
                                }
                            }
                        }

                        Button {
                            text: qsTr("Cancel")
                            font.pixelSize: marksDrawer.markIconSize * 0.85
                            Layout.preferredWidth:  marksDrawer.markIconSize * 3.6
                            Layout.preferredHeight: marksDrawer.markIconSize * 1.2
                            palette.button: "#b9c6db"
                            onClicked: {
                                plot.clearDeleteFrame()
                            }
                        }

                    }

                }

                function open() {
                    height = plotIconSize * 11
                    opened = true
                }

                function close() {
                    height = 0
                    opened = false
                }
            }

            CCheck {
                id: deleteFrame
                Layout.fillWidth: true
                checked: false
                text: qsTr("Delete Frame")
                height: plotIconSize
                onCheckedChanged: {
                    if(deleteFrame.checked) {
                        deleteFramePanel.open()
                        plot.setDeleteFrameMode(true)
                    }
                    else {
                        deleteFramePanel.close()
                        plot.setDeleteFrameMode(false)
                    }
                }
            }

        }
    }
}

