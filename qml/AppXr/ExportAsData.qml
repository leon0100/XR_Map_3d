import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15



Rectangle {
    id: exportAsData

    width:  loadSize * 1.5
    height: loadSize * 2.0
    x: Screen.width * 0.5  - width * 0.5
    y: 10
    z: 99
    visible: theme.exportAsDataVisible

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0;  color: "#f5f7fa" }
            GradientStop { position: 1.0;  color: "#c3cfe2" }
        }
    }

    property int loadSize: Math.min(Screen.width, Screen.height) * 0.4
    property int iconSize: loadSize * 0.04


    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        Label { text: qsTr("Export Data");  font.bold: true;  font.pixelSize: iconSize; }

        //文件格式
        GroupBox {
            title: qsTr("文件格式")
            font.pixelSize: iconSize;
            Layout.fillWidth: true
            height: iconSize * 2

            RowLayout {
                Layout.fillWidth: true
                spacing: iconSize * 2
                XRCheckBox {
                    text: "CSV格式"
                    checked: true
                    group: "fileFormat"
                }

                XRCheckBox {
                    text: "CAD(CASS)文本格式"
                    group: "fileFormat"
                }

                XRCheckBox {
                    text: "BLH(HGO)文本格式"
                    group: "fileFormat"
                }
            }
        }


        // GPS坐标格式
        GroupBox {
            title: qsTr("GPS坐标格式")
            font.pixelSize: iconSize;
            Layout.fillWidth: true
            height: iconSize * 4

            GridLayout {
                anchors.fill: parent
                columns: 2

                XRCheckBox {
                    text:"ddd.dddddddd"
                    checked:true
                    group: "GPSFormat"
                }

                XRCheckBox {
                    text:"dddmm.mmmmmm"
                    group: "GPSFormat"
                }

                XRCheckBox {
                    text:"ddd:mm:ss.ssss"
                    group: "GPSFormat"
                }

                XRCheckBox {
                    text:"ddd.mm[ss.ss]"
                    group: "GPSFormat"
                }
            }
        }


        // GPS状态
        GroupBox {
            title: "采样距离"
            font.pixelSize: iconSize;
            Layout.fillWidth: true

            RowLayout {
                spacing: iconSize * 2
                TextField {
                    width: iconSize * 2
                    text: "0"
                }

                ComboBox {
                    width: iconSize * 2
                    model: ["M", "KM"]
                }

                XRCheckBox {
                    text: "优于设置"
                    checked: true
                }
            }
        }



        // 深度输出设置
        GroupBox {
            title: "深度输出设置"
            font.pixelSize: iconSize;
            Layout.fillWidth: true
            height: iconSize * 4

            RowLayout {
                anchors.fill: parent
                spacing: iconSize

                RowLayout {
                    Layout.fillHeight: true
                    spacing: iconSize * 0.75

                    Label {
                        text: "单位:"
                        font.pixelSize: iconSize;
                    }

                    XRCheckBox {
                        text: "M"
                        checked: true
                        group: "depthFormat"
                    }

                    XRCheckBox {
                        text: "F"
                        group: "depthFormat"
                    }
                }


                ColumnLayout{
                    Layout.fillHeight: true
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: iconSize

                        Label {
                            text: "声速:"
                            font.pixelSize: iconSize
                            width: iconSize
                        }

                        TextField {
                            text: "1500"
                            font.pixelSize: iconSize
                            width: iconSize
                        }

                        Label {
                            text: "m/s"
                            font.pixelSize: iconSize;
                            width: iconSize
                        }
                    }


                    RowLayout {
                        Layout.fillWidth: true
                        spacing: iconSize

                        Label {
                            text: "吃水:"
                            font.pixelSize: iconSize;
                            width: iconSize
                        }

                        TextField {
                            text: "0"
                            font.pixelSize: iconSize;
                            width: iconSize

                        }

                        Label {
                            text: "m"
                            font.pixelSize: iconSize;
                            width: iconSize
                        }
                    }
                }

                XRCheckBox {
                    text: "更新深度"
                    checked:true
                }

            }

        }



        // 水面高程设置
        GroupBox {
            title: "水面高程设置"
            font.pixelSize: iconSize;
            Layout.fillWidth: true
            height: iconSize * 6

            ColumnLayout {
                anchors.fill: parent
                spacing: iconSize

                RowLayout {
                  Layout.fillHeight: true
                  spacing: iconSize

                  XRCheckBox {
                      text: "RTK高程"
                      checked: true
                  }

                  Label {
                      text: "天线到水面高度补偿:"
                      font.pixelSize: iconSize;
                  }


                  TextField {
                      width: iconSize
                      text: "0"
                  }

                  Label {
                      text: "m"
                      font.pixelSize: iconSize
                      width: iconSize
                  }

                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: iconSize

                    XRCheckBox {
                        text: "潮差"
                    }

                    Button {
                        text: "选择文件"
                        enabled: false
                    }

                    XRCheckBox {
                        text: "固定水位"
                    }

                    TextField {
                        text: "0"
                        width: iconSize
                        enabled: false
                    }

                    Label {
                        text: "m"
                        font.pixelSize: iconSize;
                    }
                }

            }
        }




        // 其它选项
        GroupBox {
            title:"其它选项"
            font.pixelSize: iconSize;
            Layout.fillWidth: true
            height: iconSize * 4

            GridLayout {
                anchors.fill: parent
                columns: 3

                XRCheckBox {
                    text:"删除深度为0采样点"
                    checked:true
                }

                XRCheckBox {
                    text:"计算水底高程"
                    checked:true
                }

                XRCheckBox {
                    text:"添加列标题"
                    checked:true
                }

                XRCheckBox {
                    text:"添加行编号"
                    checked:true
                }

                XRCheckBox {
                    text:"应用深度滤波值"
                }

            }

        }



    RowLayout {
        Layout.alignment: Qt.AlignHCenter
        spacing: loadSize * 0.3

        Button {
            text: qsTr("Export")
            font.pixelSize: iconSize;
            width: iconSize * 3
            onClicked: {
                console.log("开始导出")
            }
        }

        Button {
            text: qsTr("Close")
            font.pixelSize: iconSize;
            width: iconSize * 3
            onClicked:{
                theme.exportAsDataVisible = false
            }
        }

    }




    }


}
