import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15

Dialog {
    id: helpDialog
    width: Screen.width
    height: Screen.height
    modal: true
    title: qsTr("Help Document")

    ColumnLayout {
        anchors.fill: parent

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            TextArea {
                id: textBrowser
                readOnly: true
                textFormat: Text.RichText
                wrapMode: Text.Wrap

                text: `
<h1 align="center">Help Documentation</h1>

<h2>Method 1: Use Google Maps API</h2>
<p>
Visit <a href="https://cloud.google.com/maps-platform">Google Cloud Platform</a>
</p>

<pre>
https://maps.googleapis.com/maps/api/staticmap?center={latitude},{longitude}&zoom={zoom_level}
</pre>

<h2>Method 2</h2>
<p>
<a href="copy-en-url">Copy EN URL</a>
</p>

<p>
<a href="copy-cn-url">复制 CN URL</a>
</p>
`
                onLinkActivated: function(link) {
                    if (link === "copy-en-url") {
                        Qt.callLater(function() {
                            Qt.application.clipboard.setText(
                                "http://mt2.google.com/vt/lyrs=y&hl=en&x=%1&y=%2&z=%3"
                            )
                        })
                        messageDialog.text = "URL copied!"
                        messageDialog.open()
                    }
                    else if (link === "copy-cn-url") {
                        Qt.application.clipboard.setText(
                            "http://mt2.google.com/vt/lyrs=y&hl=zh-CN&x=%1&y=%2&z=%3"
                        )
                        messageDialog.text = "复制成功"
                        messageDialog.open()
                    }
                    else {
                        Qt.openUrlExternally(link)
                    }
                }
            }
        }

        Button {
            text: qsTr("Close")
            Layout.alignment: Qt.AlignHCenter
            onClicked: helpDialog.close()
        }
    }

    // MessageDialog {
    //     id: messageDialog
    //     title: "Info"
    // }
}
