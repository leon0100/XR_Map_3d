#include "themes.h"

#include <QMessageBox>



Themes::Themes() : QObject(),
    instrumentsGrade_(-1),
    resolutionCoeff_(1.0),
    isFakeCoords_(false)
{

#if defined (FAKE_COORDS)
    isFakeCoords_ = true;
#endif

    _isConsoleVisible = false;
}

void Themes::setQmlEngine(QQmlApplicationEngine* engine)
{
    qmlEngine_ = engine;
}

void Themes::setTheme(int theme_id)
{
    _id = theme_id;

    QSettings settings("Toslon", "ToslonApp");
    translator_ = new QTranslator;
    QGuiApplication::instance()->installTranslator(translator_);

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

    }
    else if(theme_id == 1) {
        _textColor = new QColor(255, 255, 255);
        _textSolidColor = new QColor(0, 0, 0);
        _menuBackColor = new QColor(0, 0, 0, 255);
        _controlBackColor = new QColor(55, 55, 55);
        _controlBorderColor = new QColor(155, 155, 155);
        _controlSolidBackColor = new QColor(255, 255, 255);
        _controlSolidBorderColor = new QColor(0, 0, 0, 0);

        _disabledTextColor = new QColor(150, 150, 150);
        _disabledBackColor = new QColor(50, 50, 50);

    }
    else if(theme_id == 2) {
        _textColor = new QColor(25, 25, 25);
        _textSolidColor = new QColor(25, 25, 25);
        _menuBackColor = new QColor(240, 240, 240, 240);
        _controlBackColor = new QColor(250, 250, 250);
        _controlBorderColor = new QColor(100, 100, 100);
        _controlSolidBackColor = new QColor(255, 255, 255);
        _controlSolidBorderColor = new QColor(150, 150, 150);

        _disabledTextColor = new QColor(150, 150, 150);
        _disabledBackColor = new QColor(50, 50, 50);

    }
    else if(theme_id == 3) {
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

    // getSoftwareParameters();
}


void Themes::updateResCoeff()
{
    qreal currCoeff = checkResolutionCoeff();
    if (!qFuzzyCompare(1.0 + currCoeff, 1.0 + resolutionCoeff_)) {
        resolutionCoeff_ = currCoeff;
        emit changed();
    }
};

void Themes::bootConfig()
{
    bool firstRun = softwareParameters_.isFirstRun;
    firstRun = true;
    if (firstRun)
    {
        softwareParameters_.isFirstRun = false;

        QMessageBox msgBox;
        msgBox.setWindowTitle("Location Inquiry");
        msgBox.setText("Are you in China?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);

        int result = msgBox.exec();
        if(result == QMessageBox::Yes) {
            softwareParameters_.inChina = true;
            softwareParameters_.currentLanguage = 1;
            softwareParameters_.mapSourceType = geovisEarthSource;
            // QMessageBox::warning(nullptr, tr("Hint"), tr("This application relies on OpenStreetMap,"
            //          " and may not be able to use the [Place-Name-Search] in China."));
            QMessageBox::warning(nullptr, tr("提示"),
                            tr("本应用依赖OpenStreetMap图源, 在中国大陆地区可能无法使用[地名搜索]功能"));
        }
        else {
            softwareParameters_.inChina = false;
            softwareParameters_.currentLanguage = 0;
            softwareParameters_.mapSourceType = openStreetMapSource;

        }

        softwareParameters_.currentLon = 118.05459876165770;
        softwareParameters_.currentLati = 30.39485766185152;
        softwareParameters_.currentLevel = 14;
    }


    // satelliteImageLoad_ = new SatelliteImageLoad;
    // if(softwareParameters_.existGoogle == 1){
    //     satelliteImageLoad_->systemAddGoogleMap();
    // }
    // satelliteImageLoad_->hide();


    /*-statusBar-*/
    QString lonDirection = softwareParameters_.currentLon >= 0 ? "°E" : "°W";
    QString latiDirection = softwareParameters_.currentLati >= 0 ? "°N" : "°S";
    double lonDou = qAbs(softwareParameters_.currentLon);
    double latiDou = qAbs(softwareParameters_.currentLati);
    QString coor = "Coordinate: " + QString::number(lonDou, 'f', 6) + lonDirection + ","
                   + QString::number(latiDou, 'f', 6) + latiDirection;


    // QGuiApplication::instance()->removeTranslator(translator_);

    if(softwareParameters_.currentLanguage == 1) {
        qDebug() << "softwareParameters_.currentLanguage111111111111";
        translator_->load(":translations/translation_ch.qm");
    }
    else if(softwareParameters_.currentLanguage == 0) {
        qDebug() << "softwareParameters_.currentLanguage2222222222222";
        translator_->load(":translations/translation_en.qm");
    }


    if(softwareParameters_.inChina) {

    }


    if(softwareParameters_.existGoogle == 1) {
        // googleAction_ = new QAction(tr("%1").arg(GOOGLEMAP),this);
        // googleAction_->setCheckable(true);
        // actionMapGroup_->addAction(googleAction_);
        // ui->menuMap->addAction(googleAction_);
        // mapMenu_->addAction(googleAction_);
        // connect(googleAction_,&QAction::triggered,this,&GraphMenu::slot_actionMapSwitch);
    }


    emit bootConfigChanged();
}

void Themes::getSoftwareParameters()
{
#ifdef Q_OS_ANDROID
    QAndroidJniObject context = QtAndroid::androidContext();

    QAndroidJniObject jArray = QAndroidJniObject::callStaticObjectMethod(
        "com/nqc/Config",    "loadSoftwareParameters",  "(Landroid/content/Context;)[F",
        context.object<jobject>()   );

    if (jArray.isValid()) {
        jfloatArray arr = jArray.object<jfloatArray>();
        QAndroidJniEnvironment env;
        jsize len = env->GetArrayLength(arr);
        jfloat *elements = env->GetFloatArrayElements(arr, 0);

        if (len >= 9) {
            softwareParameters_.mapSourceType   = (MapSourceType)elements[0];
            softwareParameters_.currentLevel    = (int)elements[1];
            softwareParameters_.currentLon      = elements[2];
            softwareParameters_.currentLati     = elements[3];
            softwareParameters_.existGoogle     = (u8)elements[4];
            softwareParameters_.currentLanguage = (u8)elements[5];
            softwareParameters_.inChina         = (elements[6] > 0.5f);
            softwareParameters_.crcValue        = (u8)elements[7];
            softwareParameters_.isFirstRun      = (elements[8] > 0.5f);
        }

        env->ReleaseFloatArrayElements(arr, elements, 0);
    } else {
        qDebug() << "jArray.is not  Valid()...";
    }

#elif defined(Q_OS_WIN)
    QFile file(":/config/cfg");

    int size = sizeof(softwareParameters_);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QDataStream in(&file);
        if(in.readRawData((char*)&softwareParameters_,size)) {
            u8 crc = XorCheckSum((u8*)&softwareParameters_,(size-1));
            if(crc == softwareParameters_.crcValue) {
                qDebug() << "getsoftware: " << softwareParameters_.currentLati << "  " << softwareParameters_.currentLon;
                file.close();
                return;
            }
            else {
                qDebug() << "crc ERR";
                file.close();
                return;
            }
        }
        else {
            qDebug()<<"read failed";
            file.close();
            return;
        }
    }
    else
    {
        qDebug() << "无法打开文件: " << "  error:" << file.error() << "  reason:" << file.errorString();
        file.close();
        return;
    }

#endif
}


