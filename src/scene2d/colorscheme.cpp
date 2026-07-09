#include "colorscheme.h"

int ZyColorScheme::backgroundIndex = 2;
int ZyColorScheme::background[] = {0xffffff, 0x000000, 0x104684};
int ZyColorScheme::colorLine = 0;
int ZyColorScheme::onlineColor = 0xff0000;

/*--------------------------------------------------------------------------------------*/
int ZyColorScheme::colorScheme_surface[255] = {0};
int ZyColorScheme::colorScheme_fish[255] = {0};
int ZyColorScheme::colorScheme_bottom[255] = {0};

int ZyColorScheme::colorScheme_surface_2[255] = {0};
int ZyColorScheme::colorScheme_fish_2[255] = {0};
int ZyColorScheme::colorScheme_bottom_2[255] = {0};

ZyColorScheme::ZyColorScheme(){}
ZyColorScheme::~ZyColorScheme(){}

void ZyColorScheme::loadColorScheme(QString fileName)
{
    /*--文件是否存在--*/
    // if(!fileName.isEmpty())
    // {
    //     readColorToColorList(fileName);

    //     getColorFromColorList();
    // }

    initDefaultColorList();
}
void ZyColorScheme::saveColorScheme()
{
    QString fileName = qApp->applicationDirPath() + "/dcs_caise.tcs";

    QSettings selfColor(fileName, QSettings::IniFormat);

    selfColor.beginGroup("surface");
    selfColor.setValue("count", colorList_surface.count());
    for(int i=0;i<colorList_surface.count();i++)
    {
        selfColor.setValue("position_"+QString::number(i), colorList_surface.at(i).colorPosition);
        selfColor.setValue("value_"+QString::number(i), colorList_surface.at(i).colorValue);
    }
    selfColor.endGroup();

    selfColor.beginGroup("fish");
    selfColor.setValue("count", colorList_fish.count());
    for(int i=0;i<colorList_fish.count();i++)
    {
        selfColor.setValue("position_"+QString::number(i), colorList_fish.at(i).colorPosition);
        selfColor.setValue("value_"+QString::number(i), colorList_fish.at(i).colorValue);
    }
    selfColor.endGroup();

    selfColor.beginGroup("bottom");
    selfColor.setValue("count", colorList_bottom.count());
    for(int i=0;i<colorList_bottom.count();i++)
    {
        selfColor.setValue("position_"+QString::number(i), colorList_bottom.at(i).colorPosition);
        selfColor.setValue("value_"+QString::number(i), colorList_bottom.at(i).colorValue);
    }
    selfColor.endGroup();
}

void ZyColorScheme::readColorToColorList(QString fileName)
{
    /*-用于存取自定义色表-*/
    QSettings selfColor(fileName, QSettings::IniFormat);
    StructColorList tmp;

    /*-自定义配色表读取-*/
    colorList_surface.clear();
    selfColor.beginGroup("surface");
    qDebug() << "11111111111111111selfColor.value count,.toInt()" << selfColor.value("count",0).toInt();
    for(int i=0;i<selfColor.value("count",0).toInt();i++) {
        tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
        tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
        qDebug() << "tmp.colorPosition:" << tmp.colorPosition  << "  tmp.colorValue" << tmp.colorValue;
        colorList_surface.append(tmp);
    }
    selfColor.endGroup();

    colorList_fish.clear();
    selfColor.beginGroup("fish");
    qDebug() << "22222222222222222selfColor.value count,.toInt()" << selfColor.value("count",0).toInt();
    for(int i=0;i<selfColor.value("count",0).toInt();i++) {
        tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
        tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
        qDebug() << "tmp.colorPosition:" << tmp.colorPosition  << "  tmp.colorValue" << tmp.colorValue;
        colorList_fish.append(tmp);
    }
    selfColor.endGroup();

    colorList_bottom.clear();
    selfColor.beginGroup("bottom");
    qDebug() << "3333333333333333selfColor.value count,.toInt()" << selfColor.value("count",0).toInt();
    for(int i=0;i<selfColor.value("count",0).toInt();i++) {
        tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
        tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
        qDebug() << "tmp.colorPosition:" << tmp.colorPosition  << "  tmp.colorValue" << tmp.colorValue;
        colorList_bottom.append(tmp);
    }
    selfColor.endGroup();
}

