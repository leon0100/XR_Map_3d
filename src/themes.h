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



class Themes : public QObject
{
    Q_OBJECT

public:

    Themes() :
        instrumentsGrade_(-1),
        resolutionCoeff_(1.0),
        isFakeCoords_(false)
    {
#if defined (FAKE_COORDS)
        isFakeCoords_ = true;
#endif

        _isConsoleVisible = false;
    }

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

    Q_PROPERTY(bool consoleVisible READ consoleVisible WRITE setConsoleVisible NOTIFY interfaceChanged)
    Q_PROPERTY(int instrumentsGrade READ getInstrumentsGrade WRITE setInstrumentsGrade NOTIFY instrumentsGradeChanged)

    Q_INVOKABLE void updateResCoeff() {
        qreal currCoeff = checkResolutionCoeff();
        if (!qFuzzyCompare(1.0 + currCoeff, 1.0 + resolutionCoeff_)) {
            resolutionCoeff_ = currCoeff;
            emit changed();
        }
    };

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

    void setTheme(int theme_id = 0) {
        _id = theme_id;

        int fontId = QFontDatabase::addApplicationFont(":/fonts/AlibabaPuHuiTi-3-55-Regular.ttf");
        if (fontId == 0) {
            QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
            QFont font(fontFamily, 12);
            QApplication::setFont(font);
            _textFont  = new QFont(fontFamily, 26);
            _textFontS = new QFont(fontFamily, 20);
        }

        _textErrorColor = new QColor(250, 0, 0);

        _frameBackColor = new QColor(45, 45, 45, 50);

        if(theme_id == 0) {
            _textColor = new QColor(250, 250, 250);
            _textSolidColor = new QColor(250, 250, 250);
            _menuBackColor = new QColor(45, 45, 45, 240);
            _controlBackColor = new QColor(60, 60, 60);
            _controlBorderColor = new QColor(100, 100, 100);
            _controlSolidBackColor = new QColor(100, 100, 100);
            _controlSolidBorderColor = new QColor(150, 150, 150);

            _disabledTextColor = new QColor(150, 150, 150);
            _disabledBackColor = new QColor(50, 50, 50);
            _hoveredBackColor = new QColor(70,70,70);

        } else if(theme_id == 1) {
            _textColor = new QColor(255, 255, 255);
            _textSolidColor = new QColor(0, 0, 0);
            _menuBackColor = new QColor(0, 0, 0, 255);
            _controlBackColor = new QColor(55, 55, 55);
            _controlBorderColor = new QColor(155, 155, 155);
            _controlSolidBackColor = new QColor(255, 255, 255);
            _controlSolidBorderColor = new QColor(0, 0, 0, 0);

            _disabledTextColor = new QColor(150, 150, 150);
            _disabledBackColor = new QColor(50, 50, 50);

        } else if(theme_id == 2) {
            _textColor = new QColor(25, 25, 25);
            _textSolidColor = new QColor(25, 25, 25);
            _menuBackColor = new QColor(240, 240, 240, 240);
            _controlBackColor = new QColor(250, 250, 250);
            _controlBorderColor = new QColor(100, 100, 100);
            _controlSolidBackColor = new QColor(255, 255, 255);
            _controlSolidBorderColor = new QColor(150, 150, 150);

            _disabledTextColor = new QColor(150, 150, 150);
            _disabledBackColor = new QColor(50, 50, 50);

        } else if(theme_id == 3) {
            _textColor = new QColor(0, 0, 0);
            _textSolidColor = new QColor(255, 255, 255);
            _menuBackColor = new QColor(250, 250, 250, 250);
            _controlBackColor = new QColor(255, 255, 255);
            _controlBorderColor = new QColor(100, 100, 100);
            _controlSolidBackColor = new QColor(0, 0, 0);
            _controlSolidBorderColor = new QColor(255, 255, 255);

            _disabledTextColor = new QColor(150, 150, 150);
            _disabledBackColor = new QColor(50, 50, 50);
        }

        QScreen *screen = QGuiApplication::primaryScreen();
        if (screen) {
            QSize size = screen->size();
            screenWidth_  = size.width();
            screenHeight_ = size.height();
            screenSize_   = qMin(screenWidth_, screenHeight_);
            menuWidth_    = screenSize_ * 0.06;
            iconSize_     = menuWidth_  * 0.3;
        } else {
            screenSize_ = 600;
        }

        emit changed();
    }

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

signals:
    void changed();
    void interfaceChanged();
    void instrumentsGradeChanged();

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
    qreal checkResolutionCoeff() const;
    qreal resolutionCoeff_;
    bool isFakeCoords_;
};

inline qreal Themes::checkResolutionCoeff() const
{
    qreal retVal = 1.0;

#if defined(Q_OS_ANDROID) || defined(LINUX_ES)
    retVal = 2.0;
#endif

    // QScreen *screen = QApplication::primaryScreen();
    // if (screen) {
    //     auto physDotsPerInch = screen->physicalDotsPerInch();
    //     Q_UNUSED(physDotsPerInch)


    //     // retVal = 2.0; // test
    //     // qDebug() << "Logical DPI:" << screen->logicalDotsPerInch();
    //     // qDebug() << "Physical DPI:" << screen->physicalDotsPerInch();
    //     // qDebug() << "Device Pixel Ratio:" << screen->devicePixelRatio();
    // }

    return retVal;
}

#endif // THEME_H
