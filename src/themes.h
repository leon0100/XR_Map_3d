#ifndef THEME_H
#define THEME_H

#include <QObject>
#include <QFont>
#include <QColor>
#include <QGuiApplication>
#include <QScreen>
#include <QtAlgorithms>
#include <QDebug>
#include <QFontDatabase>
#include <QApplication>
#include <QTranslator>
#include <QQmlApplicationEngine>


#include <QWidget>
#include <QMenu>
#include <QMessageBox>
#include <QStringListModel>
#include <QListWidgetItem>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <QPushButton>
#include <QLineEdit>
#include <QTextBrowser>
#include <QDockWidget>
#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>



#include "dataset_defs.h"
#include "console.h"













#define  AMAP               "Amap"
#define  GEOVISEARTHMAP     "GeovisEarthMap"
#define  OPENSTREETMAP      "OpenStreetMap"
#define  GOOGLEMAP          "GoogleMap"
#define  USERDEFINED        "User Defined"

#define SAVE_SOFTWARE_PAR_PATH    "./config"  //默认保存参数的路径
#define SAVE_SOFTWARE_PAR         "cfg"      //默认保存参数的路径


class HelpDialog : public QDialog {
    Q_OBJECT
public:
    explicit HelpDialog(QWidget *parent = nullptr) : QDialog(parent)
    {
        setWindowTitle(tr("Help Document"));
        setWindowFlags(Qt::Dialog | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen) {
        //     // resize(screen->size().width() * 0.75, screen->size().height() * 0.8);
            resize(screen->availableGeometry().size());
        }
        showMaximized();


        QVBoxLayout *layout = new QVBoxLayout(this);

        textBrowser = new QTextBrowser(this);
        textBrowser->setOpenLinks(false);
        textBrowser->setOpenExternalLinks(false);
        textBrowser->setReadOnly(true);
        textBrowser->setTextInteractionFlags(Qt::TextSelectableByMouse| Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse);
        textBrowser->setHtml(
            "<h1 style=\"text-align: center;\">Help Documentation</h1>"
            "<h2 style=\"margin-top: 30px;font-size: 16px; font-weight: normal;\">Method 1: Use Google Maps API to Load Map Tiles</h2>"
            "<p>Visit <a href=\"https://cloud.google.com/maps-platform\">Google Cloud Platform</a> and register an account. Choose the appropriate service based on your needs (such as Maps SDK or Static Maps API) and obtain a valid API key.</p>"
            "<p>Using the Google Maps Tile API, you can construct a request URL to fetch map tiles. The tile URL format is typically as follows:</p>"
            "<pre>"
            "https://maps.googleapis.com/maps/api/staticmap?center={latitude},{longitude}&zoom={zoom_level}&size={width}x{height}&key={API_KEY}"
            "</pre>"
            "<h2 style=\"margin-top: 30px;font-size: 16px; font-weight: normal;\">Method 2: Directly Use Pre-Built URLs to Load Maps</h2>"
            "<p>You can directly copy the URL path below, paste it into the text box below this dialog, and click the green button on the right to successfully load Google tile maps. You can view and select the Google map source in the \"Map\" settings.</p>"
            "<pre style=\"font-size: 17px;text-decoration: underline;\">"
            "<div style=\"background-color:#f8f9fa;border:1px solid #ddd;padding:10px;border-radius:6px;\">"
            "<div style=\"font-family:monospace;font-size:20px;\">"
            "http://mt2.google.com/vt/lyrs=y&hl=en&x=%1&y=%2&z=%3"
            "</div>"
            "<a href=\"copy-en-url\" "
            "style=\"background:#4CAF50;color:white;padding:6px 12px;border-radius:4px;"
            "text-decoration:none;font-size:14px;\">Copy URL</a>"
            "</div>"
            "</pre>"
            "<h2 style=\"margin-top: 50px;\">Disclaimer</h2>"
            "<ul>"
            "<li>When using the Google Maps API, be sure to comply with the <a href=\"https://developers.google.com/maps/terms\">Google Maps/Google Earth API Terms of Service</a> and the <a href=\"https://www.google.com/permissions/geoguidelines/intl/en/terms.html\">Copyright Policy</a>.</li>"
            "<li>Ensure that your application does not violate any terms.</li>"
            "<li>The content provided in this document is for reference only. Users should consult and follow Google's official terms and policies before use.</li>"
            "<li>This document does not take responsibility for any copyright or legal issues arising from using this method.</li>"
            "<li>The final right of interpretation belongs to our company.</li>"
            "</ul>"
            "<hr style=\"margin: 50px 0; border: 1px solid #ddd;\">"
            "<h1 style=\"text-align: center;\">帮 助 文 档</h1>"
            "<h2 style=\"margin-top: 30px;font-size: 16px; font-weight: normal;\">方式1：使用 Google Maps API 加载地图瓦片</h2>"
            "<p>访问 <a href=\"https://cloud.google.com/maps-platform\">Google Cloud Platform</a> 并注册账号。根据您的需求选择相应的服务（如 Maps SDK 或 Static Maps API），并获取一个有效的API密钥。</p>"
            "<p>使用 Google Maps Tile API，您可以构建一个请求URL来获取地图瓦片。瓦片的URL格式通常如下：</p>"
            "<pre>"
            "https://maps.googleapis.com/maps/api/staticmap?center={latitude},{longitude}&zoom={zoom_level}&size={width}x{height}&key={API_KEY}"
            "</pre>"
            "<h2 style=\"margin-top: 30px;font-size: 16px; font-weight: normal;\">方式2：直接使用预构建的 URL 加载地图</h2>"
            "<p>您可以直接复制以下URL路径，粘贴到该对话框下方的文本框内，并点击右侧绿色按钮，则Google瓦片地图可成功加载，您可在设置的 \"Map\" 中查看，并选中Google图源使用。</p>"
            "<pre style=\"font-size: 17px;text-decoration: underline;\">"
            "<div style=\"background-color:#f8f9fa;border:1px solid #ddd;padding:10px;border-radius:6px;\">"
            "<div style=\"font-family:monospace;font-size:20px;\">"
            "http://mt2.google.com/vt/lyrs=y&hl=zh-CN&x=%1&y=%2&z=%3"
            "</div>"
            "<a href=\"copy-cn-url\" "
            "style=\"background:#4CAF50;color:white;padding:6px 12px;border-radius:4px;"
            "text-decoration:none;font-size:14px;\">复制 URL</a>"
            "</div>"
            "</pre>"
            "<h2 style=\"margin-top: 50px;\">免责声明</h2>"
            "<ul>"
            "<li>使用Google地图API时，请务必遵守 <a href=\"https://developers.google.com/maps/terms\">Google Maps/Google Earth API 服务条款</a> 和 <a href=\"https://www.google.com/permissions/geoguidelines/intl/en/terms.html\">版权政策</a>。</li>"
            "<li>确保您的应用程序不会违反任何条款。</li>"
            "<li>本文档提供的内容仅供参考，请用户在使用前自行查阅并遵 Google的官方条款和政策。</li>"
            "<li>本文档不对任何因使用此方法而产生的版权或法律问题承担责任。</li>"
            "<li>最终解释权归本公司所有。</li>"
            "</ul>"
            );


        layout->addWidget(textBrowser);

        connect(textBrowser, &QTextBrowser::anchorClicked, this, [=](const QUrl &url){
            QString link = url.toString();
            if (link == "copy-en-url") {
                QGuiApplication::clipboard()->setText("http://mt2.google.com/vt/lyrs=y&hl=en&x=%1&y=%2&z=%3");
                QMessageBox::information(this, "Copied", "URL copied to clipboard!");
                // GIF->dialogInfo(Dialog_OK, tr("URL copied to clipboard!"));
            }
            else if (link == "copy-cn-url") {
                QGuiApplication::clipboard()->setText("http://mt2.google.com/vt/lyrs=y&hl=zh-CN&x=%1&y=%2&z=%3");
                QMessageBox::information(this, "复制成功", "URL 已复制到剪贴板！");
                // GIF->dialogInfo(Dialog_OK, tr("URL 已复制到剪贴板！"));
            }
        });

        QHBoxLayout *buttonLayout = new QHBoxLayout();
        QPushButton *closeBtn = new QPushButton(tr("Close"), this);
        closeBtn->setFixedSize(120, 36);
        buttonLayout->addStretch();
        buttonLayout->addWidget(closeBtn);
        buttonLayout->addStretch();
        layout->addLayout(buttonLayout);

        connect(closeBtn, &QPushButton::clicked, this, &HelpDialog::accept);
    }


private:
    QTextBrowser *textBrowser;

};