void Themes::saveSoftwareParameters()
{
//     QPoint centeScenePt = mapGraphicsView_->getViewMiddlePos();
//     Bing::pixelXYToLatLong(centeScenePt,mapGraphicsView_->getCurrentLevel(),
//                            softwareParameters_.currentLon,softwareParameters_.currentLati);
//     softwareParameters_.currentLevel = 16;
//     softwareParameters_.existGoogle = 0;
//     QList<QAction*> actions;
// #ifdef Q_OS_WIN
//     actions = ui->menuMap->actions();
// #elif defined(Q_OS_ANDROID)
//     actions = mapMenu_->actions();
// #endif
//     for(QAction *action: actions) {
//         QString mapName = action->text();
//         if(mapName == GOOGLEMAP) {
//             softwareParameters_.existGoogle = 1;
//         }
//         if(action->isChecked()) {
//             if(mapName == GOOGLEMAP) {
//                 softwareParameters_.mapSourceType = googleMapSource;
//             }
//             else if(mapName == GEOVISEARTHMAP) {
//                 softwareParameters_.mapSourceType = geovisEarthSource;
//             }
//             else if(mapName == OPENSTREETMAP) {
//                 softwareParameters_.mapSourceType = openStreetMapSource;
//             }
//             else if(mapName == AMAP) {
//                 softwareParameters_.mapSourceType = amapMapSource;
//             }
//         }
//     }

    // softwareParametersStru.isFirstRun = true;//用来生成配置文件config（已经生成好了，以后打包一般不用打开这行）

    //抑或校验
    u8 crc = XorCheckSum((u8*)&softwareParameters_,(sizeof(softwareParameters_)-1));
    softwareParameters_.crcValue = crc;

#ifdef Q_OS_ANDROID
    QAndroidJniObject context = QtAndroid::androidContext();
    QAndroidJniObject::callStaticMethod<void>(
        "com/nqc/Config",
        "saveSoftwareParameters",
        "(Landroid/content/Context;IIDDIIZIZ)V",
        context.object<jobject>(),
        (jint)softwareParametersStru.mapSourceType,
        (jint)softwareParametersStru.currentLevel,
        (jdouble)softwareParametersStru.currentLon,
        (jdouble)softwareParametersStru.currentLati,
        (jint)softwareParametersStru.existGoogle,
        (jint)softwareParametersStru.currentLanguage,
        (jboolean)softwareParametersStru.inChina,
        (jint)softwareParametersStru.crcValue,
        (jboolean)softwareParametersStru.isFirstRun );

#elif defined(Q_OS_WIN)
    QFile file(":/config/cfg");
    if(file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        if(!out.writeRawData((char *)&softwareParameters_,sizeof(softwareParameters_)))
        {
            qDebug()<<"write parameter failed!";
            file.close();
        }
    }
#endif
}


u8 Themes::XorCheckSum(u8*input, u8 length)
{
    u16 idx = 0; u8 xorResult = 0;

    // !! Initialize the reseut with the 'array[0]' !!
    xorResult = input[0];

    // X-OR Loop check
    for (idx = 1; idx < length; idx++)
    {
        xorResult ^= input[idx];
    }

    return xorResult;
}


int Themes::getCurrentLanguage()
{
    return (int)(softwareParameters_.currentLanguage);
}

void Themes::setCurrentLanguage(int lang)
{
    if(softwareParameters_.currentLanguage == lang) {
        return;
    }

    softwareParameters_.currentLanguage = (u8)lang;
    translator_->load(lang == 1 ? ":translations/translation_ch.qm" : ":translations/translation_en.qm");

    if(qmlEngine_) {
        qmlEngine_->retranslate();
    }

    emit bootConfigChanged();
}

