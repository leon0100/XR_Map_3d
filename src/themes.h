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


#include "dataset_defs.h"
#include "console.h"


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

    Q_PROPERTY(bool consoleVisible READ consoleVisible WRITE setConsoleVisible NOTIFY interfaceChanged)
    Q_PROPERTY(int instrumentsGrade READ getInstrumentsGrade WRITE setInstrumentsGrade NOTIFY instrumentsGradeChanged)
    Q_PROPERTY(int currentLanguage  READ getCurrentLanguage  WRITE setCurrentLanguage NOTIFY bootConfigChanged);



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
    void getSoftwareParameters();
    void saveSoftwareParameters();
    void refreshLanguage();


    Q_INVOKABLE void updateResCoeff();

private:
    u8 XorCheckSum(u8* input, u8 length);


signals:
    void changed();
    void interfaceChanged();
    void instrumentsGradeChanged();
    void bootConfigChanged();

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