class QQmlApplicationEngine;


class Themes : public QObject
{
    Q_OBJECT

public:
    Themes();

    Q_PROPERTY(bool isFakeCoords READ getIsFakeCoords NOTIFY changed)
    Q_PROPERTY(qreal resCoeff READ getResolutionCoeff NOTIFY changed)

    Q_PROPERTY(QColor disabledTextColor READ disabledTextColor NOTIFY changed)
    Q_PROPERTY(QColor disabledBackColor READ disabledBackColor NOTIFY changed)
    Q_PROPERTY(QColor hoveredBackColor READ hoveredBackColor NOTIFY changed)

    Q_PROPERTY(QColor textColor READ textColor NOTIFY changed)
    Q_PROPERTY(QColor textSolidColor READ textSolidColor NOTIFY changed)
    Q_PROPERTY(QColor textErrorColor READ textErrorColor NOTIFY changed)
    Q_PROPERTY(QFont textFont READ textFont NOTIFY changed)
    Q_PROPERTY(QFont textFontS READ textFontS NOTIFY changed)

    Q_PROPERTY(QColor menuBackColor READ menuBackColor NOTIFY changed)
    Q_PROPERTY(QColor frameBackColor READ frameBackColor NOTIFY changed)

    Q_PROPERTY(QColor controlBackColor READ controlBackColor NOTIFY changed)
    Q_PROPERTY(QColor controlBorderColor READ controlBorderColor NOTIFY changed)
    Q_PROPERTY(QColor controlSolidBackColor READ controlSolidBackColor NOTIFY changed)
    Q_PROPERTY(QColor controlSolidBorderColor READ controlSolidBorderColor NOTIFY changed)
    Q_PROPERTY(int screenSize   READ screenSize   NOTIFY changed)
    Q_PROPERTY(int screenWidth  READ screenWidth  NOTIFY changed)
    Q_PROPERTY(int screenHeight READ screenHeight NOTIFY changed)
    Q_PROPERTY(int menuWidth    READ menuWidth    NOTIFY changed)
    Q_PROPERTY(int iconSize     READ iconSize     NOTIFY changed)

