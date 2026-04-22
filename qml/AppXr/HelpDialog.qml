import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15

Dialog {
    id: helpDialog
    title: qsTr("Help Document")
    width: 800
    height: 600
    // modal: true
    standardButtons: Dialog.Ok
    visible: true


    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            TextArea {
                id: helpText
                readOnly: true
                textFormat: Text.RichText
                font.pointSize: 12

                text: "" +
                    "<h1 style=\"text-align: center;\">Google Maps API Help</h1>" +
                    "<h2 style=\"margin-top: 30px;font-size: 16px; font-weight: normal;\">Method 1: Use Google Maps API to load map tiles</h2>" +
                    "<p>Visit <a href=\"https://cloud.google.com/maps-platform\">Google Cloud Platform</a> and register an account. Select the appropriate service (such as Maps SDK or Static Maps API) according to your needs, and obtain a valid API key.</p>" +
                    "<p>Using the Google Maps Tile API, you can construct a request URL to get map tiles. The tile URL format is usually as follows:</p>" +
                    "<pre>" +
                    "https://maps.googleapis.com/maps/api/staticmap?center={latitude},{longitude}&zoom={zoom_level}&size={width}x{height}&key={API_KEY}" +
                    "</pre>" +
                    "<h2 style=\"margin-top: 30px;font-size: 16px; font-weight: normal;\">Method 2: Directly use pre-built URL to load map</h2>" +
                    "<p>You can directly copy the following URL path, paste it into the text box at the bottom of this dialog, and click the green button on the right. The Google tile map will be loaded successfully. You can view it in the \"Map\" settings and select the Google map source.</p>" +
                    "<pre style=\"font-size: 17px;text-decoration: underline;\">" +
                    "<div style=\"background-color:#f8f9fa;border:1px solid #ddd;padding:10px;border-radius:6px;\">" +
                    "<div style=\"font-family:monospace;font-size:20px;\">" +
                    "http://mt2.google.com/vt/lyrs=y&hl=en&x=%1&y=%2&z=%3" +
                    "</div>" +
                    "<a href=\"copy-en-url\" " +
                    "style=\"background:#4CAF50;color:white;padding:6px 12px;border-radius:4px;" +
                    "text-decoration:none;font-size:14px;\">Copy URL</a>" +
                    "</div>" +
                    "</pre>" +
                    "<h2 style=\"margin-top: 30px;font-size: 16px; font-weight: normal;\">方式2：直接使用预构建的 URL 加载地图</h2>" +
                    "<p>您可以直接复制以下URL路径，粘贴到该对话框下方的文本框内，并点击右侧绿色按钮，则Google瓦片地图可成功加载，您可在设置的 \"Map\" 中查看，并选中Google图源使用。</p>" +
                    "<pre style=\"font-size: 17px;text-decoration: underline;\">" +
                    "<div style=\"background-color:#f8f9fa;border:1px solid #ddd;padding:10px;border-radius:6px;\">" +
                    "<div style=\"font-family:monospace;font-size:20px;\">" +
                    "http://mt2.google.com/vt/lyrs=y&hl=zh-CN&x=%1&y=%2&z=%3" +
                    "</div>" +
                    "<a href=\"copy-cn-url\" " +
                    "style=\"background:#4CAF50;color:white;padding:6px 12px;border-radius:4px;" +
                    "text-decoration:none;font-size:14px;\">复制 URL</a>" +
                    "</div>" +
                    "</pre>" +
                    "<h2 style=\"margin-top: 50px;\">Disclaimer</h2>" +
                    "<ul>" +
                    "<li>When using the Google Maps API, be sure to comply with the <a href=\"https://developers.google.com/maps/terms\">Google Maps/Google Earth API Terms of Service</a> and the <a href=\"https://www.google.com/permissions/geoguidelines/intl/en/terms.html\">Copyright Policy</a>.</li>" +
                    "<li>Ensure that your application does not violate any terms.</li>" +
                    "<li>The content provided in this document is for reference only. Users should consult and follow Google's official terms and policies before use.</li>" +
                    "<li>This document does not take responsibility for any copyright or legal issues arising from using this method.</li>" +
                    "<li>The final right of interpretation belongs to our company.</li>" +
                    "</ul>" +
                    "<hr style=\"margin: 50px 0; border: 1px solid #ddd;\">" +
                    "<h1 style=\"text-align: center;\">帮 助 文 档</h1>" +
                    "<h2 style=\"margin-top: 30px;font-size: 16px; font-weight: normal;\">方式1：使用 Google Maps API 加载地图瓦片</h2>" +
                    "<p>访问 <a href=\"https://cloud.google.com/maps-platform\">Google Cloud Platform</a> 并注册账号。根据您的需求选择相应的服务（如 Maps SDK 或 Static Maps API），并获取一个有效的API密钥。</p>" +
                    "<p>使用 Google Maps Tile API，您可以构建一个请求URL来获取地图瓦片。瓦片的URL格式通常如下：</p>" +
                    "<pre>" +
                    "https://maps.googleapis.com/maps/api/staticmap?center={latitude},{longitude}&zoom={zoom_level}&size={width}x{height}&key={API_KEY}" +
                    "</pre>" +
                    "<h2 style=\"margin-top: 30px;font-size: 16px; font-weight: normal;\">方式2：直接使用预构建的 URL 加载地图</h2>" +
                    "<p>您可以直接复制以下URL路径，粘贴到该对话框下方的文本框内，并点击右侧绿色按钮，则Google瓦片地图可成功加载，您可在设置的 \"Map\" 中查看，并选中Google图源使用。</p>" +
                    "<pre style=\"font-size: 17px;text-decoration: underline;\">" +
                    "<div style=\"background-color:#f8f9fa;border:1px solid #ddd;padding:10px;border-radius:6px;\">" +
                    "<div style=\"font-family:monospace;font-size:20px;\">" +
                    "http://mt2.google.com/vt/lyrs=y&hl=zh-CN&x=%1&y=%2&z=%3" +
                    "</div>" +
                    "<a href=\"copy-cn-url\" " +
                    "style=\"background:#4CAF50;color:white;padding:6px 12px;border-radius:4px;" +
                    "text-decoration:none;font-size:14px;\">复制 URL</a>" +
                    "</div>" +
                    "</pre>" +
                    "<h2 style=\"margin-top: 50px;\">免责声明</h2>" +
                    "<ul>" +
                    "<li>使用Google地图API时，请务必遵守 <a href=\"https://developers.google.com/maps/terms\">Google Maps/Google Earth API 服务条款</a> 和 <a href=\"https://www.google.com/permissions/geoguidelines/intl/en/terms.html\">版权政策</a>。</li>" +
                    "<li>确保您的应用程序不会违反任何条款。</li>" +
                    "<li>本文档提供的内容仅供参考，请用户在使用前自行查阅并遵 Google的官方条款和政策。</li>" +
                    "<li>本文档不对任何因使用此方法而产生的版权或法律问题承担责任。</li>" +
                    "<li>最终解释权归本公司所有。</li>" +
                    "</ul>"

                onLinkActivated: function(link) {
                    if (link === "copy-en-url") {
                        clipboard.text = "http://mt2.google.com/vt/lyrs=y&hl=en&x=%1&y=%2&z=%3";
                        messageDialog.title = "Copied";
                        messageDialog.text = "URL copied to clipboard!";
                        messageDialog.open();
                    } else if (link === "copy-cn-url") {
                        clipboard.text = "http://mt2.google.com/vt/lyrs=y&hl=zh-CN&x=%1&y=%2&z=%3";
                        messageDialog.title = "复制成功";
                        messageDialog.text = "URL 已复制到剪贴板！";
                        messageDialog.open();
                    } else {
                        // Open external links in default browser
                        Qt.openUrlExternally(link);
                    }
                }
            }
        }
    }

    MessageDialog {
        id: messageDialog
        standardButtons: MessageDialog.Ok
    }

}
