import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15


Item {
    id: root
    width: 200
    height: 160


    property string defaultLatLon: "000.000"
    property string defaultValue: "0.0"

    property bool paused: bleManager ? bleManager.dataPaused : false


    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0.7, 1.0, 0.3, 0.7)
        border.color: Qt.rgba(0.5,0.8,0.2,0.9)
        border.width: 1
        anchors.margins: 4
        visible: true
    }


    GridLayout {
        columns: 2
        columnSpacing: 8
        rowSpacing: 5
        anchors.centerIn: parent


        // 纬度
        Text { text: qsTr("Lat:"); font.pixelSize: 17; color: "#102040" }
        Text {
            id: latiText
            font.pixelSize: 18
            font.bold: true
            color: "#102040"
            text: paused ? text : (bleManager.latitude + "°")
        }


        // 经度
        Text { text: qsTr("Lon:"); font.pixelSize: 17; color: "#102040" }
        Text {
            font.pixelSize: 18
            font.bold: true
            color: "#102040"
            text: paused ? text : (bleManager.longitude + "°")
        }


        // 航向角
        Text { text: qsTr("Course:"); font.pixelSize: 17; color: "#102040" }
        Text {
            font.pixelSize: 18
            font.bold: true
            color: "#102040"
            text: paused ? text : (bleManager.angle + "°")
        }


        // 速度
        Text { text: qsTr("Speed:"); font.pixelSize: 17; color: "#102040" }
        Text {
            font.pixelSize: 18
            font.bold: true
            color: "#102040"
            text: paused ? text : (bleManager.speed + " m/s")
        }


        //深度
        Text { text: qsTr("Depth:"); font.pixelSize: 17; color: "#102040" }
        Text {
            font.pixelSize: 18
            font.bold: true
            color: "#102040"
            text: paused ? text : (bleManager.depth + " m")
        }

    }



}


