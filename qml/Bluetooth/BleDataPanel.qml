import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

Item {
    id: root
    width: bleSize * 0.6
    height: bleSize * 0.5
    x: Screen.width -  width - 3
    y: Screen.height - 2 * height
    z: 99

    property int bleSize: Math.min(Screen.width, Screen.height) * 0.4
    property int layoutHeight: bleSize * 0.1
    property int iconSize: bleSize * 0.06

    property string defaultLatLon: "000.000"
    property string defaultValue: "0.0"

    property bool dataReading: BleManager ? BleManager.dataReading : true


    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0.7, 1.0, 0.3, 0.7)
        border.color: Qt.rgba(0.5,0.8,0.2,0.9)
        border.width: 1
        anchors.margins: 4
    }


    GridLayout {
        anchors.fill: parent
        anchors.margins: 10
        columns: 2
        columnSpacing: 8
        rowSpacing: 5


        // 纬度
        Text { text: qsTr("Lat:"); font.pixelSize: iconSize-1; color: "#102040" }
        Text {
            id: latiText
            font.pixelSize: iconSize
            font.bold: true
            color: "#102040"
            text: dataReading ? (BleManager.latitude + "°") : text
        }


        // 经度
        Text { text: qsTr("Lon:"); font.pixelSize: iconSize-1; color: "#102040" }
        Text {
            font.pixelSize: iconSize
            font.bold: true
            color: "#102040"
            text: dataReading ? (BleManager.longitude + "°") : text
        }


        // 航向角
        Text { text: qsTr("Course:"); font.pixelSize: iconSize-1; color: "#102040" }
        Text {
            font.pixelSize: iconSize
            font.bold: true
            color: "#102040"
            text: dataReading ? (BleManager.angle + "°") : text
        }


        // 速度
        Text { text: qsTr("Speed:"); font.pixelSize: iconSize-1; color: "#102040" }
        Text {
            font.pixelSize: iconSize
            font.bold: true
            color: "#102040"
            text: dataReading ? (BleManager.speed + " m/s") : text
        }


        //深度
        Text { text: qsTr("Depth:"); font.pixelSize: iconSize-1; color: "#102040" }
        Text {
            font.pixelSize: iconSize
            font.bold: true
            color: "#102040"
            text: dataReading ? (BleManager.depth + " m") : text
        }

    }



}