    Q_PROPERTY(int themeID READ themeId WRITE setTheme NOTIFY changed)

    Q_PROPERTY(bool consoleVisible  READ consoleVisible WRITE setConsoleVisible NOTIFY interfaceChanged)
    Q_PROPERTY(int instrumentsGrade READ getInstrumentsGrade WRITE setInstrumentsGrade NOTIFY instrumentsGradeChanged)
    Q_PROPERTY(int currentLanguage  READ getCurrentLanguage  WRITE setCurrentLanguage  NOTIFY bootConfigChanged);
    Q_PROPERTY(int currentMaptype   READ getCurrentMaptype   WRITE setCurrentMaptype   NOTIFY bootConfigChanged);
    Q_PROPERTY(bool googleExist     READ getGoogleExist      WRITE setGoogleExist      NOTIFY bootConfigChanged);
    Q_PROPERTY(int mapSourceLoadVisible  READ getMapSourceLoadVisible  WRITE setMapSourceLoadVisible
                   NOTIFY mapSourceLoadVisibleChanged);



    bool getIsFakeCoords() const { return isFakeCoords_; };
    qreal getResolutionCoeff() const { return resolutionCoeff_; };
    QColor textColor() { return *_textColor; }
    QColor textErrorColor() { return *_textErrorColor; }
    QColor disabledTextColor() { return *_disabledTextColor; }
    QColor disabledBackColor() {return *_disabledBackColor;}
    QColor hoveredBackColor() { return *_hoveredBackColor; }
    QColor textSolidColor() { return *_textSolidColor; }
    QFont textFont() { return *_textFont; }
    QFont textFontS() { return *_textFontS; }