void ZyColorScheme::getColorFromColorList()
{
    /*-表层渐变显示信号颜色图-*/
    QLinearGradient linearGradient_Surface(QPointF(0, 0), QPointF(0, 255));
    QLinearGradient linearGradient_Fish(QPointF(0, 0), QPointF(0, 255));
    QLinearGradient linearGradient_Bottom(QPointF(0, 0), QPointF(0, 255));
    QPixmap pixmap(100,256);
    QPainter painter;
    QImage GradientImage;

    /*-颜色链表中按顺序进行渐变-*/
    for(int i = 0; i < colorList_surface.count(); i++) {
        linearGradient_Surface.setColorAt(colorList_surface.at(i).colorPosition/255.0, colorList_surface.at(i).colorValue);
    }
    painter.begin(&pixmap);
    painter.setBrush(linearGradient_Surface);
    painter.drawRect(0, 0, 100, 256);
    painter.end();
    GradientImage = pixmap.toImage();
    for(int i = 0;i < 255; i++) {
        colorScheme_surface[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
    }
    /*-颜色链表中按顺序进行渐变-*/
    for(int i = 0; i < colorList_fish.count(); i++) {
        linearGradient_Fish.setColorAt(colorList_fish.at(i).colorPosition/255.0, colorList_fish.at(i).colorValue);
    }
    painter.begin(&pixmap);
    painter.setBrush(linearGradient_Fish);
    painter.drawRect(0, 0, 100, 256);
    painter.end();
    GradientImage = pixmap.toImage();
    for(int i = 0; i < 255; i++) {
        colorScheme_fish[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
    }
    /*-颜色链表中按顺序进行渐变-*/
    for(int i = 0; i<colorList_bottom.count(); i++) {
        linearGradient_Bottom.setColorAt(colorList_bottom.at(i).colorPosition/255.0, colorList_bottom.at(i).colorValue);
    }
    painter.begin(&pixmap);
    painter.setBrush(linearGradient_Bottom);
    painter.drawRect(0, 0, 100, 256);
    painter.end();
    GradientImage = pixmap.toImage();
    for(int i = 0; i < 255; i++) {
        colorScheme_bottom[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
    }
}

void ZyColorScheme::loadColorScheme_2(QString fileName)
{
    /*-文件是否存在-*/
    if(!fileName.isEmpty()) {
        readColorToColorList_2(fileName);
        getColorFromColorList_2();
    }
}

void ZyColorScheme::saveColorScheme_2()
{
    QString fileName = qApp->applicationDirPath() + "/dcs_2.tcs";

    QSettings selfColor(fileName, QSettings::IniFormat);

    selfColor.beginGroup("surface");
    selfColor.setValue("count", colorList_surface_2.count());
    for(int i=0;i<colorList_surface_2.count();i++)
    {
        selfColor.setValue("position_"+QString::number(i), colorList_surface_2.at(i).colorPosition);
        selfColor.setValue("value_"+QString::number(i), colorList_surface_2.at(i).colorValue);
    }
    selfColor.endGroup();

    selfColor.beginGroup("fish");
    selfColor.setValue("count", colorList_fish_2.count());
    for(int i=0;i<colorList_fish_2.count();i++)
    {
        selfColor.setValue("position_"+QString::number(i), colorList_fish_2.at(i).colorPosition);
        selfColor.setValue("value_"+QString::number(i), colorList_fish_2.at(i).colorValue);
    }
    selfColor.endGroup();

    selfColor.beginGroup("bottom");
    selfColor.setValue("count", colorList_bottom_2.count());
    for(int i=0;i<colorList_bottom_2.count();i++)
    {
        selfColor.setValue("position_"+QString::number(i), colorList_bottom_2.at(i).colorPosition);
        selfColor.setValue("value_"+QString::number(i), colorList_bottom_2.at(i).colorValue);
    }
    selfColor.endGroup();
}

void ZyColorScheme::readColorToColorList_2(QString fileName)
{
    /*-用于存取自定义色表-*/
    QSettings selfColor(fileName, QSettings::IniFormat);
    StructColorList tmp;

    /*-自定义配色表读取-*/
    colorList_surface_2.clear();
    selfColor.beginGroup("surface");
    for(int i=0;i<selfColor.value("count",0).toInt();i++)
    {
        tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
        tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
        colorList_surface_2.append(tmp);
    }
    selfColor.endGroup();

    colorList_fish_2.clear();
    selfColor.beginGroup("fish");
    for(int i=0;i<selfColor.value("count",0).toInt();i++)
    {
        tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
        tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
        colorList_fish_2.append(tmp);
    }
    selfColor.endGroup();

    colorList_bottom_2.clear();
    selfColor.beginGroup("bottom");
    for(int i=0;i<selfColor.value("count",0).toInt();i++)
    {
        tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
        tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
        colorList_bottom_2.append(tmp);
    }
    selfColor.endGroup();
}

void ZyColorScheme::getColorFromColorList_2()
{
    /*-表层渐变显示信号颜色图-*/
    QLinearGradient linearGradient_Surface(QPointF(0, 0), QPointF(0, 255));
    QLinearGradient linearGradient_Fish(QPointF(0, 0), QPointF(0, 255));
    QLinearGradient linearGradient_Bottom(QPointF(0, 0), QPointF(0, 255));
    QPixmap pixmap(100,256);
    QPainter painter;
    QImage GradientImage;

    /*-颜色链表中按顺序进行渐变-*/
    for(int i = 0; i<colorList_surface_2.count(); i++) {
        linearGradient_Surface.setColorAt(colorList_surface_2.at(i).colorPosition/255.0, colorList_surface_2.at(i).colorValue);
    }
    painter.begin(&pixmap);
    painter.setBrush(linearGradient_Surface);
    painter.drawRect(0, 0, 100, 256);
    painter.end();
    GradientImage = pixmap.toImage();
    for(int i = 0;i < 255;i++) {
        colorScheme_surface_2[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
    }
    /*-颜色链表中按顺序进行渐变-*/
    for(int i = 0; i < colorList_fish_2.count(); i++) {
        linearGradient_Fish.setColorAt(colorList_fish_2.at(i).colorPosition/255.0, colorList_fish_2.at(i).colorValue);
    }
    painter.begin(&pixmap);
    painter.setBrush(linearGradient_Fish);
    painter.drawRect(0, 0, 100, 256);
    painter.end();
    GradientImage = pixmap.toImage();
    for(int i = 0; i < 255; i++) {
        colorScheme_fish_2[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
    }
    /*-颜色链表中按顺序进行渐变-*/
    for(int i = 0; i < colorList_bottom_2.count(); i++) {
        linearGradient_Bottom.setColorAt(colorList_bottom_2.at(i).colorPosition/255.0, colorList_bottom_2.at(i).colorValue);
    }
    painter.begin(&pixmap);
    painter.setBrush(linearGradient_Bottom);
    painter.drawRect(0, 0, 100, 256);
    painter.end();
    GradientImage = pixmap.toImage();
    for(int i = 0;i < 255; i++) {
        colorScheme_bottom_2[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
    }
}

void ZyColorScheme::initDefaultColorList()
{
    StructColorList tmp;

    //==============================
    // surface
    //==============================
    colorList_surface.clear();

    tmp.colorPosition = 0;
    tmp.colorValue = 2908582;
    colorList_surface.append(tmp);

    tmp.colorPosition = 207;
    tmp.colorValue = 16711680;
    colorList_surface.append(tmp);

    tmp.colorPosition = 110;
    tmp.colorValue = 16776960;
    colorList_surface.append(tmp);

    tmp.colorPosition = 50;
    tmp.colorValue = 65280;
    colorList_surface.append(tmp);

    tmp.colorPosition = 255;
    tmp.colorValue = 0;
    colorList_surface.append(tmp);


    //==============================
    // fish
    //==============================
    colorList_fish.clear();

    tmp.colorPosition = 0;
    tmp.colorValue = 671120;
    colorList_fish.append(tmp);

    tmp.colorPosition = 0;
    tmp.colorValue = 0;
    colorList_fish.append(tmp);

    tmp.colorPosition = 14;
    tmp.colorValue = 8289918;
    colorList_fish.append(tmp);

    tmp.colorPosition = 41;
    tmp.colorValue = 3249152;
    colorList_fish.append(tmp);

    tmp.colorPosition = 64;
    tmp.colorValue = 4376576;
    colorList_fish.append(tmp);

    tmp.colorPosition = 102;
    tmp.colorValue = 16766730;
    colorList_fish.append(tmp);

    tmp.colorPosition = 153;
    tmp.colorValue = 8026746;
    colorList_fish.append(tmp);

    tmp.colorPosition = 251;
    tmp.colorValue = 16711680;
    colorList_fish.append(tmp);

    tmp.colorPosition = 255;
    tmp.colorValue = 0;
    colorList_fish.append(tmp);



    //==============================
    // bottom
    //==============================
    colorList_bottom.clear();

    tmp.colorPosition = 0;
    tmp.colorValue = 2934783;
    colorList_bottom.append(tmp);

    tmp.colorPosition = 1;
    tmp.colorValue = 0;
    colorList_bottom.append(tmp);

    tmp.colorPosition = 31;
    tmp.colorValue = 4932869;
    colorList_bottom.append(tmp);

    tmp.colorPosition = 69;
    tmp.colorValue = 9073930;
    colorList_bottom.append(tmp);

    tmp.colorPosition = 104;
    tmp.colorValue = 16768826;
    colorList_bottom.append(tmp);

    tmp.colorPosition = 255;
    tmp.colorValue = 0;
    colorList_bottom.append(tmp);


    //重新生成255级颜色表
    getColorFromColorList();
}


int ZyColorScheme::getOnlineColor()
{
    return onlineColor;
}
void ZyColorScheme::setOnlineColor(int value)
{
    onlineColor = value;
}

int ZyColorScheme::getBackgroundIndex()
{
    return backgroundIndex;
}
void ZyColorScheme::setBackgroundIndex(int value)
{
    backgroundIndex = value;
}


