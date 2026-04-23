import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15




Dialog
{
    id: helpDialog
    title: qsTr("Help Document")
    width: Screen.width
    height: Screen.height
    standardButtons: Dialog.NoButton

    property int iconSize:  Math.min(Screen.width, Screen.height) * 0.01

    MessageDialog {
       id: messageDialog
       title: qsTr("Copied")
       text: qsTr("URL copied to clipboard!")
       icon: StandardIcon.Information
       standardButtons: StandardButton.Ok
    }

    TextEdit {
       id: clipboardHelper
       visible: false
    }

    function copyToClipboard(txt) {
       clipboardHelper.text = txt;
       clipboardHelper.selectAll();
       clipboardHelper.copy();
    }

    ColumnLayout {
        anchors.fill: parent

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            contentWidth: availableWidth

            Text {
                id: helpText
                width: parent.width
                wrapMode: Text.Wrap
                textFormat: Text.RichText
                font.pointSize: iconSize

                text:
                    "<h1 style=\"text-align: center; margin:12px 0;\">Google Maps API Help</h1>" +

                    "<h2 style=\"margin:12px 0 6px 0;font-size:16px; font-weight: normal;\">Method 1: Use Google Maps API to load map tiles</h2>" +
                    "<p style=\"margin:6px 0;\">Visit <a href=\"https://cloud.google.com/maps-platform\">Google Cloud Platform</a> and register an account. Select the appropriate service (such as Maps SDK or Static Maps API) according to your needs, and obtain a valid API key.</p>" +
                    "<p style=\"margin:6px 0;\">Using the Google Maps Tile API, you can construct a request URL to get map tiles. The tile URL format is usually as follows:</p>" +

                    "<pre>" +
                    "https://maps.googleapis.com/maps/api/staticmap?center={latitude},{longitude}&zoom={zoom_level}&size={width}x{height}&key={API_KEY}" +
                    "</pre>" +

                    "<h2 style=\"margin:12px 0 6px 0;font-size:16px; font-weight: normal;\">Method 2: Directly use pre-built URL to load map</h2>" +
                    "<p style=\"margin:6px 0;\">You can directly copy the following URL path, paste it into the text box at the bottom of this dialog, and click the green button on the right.</p>" +

                    "<div style=\"background-color:#f8f9fa;border:1px solid #ddd;padding:10px;border-radius:6px;\">" +
                    "<div style=\"font-family:monospace;\">" +
                    "http://mt2.google.com/vt/lyrs=y&hl=en&x=%1&y=%2&z=%3" +
                    "</div>" +
                    "<a href=\"copy-en-url\" " +
                    "style=\"background:#4CAF50;color:white;padding:6px 12px;border-radius:4px;text-decoration:none;\">Copy URL</a>" +
                    "</div>" +

                    "<h2 style=\"margin:16px 0 6px 0;\">Disclaimer</h2>" +
                    "<ul style=\"margin:6px 0; padding-left:16px;\">" +
                    "<li>When using the Google Maps API, comply with the <a href=\"https://developers.google.com/maps/terms\">Terms of Service</a>.</li>" +
                    "<li>The content is for reference only.</li>" +
                    "</ul>" +

                    "<hr style=\"margin:20px 0;\">" +

                    "<h1 style=\"text-align: center; margin:12px 0;\">帮助文档</h1>" +

                    "<h2 style=\"margin:12px 0 6px 0;font-size:16px; font-weight: normal;\">方式1：使用 Google Maps API 加载地图瓦片</h2>" +
                    "<p style=\"margin:6px 0;\">访问 <a href=\"https://cloud.google.com/maps-platform\">Google Cloud Platform</a> 获取 API Key。</p>" +

                    "<h2 style=\"margin:12px 0 6px 0;font-size:16px; font-weight: normal;\">方式2：直接使用预构建的 URL 加载地图</h2>" +
                    "<p style=\"margin:6px 0;\">复制以下 URL 使用：</p>" +

                    "<div style=\"background-color:#f8f9fa;border:1px solid #ddd;padding:10px;border-radius:6px;\">" +
                    "<div style=\"font-family:monospace;\">" +
                    "http://mt2.google.com/vt/lyrs=y&hl=zh-CN&x=%1&y=%2&z=%3" +
                    "</div>" +
                    "<a href=\"copy-cn-url\" " +
                    "style=\"background:#4CAF50;color:white;padding:6px 12px;border-radius:4px;text-decoration:none;\">复制 URL</a>" +
                    "</div>" +

                    "<h2 style=\"margin:16px 0 6px 0;\">免责声明</h2>" +
                    "<ul style=\"margin:6px 0; padding-left:16px;\">" +
                    "<li>请遵守 Google 地图服务条款。</li>" +
                    "<li>本文仅供参考。</li>" +
                    "</ul>"

                onLinkActivated: function(link) {
                    if (link === "copy-en-url") {
                        var urlEn = "http://mt2.google.com/vt/lyrs=y&hl=en&x=%1&y=%2&z=%3";
                        copyToClipboard(urlEn);
                        messageDialog.open();
                    }
                    else if (link === "copy-cn-url") {
                        var urlCn = "http://mt2.google.com/vt/lyrs=y&hl=zh-CN&x=%1&y=%2&z=%3";
                       copyToClipboard(urlCn);
                        messageDialog.open();
                    }
                }
            }
        }


        Rectangle {
           id: customOkButton
           Layout.preferredWidth: parent.width * 0.2
           Layout.preferredHeight: iconSize * 6
           Layout.alignment: Qt.AlignHCenter
           Layout.bottomMargin: Screen.height * 0.1

           radius: 8

           Text {
             anchors.centerIn: parent
             text: qsTr("OK")
             font.pixelSize: iconSize * 3.0
             font.bold: true
           }

           MouseArea {
             anchors.fill: parent
             hoverEnabled: true
             onClicked: helpDialog.close()
          }
       }


    }

}
