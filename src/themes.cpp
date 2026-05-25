#include "themes.h"

#include <QSslSocket>



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
    if(translator_) {
        QGuiApplication::instance()->installTranslator(translator_);
        translator_->load(":translations/translation_en.qm");
    }

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


void Themes::updateResCoeff()
{
    qreal currCoeff = checkResolutionCoeff();
    if (!qFuzzyCompare(1.0 + currCoeff, 1.0 + resolutionCoeff_)) {
        resolutionCoeff_ = currCoeff;
        emit changed();
    }
};

void Themes::openGoogleHelpDocument()
{
    HelpDialog helpDialog;
    helpDialog.exec();
}


void Themes::bootConfig()
{
    loadSoftwareParameters();
    bool firstRun = softwareParameters_.isFirstRun;
    if (firstRun)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Location Inquiry");
        msgBox.setText("Are you in China?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int result = msgBox.exec();
        if(result == QMessageBox::Yes) {
            softwareParameters_.inChina = true;
            softwareParameters_.currentLanguage = 1;
            softwareParameters_.mapSourceType = geovisEarthSource;
            // QMessageBox::warning(nullptr, tr("Hint"), tr("This application relies on OpenStreetMap,"
            //          " and may not be able to use the [Place-Name-Search] in China."));
            QMessageBox::warning(nullptr, "提示", "本应用依赖OpenStreetMap图源, 在中国大陆地区可能无法使用[地名搜索]功能");
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


    /*-statusBar-*/
    QString lonDirection = softwareParameters_.currentLon >= 0 ? "°E" : "°W";
    QString latiDirection = softwareParameters_.currentLati >= 0 ? "°N" : "°S";
    double lonDou = qAbs(softwareParameters_.currentLon);
    double latiDou = qAbs(softwareParameters_.currentLati);
    QString coor = "Coordinate: " + QString::number(lonDou, 'f', 6) + lonDirection + ","
                   + QString::number(latiDou, 'f', 6) + latiDirection;


    if(softwareParameters_.inChina) {
        qDebug() <<"inChina........." << softwareParameters_.inChina;
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

SoftwareParametersStru Themes::getSoftwareParameters()
{
    return softwareParameters_;
}

void Themes::loadSoftwareParameters()
{
#ifdef Q_OS_ANDROID
    QAndroidJniObject context = QtAndroid::androidContext();

    QAndroidJniObject jArray = QAndroidJniObject::callStaticObjectMethod(
        "com/nqc/Config",  "loadSoftwareParameters",  "(Landroid/content/Context;)[F", context.object<jobject>());

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
    QString path = SAVE_SOFTWARE_PAR_PATH;
    QDir().mkpath(path);
    path = path + "/" + SAVE_SOFTWARE_PAR;
    QFile file(path);

    int size = sizeof(softwareParameters_);

    if(!file.exists()) {
        qDebug() << "cfg not exist, create...";
        softwareParameters_.isFirstRun = true;
        saveSoftwareParameters();
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QDataStream in(&file);
        if(in.readRawData((char*)&softwareParameters_,size)) {
            u8 crc = XorCheckSum((u8*)&softwareParameters_,(size-1));
            if(crc == softwareParameters_.crcValue) {
                // qDebug() << "getsoftware: " << softwareParameters_.currentLati << "  " << softwareParameters_.currentLon;
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
    qDebug() << "saveSoftwareParameters............";

    //异或校验
    u8 crc = XorCheckSum((u8*)&softwareParameters_,(sizeof(softwareParameters_)-1));
    softwareParameters_.crcValue = crc;

#ifdef Q_OS_ANDROID
    QAndroidJniObject context = QtAndroid::androidContext();
    QAndroidJniObject::callStaticMethod<void>(
         "com/nqc/Config",
        "saveSoftwareParameters",
        "(Landroid/content/Context;IIDDIIZIZ)V",
        context.object<jobject>(),
        (jint)softwareParameters_.mapSourceType,
        (jint)softwareParameters_.currentLevel,
        (jdouble)softwareParameters_.currentLon,
        (jdouble)softwareParameters_.currentLati,
        (jint)softwareParameters_.existGoogle,
        (jint)softwareParameters_.currentLanguage,
        (jboolean)softwareParameters_.inChina,
        (jint)softwareParameters_.crcValue,
        (jboolean)softwareParameters_.isFirstRun );

#elif defined(Q_OS_WIN)
    QString path = SAVE_SOFTWARE_PAR_PATH;
    QDir().mkpath(path);
    path = path + "/" + SAVE_SOFTWARE_PAR;
    QFile file(path);
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

int Themes::getCurrentMaptype()
{
    return softwareParameters_.mapSourceType;
}

void Themes::setCurrentMaptype(int type)
{
    softwareParameters_.mapSourceType = MapSourceType(type);

    emit bootConfigChanged();
}

bool Themes::getGoogleExist()
{
    return softwareParameters_.existGoogle;
}

void Themes::setGoogleExist(bool googleExist)
{
    if(softwareParameters_.existGoogle == googleExist) {
        return;
    }
    softwareParameters_.existGoogle = googleExist;

    emit bootConfigChanged();
}

bool Themes::getMapSourceLoadVisible()
{
    return mapSourceLoadVisible_;
}

void Themes::setMapSourceLoadVisible(bool visible)
{
    if(mapSourceLoadVisible_ == visible) {
        return;
    }
    mapSourceLoadVisible_ = visible;

    emit mapSourceLoadVisibleChanged();
}

void Themes::refreshLanguage()
{
    if(softwareParameters_.isFirstRun) {
        softwareParameters_.isFirstRun = false;
        if(softwareParameters_.inChina) {
            softwareParameters_.currentLanguage = 0;
            setCurrentLanguage(1);
        }
        return;
    }

    if(softwareParameters_.currentLanguage == 1) {
        softwareParameters_.currentLanguage = 0;
        setCurrentLanguage(1);
    }

}
