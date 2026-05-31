import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

Rectangle {
    id: locations
    width:  locationsSize * 0.9
    height: locationsSize
    radius: 10
    color: "#e2e8f2"
    x: Screen.width * 0.5 - width * 0.5
    y: Screen.height * 0.5 -  height * 0.75
    z: 999

    property int locationsSize: Math.min(Screen.width, Screen.height) * 0.5
    property int iconSize: locationsSize * 0.03
    property int boxSize: iconSize * 1.2

    // 0: LatLon  1: PlaceName  2: KML
    property int locationStyle: 0

    ColumnLayout
    {
        anchors.fill: parent
        spacing: iconSize * 0.9
        anchors.margins: 10

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: boxSize
            Layout.alignment: Qt.AlignTop
            color: "transparent"

            Row {
                anchors.fill: parent
                anchors.margins: iconSize * 0.6
                spacing: 2

                Image {
                    width: boxSize
                    height: boxSize
                    source: "qrc:/XR/icon_XR_map.png"
                    fillMode: Image.PreserveAspectFit
                }

                Text {
                    text: qsTr("Sepcify Position")
                    font.pixelSize: iconSize * 0.9
                    color: "black"
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }


        /* ================================== LatLon ===================================== */
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 2
            color: locationStyle === 0 ? "#cae1ff" : "transparent"
            radius: 10

            ColumnLayout {
                anchors.fill: parent
                spacing: iconSize * 0.3
                anchors.margins: iconSize * 0.6

                RowLayout {
                    ToolButton {
                        width: boxSize
                        height: boxSize
                        padding: 0
                        onClicked: {
                            locationStyle = 0
                            locations.setLocationStyle(0)
                        }
                        contentItem: Item {
                            anchors.fill: parent
                            Image {
                                anchors.centerIn: parent
                                width: boxSize
                                height: boxSize
                                source: "qrc:/XR/yeslogo.png"
                                visible: locationStyle === 0
                            }
                        }
                        background: Rectangle {
                            radius: 3
                            color: "transparent"
                            border.width: 2
                        }
                    }

                    Rectangle {
                        height: 2
                        color: "#808080"
                        Layout.fillWidth: true
                    }
                }

                RowLayout {
                    enabled: locationStyle === 0
                    Layout.fillWidth: true
                    spacing: iconSize

                    ColumnLayout {
                        Layout.preferredWidth: 1
                        spacing: iconSize * 0.4
                        Label { text: qsTr("Latitude");  font.pixelSize: iconSize }
                        Label { text: qsTr("Longitude"); font.pixelSize: iconSize }
                    }

                    ColumnLayout {
                        Layout.preferredWidth: 2
                        spacing: iconSize * 0.4

                        TextField {
                            id: latField
                            Layout.preferredHeight: boxSize
                            font.pixelSize: iconSize * 0.85
                            leftPadding: 6
                            rightPadding: 6
                            topPadding: 3
                            bottomPadding: 3
                            placeholderText: qsTr("Latitude")

                             //绑定 loc.latitude → QML 自动监听 signalLatitude，信号发射时更新 text
                            text: locations ? locations.latitude : "" //这种是单向绑定，locations.latitude给text赋值
                            selectByMouse: true
                            Keys.onPressed: function(event) {
                                if (event.key === Qt.Key_V && event.modifiers & Qt.ControlModifier) {
                                    Qt.callLater(function() { locations.onLatLonChanged(latField.text) })
                                }
                            }
                            // onTextChanged: locations.latitude = latField.text //如果采用这种方式的话，
                            //则C++端的Q_PROPERTY()需要在里面加个WRITE函数（目前这样会导致Binding loop）
                            onTextChanged: locations.setLatitude2(latField.text)

                            Menu {
                                id: latiMenu
                                implicitWidth: iconSize * 5
                                implicitHeight: iconSize * 2.5
                                MenuItem {
                                    text: qsTr("Paste")
                                    enabled: latField.canPaste
                                    onTriggered: {
                                        latField.paste()
                                        locations.onLatLonChanged(latField.text)
                                    }
                                }
                            }

                            onPressed: function(event) {
                                if (event.button === Qt.RightButton) {
                                    forceActiveFocus()
                                    lonMenu.close()
                                    latiMenu.popup()
                                    event.accepted = true
                                } else if (event.button === Qt.LeftButton) {
                                    latiMenu.close()
                                }
                            }
                        }

                        TextField {
                            id: lonField
                            Layout.preferredHeight: boxSize
                            font.pixelSize: iconSize * 0.85
                            leftPadding: 6
                            rightPadding: 6
                            topPadding: 3
                            bottomPadding: 3
                            placeholderText: qsTr("Longitude")
                            text: locations ? locations.longitude : ""
                            selectByMouse: true
                            Keys.onPressed: function(event) {
                                if (event.key === Qt.Key_V && event.modifiers & Qt.ControlModifier) {
                                    Qt.callLater(function() { locations.onLatLonChanged(lonField.text) })
                                }
                            }
                            onTextChanged: locations.setLongitude2(text)

                            Menu {
                                id: lonMenu
                                implicitWidth: iconSize * 5
                                implicitHeight: iconSize * 2.5
                                MenuItem {
                                    text: qsTr("Paste")
                                    enabled: lonField.canPaste
                                    onTriggered: {
                                        lonField.paste()
                                        locations.onLatLonChanged(lonField.text)
                                    }
                                }
                            }

                            onPressed: function(event) {
                                if (event.button === Qt.RightButton) {
                                    forceActiveFocus()
                                    latiMenu.close()
                                    lonMenu.popup()
                                    event.accepted = true
                                } else if (event.button === Qt.LeftButton) {
                                    lonMenu.close()
                                }
                            }
                        }

                    }

                    ColumnLayout {
                        Layout.preferredWidth: 2
                        Label {
                            font.pixelSize: iconSize * 0.90
                            text: qsTr("Latitude Longitude Format Units")
                        }
                        ComboBox {
                            Layout.preferredWidth: locationsSize * 0.4
                            Layout.preferredHeight: iconSize * 1.2
                            font.pixelSize: iconSize * 0.9
                            model: [
                                qsTr("Decimal Degrees (DD)"),
                                qsTr("Degrees, Minutes, Seconds (DMS)"),
                                qsTr("Degrees, Decimal Minutes (DDM)")
                            ]

                            onCurrentIndexChanged: locations.setLatLonFormat(currentIndex)

                            delegate: ItemDelegate {
                                width: parent.width
                                implicitHeight: iconSize * 1.5
                                padding: 2
                                contentItem: Text {
                                    text: modelData
                                    font.pixelSize: iconSize * 0.9
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                }
                            }

                            background: Rectangle {
                                radius: 8
                                color: "white"
                                border.color: "#B0B0B0"
                                border.width: 1
                            }
                        }
                    }
                }
            }
        }




        /* =================================== Place Name =========================================== */
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 3
            color: locationStyle === 1 ? "#cae1ff" : "transparent"
            radius: 10

            ColumnLayout {
                anchors.fill: parent
                spacing: 6
                anchors.margins: 10

                RowLayout {
                    ToolButton {
                        width: boxSize
                        height: boxSize
                        padding: 0
                        onClicked: {
                            locationStyle = 1
                            locations.setLocationStyle(1)
                        }
                        contentItem: Item {
                            anchors.fill: parent
                            Image {
                                anchors.centerIn: parent
                                width: boxSize
                                height: boxSize
                                source: "qrc:/XR/yeslogo.png"
                                visible: locationStyle === 1
                            }
                        }
                        background: Rectangle {
                            radius: 3
                            color: "transparent"
                            border.width: 2
                        }
                    }

                    Rectangle {
                        height: 2
                        color: "#808080"
                        Layout.fillWidth: true
                    }
                }


                /*******************模糊搜索结果列表**********************/
                Rectangle {
                    id: fuzzyPopup
                    width: fuzzyField.width
                    radius: 6
                    color: "#ffffff"
                    border.color: "#c0c0c0"
                    border.width: 1
                    Layout.leftMargin: locationsSize * 0.3
                    visible: locations ? locations.fuzzyResults.length > 0 : false
                    z: 999

                    ListView {
                        id: fuzzyList
                        width: parent.width
                        height: Math.min(contentHeight, locationsSize * 0.3)
                        clip: true

                        model: locations ? locations.fuzzyResults : []

                        delegate: Item {
                            width: fuzzyList.width
                            height: iconSize * 2

                            Rectangle {
                                anchors.fill: parent
                                color: mouseArea.containsMouse ? "#8b8378" : "#cdc0b0"
                            }

                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                anchors.leftMargin: iconSize * 0.5
                                text: modelData
                                font.pixelSize: iconSize + 1
                                elide: Text.ElideRight
                            }

                            MouseArea {
                                id: mouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    fuzzyField.text = modelData
                                    locations.clearFuzzySearch(modelData)
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    enabled: locationStyle === 1
                    Label { text: qsTr("Fuzzy Search"); font.pixelSize: iconSize }

                    TextField {
                        id: fuzzyField
                        Layout.minimumHeight: iconSize * 2
                        Layout.preferredHeight: iconSize * 2
                        Layout.preferredWidth: locationsSize * 0.3
                        Layout.minimumWidth: locationsSize * 0.3
                        font.pixelSize: iconSize
                        leftPadding: 6
                        rightPadding: 6
                        topPadding: 3
                        bottomPadding: 3
                        placeholderText: qsTr("Search place")
                        selectByMouse: true
                        Keys.onPressed: function(event) {
                            if (event.key === Qt.Key_V && (event.modifiers & Qt.ControlModifier)) {
                                Qt.callLater(function() { locations.setFuzzySearch2(fuzzyField.text) })
                            }
                        }
                        onTextChanged: locations.setFuzzySearch2(fuzzyField.text)

                        Menu {
                            id: fuzzyMenu
                            implicitWidth: iconSize * 3
                            implicitHeight: iconSize * 1.5
                            MenuItem {
                                text: qsTr("Paste")
                                enabled: fuzzyField.canPaste
                                onTriggered: {
                                    fuzzyField.paste()
                                    locations.setFuzzySearch2(fuzzyField.text)
                                }
                            }
                        }

                        onPressed: function(event) {
                            if (event.button === Qt.RightButton) {
                                forceActiveFocus()
                                fuzzyMenu.popup()
                                event.accepted = true
                            } else if (event.button === Qt.LeftButton) {
                                fuzzyMenu.close()
                            }
                        }

                        Connections {
                            target: locations
                            function onSignalFuzzySearch(text) {
                                fuzzyField.text = locations.fuzzySearch
                            }
                        }
                    }


                }

                Rectangle {
                    height: 1
                    color: "#cccccc"
                    Layout.fillWidth: true
                }

                GridLayout {
                    enabled: locationStyle === 1
                    columns: 4

                    Label { text: qsTr("Street"); font.pixelSize: iconSize }
                    TextField {
                        id: streetField
                        Layout.preferredHeight: iconSize * 1.8
                        font.pixelSize: iconSize * 1.1
                        leftPadding: 6
                        rightPadding: 6
                        topPadding: 3
                        bottomPadding: 3
                        placeholderText: qsTr("Street")
                        selectByMouse: true
                        Keys.onPressed: function(event) {
                            if (event.key === Qt.Key_V && event.modifiers & Qt.ControlModifier) {
                                Qt.callLater(function() { locations.setStreet2(streetField.text) })
                            }
                        }
                        onTextChanged: locations.setStreet2(text)

                        Menu {
                            id: streetMenu
                            implicitWidth: iconSize * 5
                            implicitHeight: iconSize * 2.5
                            MenuItem {
                                text: qsTr("Paste")
                                enabled: streetField.canPaste
                                onTriggered: {
                                    streetField.paste()
                                    locations.setStreet2(streetField.text)
                                }
                            }
                        }

                        onPressed: function(event) {
                            if (event.button === Qt.RightButton) {
                                forceActiveFocus()
                                streetMenu.popup()
                                event.accepted = true
                            } else if (event.button === Qt.LeftButton) {
                                streetMenu.close()
                            }
                        }

                        Connections {
                            target: locations
                            function onSignalStreet(text) {
                                streetField.text = locations.street
                            }
                        }


                    }




                    Label { text: qsTr("City"); font.pixelSize: iconSize }
                    TextField {
                        id: cityField
                        Layout.preferredHeight: iconSize * 1.8
                        font.pixelSize: iconSize * 1.1
                        leftPadding: 6
                        rightPadding: 6
                        topPadding: 3
                        bottomPadding: 3
                        placeholderText: qsTr("City")
                        selectByMouse: true
                        Keys.onPressed: function(event) {
                            if (event.key === Qt.Key_V && event.modifiers & Qt.ControlModifier) {
                                Qt.callLater(function() { locations.setCity2(cityField.text) })
                            }
                        }
                        onTextChanged: locations.setCity2(text)

                        Menu {
                            id: cityMenu
                            implicitWidth: iconSize * 5
                            implicitHeight: iconSize * 2.5
                            MenuItem {
                                text: qsTr("Paste")
                                enabled: cityField.canPaste
                                onTriggered: {
                                    cityField.paste()
                                    locations.setCity2(cityField.text)
                                }
                            }
                        }

                        onPressed: function(event) {
                            if (event.button === Qt.RightButton) {
                                forceActiveFocus()
                                cityMenu.popup()
                                event.accepted = true
                            } else if (event.button === Qt.LeftButton) {
                                cityMenu.close()
                            }
                        }

                        Connections {
                            target: locations
                            function onSignalCity(text) {
                                cityField.text = locations.city
                            }
                        }
                    }

                    Label { text: qsTr("State/Province"); font.pixelSize: iconSize }
                    TextField {
                        id: provinceField
                        Layout.preferredHeight: iconSize * 1.8
                        font.pixelSize: iconSize * 1.1
                        leftPadding: 6
                        rightPadding: 6
                        topPadding: 3
                        bottomPadding: 3
                        placeholderText: qsTr("State/Province")
                        selectByMouse: true
                        Keys.onPressed: function(event) {
                            if (event.key === Qt.Key_V && event.modifiers & Qt.ControlModifier) {
                                Qt.callLater(function() { locations.setProvince2(provinceField.text) })
                            }
                        }
                        onTextChanged: locations.setProvince2(text)

                        Menu {
                            id: provinceMenu
                            implicitWidth: iconSize * 5
                            implicitHeight: iconSize * 2.5
                            MenuItem {
                                text: qsTr("Paste")
                                enabled: provinceField.canPaste
                                onTriggered: {
                                    provinceField.paste()
                                    locations.setProvince2(provinceField.text)
                                }
                            }
                        }

                        onPressed: function(event) {
                            if (event.button === Qt.RightButton) {
                                forceActiveFocus()
                                provinceMenu.popup()
                                event.accepted = true
                            } else if (event.button === Qt.LeftButton) {
                                provinceMenu.close()
                            }
                        }

                        Connections {
                            target: locations
                            function onSignalProvince(text) {
                                provinceField.text = locations.province
                            }
                        }
                    }

                    Label { text: qsTr("Country"); font.pixelSize: iconSize }
                    TextField {
                        id: countryField
                        Layout.preferredHeight: iconSize * 1.8
                        font.pixelSize: iconSize * 1.1
                        leftPadding: 6
                        rightPadding: 6
                        topPadding: 3
                        bottomPadding: 3
                        placeholderText: qsTr("Country")
                        selectByMouse: true
                        Keys.onPressed: function(event) {
                            if (event.key === Qt.Key_V && event.modifiers & Qt.ControlModifier) {
                                Qt.callLater(function() { locations.setCountry2(countryField.text) })
                            }
                        }
                        onTextChanged: locations.setCountry2(text)

                        Menu {
                            id: countryMenu
                            implicitWidth: iconSize * 5
                            implicitHeight: iconSize * 2.5
                            MenuItem {
                                text: qsTr("Paste")
                                enabled: countryField.canPaste
                                onTriggered: {
                                    countryField.paste()
                                    locations.setCountry2(countryField.text)
                                }
                            }
                        }

                        onPressed: function(event) {
                            if (event.button === Qt.RightButton) {
                                forceActiveFocus()
                                countryMenu.popup()
                                event.accepted = true
                            } else if (event.button === Qt.LeftButton) {
                                countryMenu.close()
                            }
                        }

                        Connections {
                            target: locations
                            function onSignalCountry(text) {
                                countryField.text = locations.country
                            }
                        }
                    }

                }
            }
        }




        /* ==================================== KML ======================================== */
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 1
            color: locationStyle === 2 ? "#cae1ff" : "transparent"
            radius: 10

            ColumnLayout {
                anchors.fill: parent
                spacing: 6
                anchors.margins: 10

                RowLayout {
                    ToolButton {
                        width: boxSize
                        height: boxSize
                        padding: 0
                        onClicked: {
                            locationStyle = 2
                            locations.setLocationStyle(2)
                        }
                        contentItem: Item {
                            anchors.fill: parent
                            Image {
                                anchors.centerIn: parent
                                width: boxSize
                                height: boxSize
                                source: "qrc:/XR/yeslogo.png"
                                visible: locationStyle === 2
                            }
                        }
                        background: Rectangle {
                            radius: 3
                            color: "transparent"
                            border.width: 2
                        }
                    }

                    Rectangle {
                        height: 2
                        color: "#808080"
                        Layout.fillWidth: true
                    }
                }

                RowLayout {
                    enabled: locationStyle === 2
                    TextField {
                        id: textField
                        Layout.fillWidth: true
                        Layout.preferredHeight: boxSize
                        font.pixelSize: iconSize
                        leftPadding: 6
                        rightPadding: 6
                        topPadding: 3
                        bottomPadding: 3
                        placeholderText: qsTr("Import KMZ/KML File")
                        selectByMouse: true
                        Keys.onPressed: function(event) {
                            if (event.key === Qt.Key_V && event.modifiers & Qt.ControlModifier) {
                                Qt.callLater(function() { locations.setImportKml2(textField.text) })
                            }
                        }
                        onTextChanged: locations.setImportKml2(textField.text)

                        Menu {
                            id: textMenu
                            implicitWidth: iconSize * 3
                            implicitHeight: iconSize * 1.5
                            MenuItem {
                                text: qsTr("Paste")
                                enabled: textField.canPaste
                                onTriggered: {
                                    textField.paste()
                                    locations.setImportKml2(textField.text)
                                }
                            }
                        }

                        onPressed: function(event) {
                            if (event.button === Qt.RightButton) {
                                forceActiveFocus()
                                textMenu.popup()
                                event.accepted = true
                            } else if (event.button === Qt.LeftButton) {
                                textMenu.close()
                            }
                        }

                        Connections {
                            target: locations
                            function onSignalImportKml(text) {
                                textField.text = locations.importKml
                            }
                        }
                    }


                    Rectangle {
                        id: saveBtn
                        Layout.preferredWidth: boxSize
                        Layout.preferredHeight: boxSize
                        radius: 6

                        color: mouseArea2.containsMouse ? "#9bbfe9" : "#d9edff"
                        border.color: "#9bbfe9"
                        border.width: 1

                        Image {
                            anchors.centerIn: parent
                            width: boxSize
                            height: boxSize
                            source: "qrc:/XR/openkml.png"
                            smooth: true
                        }

                        MouseArea {
                            id: mouseArea2
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: locations.importKmlClicked()
                        }
                    }


                }
            }
        }


        /* ======================================= Buttons ============================================ */
        RowLayout {
            Layout.fillWidth: true
            Layout.bottomMargin: 10
            spacing: iconSize * 6

            Item { Layout.fillWidth: true }

            Button {
                id: confirmBtn
                text: qsTr("Confirm")
                Layout.preferredWidth: iconSize * 6
                font.pixelSize: iconSize + 3

                background: Rectangle {
                    radius: 6
                    color: confirmBtn.hovered ? "#d0e8ff" : "#d9edff"
                    border.color: "#9bbfe9"
                    border.width: 1
                }

                onClicked: locations.confirmClicked()

            }

            Button {
                id: cancelBtn
                text: qsTr("Cancel")
                Layout.preferredWidth: iconSize * 6
                font.pixelSize: iconSize + 3

                background: Rectangle {
                    radius: 6
                    color: cancelBtn.hovered ? "#d0e8ff" : "#d9edff"
                    border.color: "#9bbfe9"
                    border.width: 1
                }

                onClicked: locations.cancelClicked()
            }

            Item { Layout.fillWidth: true }
        }

    }
}