    QColor menuBackColor()  { return *_menuBackColor; }
    QColor frameBackColor() { return *_frameBackColor; }

    QColor controlBackColor()        { return *_controlBackColor; }
    QColor controlBorderColor()      { return *_controlBorderColor; }
    QColor controlSolidBackColor()   { return *_controlSolidBackColor; }
    QColor controlSolidBorderColor() { return *_controlSolidBorderColor; }
    int screenSize()   { return screenSize_; }
    int screenWidth()  { return screenWidth_; }
    int screenHeight() { return screenHeight_; }
    int menuWidth()    { return menuWidth_; }
    int iconSize()     { return iconSize_;  }

    int getCurrentLanguage();
    void setCurrentLanguage(int lang);
    int getCurrentMaptype();
    void setCurrentMaptype(int type);
    bool getGoogleExist();
    void setGoogleExist(bool googleExist);
    bool getMapSourceLoadVisible();
    void setMapSourceLoadVisible(bool visible);


    void setQmlEngine(QQmlApplicationEngine* engine);
    void setTheme(int theme_id = 0);

    int themeId() {
        return _id;
    }

    void setConsoleVisible(bool vis) {
        _isConsoleVisible = vis;
        emit interfaceChanged();
    }

    bool consoleVisible() {
        return _isConsoleVisible;
    }

    int getInstrumentsGrade() const {
        return instrumentsGrade_;
    }

    void setInstrumentsGrade(int instrumentsGrade) {
        instrumentsGrade_ = instrumentsGrade;
        emit instrumentsGradeChanged();
    }

    void bootConfig();
    SoftwareParametersStru getSoftwareParameters();
    void loadSoftwareParameters();
    void saveSoftwareParameters();
    void refreshLanguage();


    Q_INVOKABLE void updateResCoeff();
    Q_INVOKABLE void openGoogleHelpDocument();

private:
    u8 XorCheckSum(u8* input, u8 length);


signals:
    void changed();
    void interfaceChanged();
    void instrumentsGradeChanged();
    void bootConfigChanged();
    void mapSourceLoadVisibleChanged();
    // void mapLoadConfirm(bool googleMapExists);

protected:
    int _id = 0;

    QColor* _textColor;
    QColor* _textSolidColor;
    QColor* _textErrorColor;
    QColor* _disabledTextColor;
    QColor* _disabledBackColor;
    QColor* _hoveredBackColor;
    QFont* _textFont;
    QFont* _textFontS;

    QColor* _menuBackColor;
    QColor* _frameBackColor;
    QColor* _controlBackColor;
    QColor* _controlBorderColor;
    QColor* _controlSolidBackColor;
    QColor* _controlSolidBorderColor;
    int32_t screenSize_, screenWidth_, screenHeight_;
    int32_t menuWidth_ = 70;
    int32_t iconSize_ = 18;

    bool _isConsoleVisible;
    int instrumentsGrade_;


private:
    QQmlApplicationEngine* qmlEngine_ = nullptr;
    qreal checkResolutionCoeff() const;
    qreal resolutionCoeff_;
    bool isFakeCoords_;

    QTranslator *translator_;
    SoftwareParametersStru softwareParameters_;

    bool mapSourceLoadVisible_ = false;
};

inline qreal Themes::checkResolutionCoeff() const
{
    qreal retVal = 1.0;

#if defined(Q_OS_ANDROID) || defined(LINUX_ES)
    retVal = 2.0;
#endif

    return retVal;
}

#endif // THEME_H
