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
    if(!fileName.isEmpty())
    {
        readColorToColorList(fileName);

        getColorFromColorList();
    }
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
    for(int i=0;i<selfColor.value("count",0).toInt();i++) {
        tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
        tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
        colorList_surface.append(tmp);
    }
    selfColor.endGroup();

    colorList_fish.clear();
    selfColor.beginGroup("fish");
    for(int i=0;i<selfColor.value("count",0).toInt();i++) {
        tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
        tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
        colorList_fish.append(tmp);
    }
    selfColor.endGroup();

    colorList_bottom.clear();
    selfColor.beginGroup("bottom");
    for(int i=0;i<selfColor.value("count",0).toInt();i++) {
        tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
        tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
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


